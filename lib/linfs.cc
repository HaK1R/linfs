#include "lib/linfs.h"

#include <cstddef>
#include <exception>
#include <memory>
#include <utility>

#include "lib/entries/entry.h"
#include "lib/entries/symlink_entry.h"
#include "lib/file_impl.h"
#include "lib/layout/device_layout.h"
#include "lib/utils/exception_handler.h"

namespace fs {

namespace linfs {

namespace {

// Standard static_pointer_cast for std::shared_ptr.
using std::static_pointer_cast;

// Handwritten static_pointer_cast for std::unique_ptr.
template <typename T, typename U>
std::unique_ptr<T>
static_pointer_cast(std::unique_ptr<U>&& u) noexcept {
  return std::unique_ptr<T>(static_cast<T*>(u.release()));
}

uint64_t ToBytes(uint8_t cluster_size) {
  return 1ULL << cluster_size;
}

}  // namespace

void LinFS::Release() {
  delete this;
}

template <typename T, typename... Args>
std::unique_ptr<T> LinFS::AllocateEntry(Args&&... args) {
  Section place = allocator_->AllocateSection(1, accessor_.get());

  std::unique_ptr<T> entry;
  try {
    entry = T::Create(place.data_offset(), place.data_size(),
                      accessor_.get(), std::forward<Args>(args)...);
  }
  catch (...) {
    allocator_->ReleaseSection(place, accessor_.get());
    throw;
  }
  return entry;
}

void LinFS::ReleaseEntry(std::unique_ptr<Entry>& entry) noexcept {
  allocator_->ReleaseSection(entry->section_offset(), accessor_.get());
  entry.reset();
}

std::shared_ptr<DirectoryEntry> LinFS::GetDirectory(Path path, ErrorCode& error_code) {
  std::shared_ptr<DirectoryEntry> dir = root_entry_, next_dir;

  for (; !path.Empty(); dir = std::move(next_dir)) {
    std::shared_lock<SharedMutex> lock = dir->LockShared();

    std::unique_ptr<Entry> entry = dir->FindEntryByName(path.FirstName(),
                                                        accessor_.get());
    if (entry == nullptr) {
      error_code = ErrorCode::kErrorNotFound;
      return nullptr;
    }
    if (entry->type() == Entry::Type::kSymlink) {
      // In despite of the fact that SymlinkEntry has its own lock mechanism
      // we always use the lock of the parent directory.
      Path target = entry->As<SymlinkEntry>()->GetTarget(accessor_.get());

      // Search the resolved |path| starting from the root directory.
      next_dir = root_entry_;
      path = target / path.ExceptFirstName();
      continue;
    }
    if (entry->type() != Entry::Type::kDirectory) {
      error_code = ErrorCode::kErrorNotDirectory;
      return nullptr;
    }

    next_dir = static_pointer_cast<DirectoryEntry>(cache_.GetSharedEntry(std::move(entry)));
    path = path.ExceptFirstName();
  }

  return dir;
}

ErrorCode LinFS::Load(const char* device_path) {
  if (accessor_)
    // Find a way to say that filesystem has already been loaded.
    return ErrorCode::kErrorBusy;

  try {
    accessor_.reset(new ReaderWriter(device_path, std::ios_base::in | std::ios_base::out));

    ErrorCode error_code;
    DeviceLayout::Header header = DeviceLayout::ParseHeader(accessor_.get(), error_code);
    if (error_code != ErrorCode::kSuccess) {
      accessor_.reset();
      return error_code;
    }

    std::unique_ptr<NoneEntry> none_entry = static_pointer_cast<NoneEntry>(
        Entry::Load(header.none_entry_offset, accessor_.get()));
    allocator_ = std::make_unique<SectionAllocator>(ToBytes(header.cluster_size_log2),
                                                    uint64_t(header.total_clusters),
                                                    std::move(none_entry));
    root_entry_ = static_pointer_cast<DirectoryEntry>(
        Entry::Load(header.root_entry_offset, accessor_.get()));
    return ErrorCode::kSuccess;
  }
  catch (...) {
    accessor_.reset();
    allocator_.reset();
    root_entry_.reset();
    return ExceptionHandler::ToErrorCode(std::current_exception());
  }
}

ErrorCode LinFS::Format(const char* device_path, ClusterSize cluster_size) const {
  try {
    std::unique_ptr<ReaderWriter> writer(
        new ReaderWriter(device_path, std::ios_base::out | std::ios_base::trunc));

    DeviceLayout::Header header(cluster_size);
    DeviceLayout::WriteHeader(header, writer.get());

    DeviceLayout::Body body(header);
    NoneEntry::Create(header.none_entry_offset, sizeof body.none_entry, writer.get());
    Section root_section =
        Section::Create(header.root_entry_offset - sizeof body.root.section,
                        body.root.section.size, writer.get());
    DirectoryEntry::Create(header.root_entry_offset, root_section.data_size(),
                           writer.get(), "/");
    return ErrorCode::kSuccess;
  }
  catch (...) {
    return ExceptionHandler::ToErrorCode(std::current_exception());
  }
}

FileInterface* LinFS::OpenFile(const char* path_cstr, bool creat_excl, ErrorCode* error_code) {
  try {
    Path path = Path::Normalize(path_cstr, *error_code);
    if (*error_code != ErrorCode::kSuccess)
      return nullptr;

    // Unlike other entries, FileEntry should follow for the symlinks.  This
    // can be done by resolving symbolic links in the loop.
    while (1) {
      std::shared_ptr<DirectoryEntry> cwd = GetDirectory(path.DirectoryName(), *error_code);
      if (*error_code != ErrorCode::kSuccess)
        return nullptr;

      std::unique_lock<SharedMutex> lock = cwd->Lock();

      std::unique_ptr<Entry> entry = cwd->FindEntryByName(path.BaseName(), accessor_.get());
      if (!entry) {
        entry = AllocateEntry<FileEntry>(path.BaseName());
        try {
          cwd->AddEntry(entry.get(), accessor_.get(), allocator_.get());
        } catch (...) {
          ReleaseEntry(entry);
          throw;
        }
      }
      else if (entry->type() == Entry::Type::kSymlink) {
        // It's a symlink.  Start again.
        path = entry->As<SymlinkEntry>()->GetTarget(accessor_.get());
        continue;
      }
      else if (entry->type() != Entry::Type::kFile) {
        *error_code = ErrorCode::kErrorIsDirectory;
        return nullptr;
      }
      else if (creat_excl) {
        *error_code = ErrorCode::kErrorExists;
        return nullptr;
      }

      std::shared_ptr<FileEntry> shared_file =
          static_pointer_cast<FileEntry>(cache_.GetSharedEntry(std::move(entry)));
      return new FileImpl(shared_file, accessor_->Duplicate(), allocator_.get());
    }
  }
  catch (...) {
    *error_code = ExceptionHandler::ToErrorCode(std::current_exception());
    return nullptr;
  }
}

ErrorCode LinFS::RemoveFile(const char* path_cstr) {
  ErrorCode error_code;

  try {
    Path path = Path::Normalize(path_cstr, error_code);
    if (error_code != ErrorCode::kSuccess)
      return error_code;

    std::shared_ptr<DirectoryEntry> cwd = GetDirectory(path.DirectoryName(), error_code);
    if (error_code != ErrorCode::kSuccess)
      return error_code;

    std::unique_lock<SharedMutex> lock = cwd->Lock();

    std::unique_ptr<Entry> entry = cwd->FindEntryByName(path.BaseName(), accessor_.get());
    if (entry == nullptr)
      return ErrorCode::kErrorNotFound;
    if (entry->type() != Entry::Type::kFile)
      return ErrorCode::kErrorIsDirectory;
    if (cache_.EntryIsShared(entry.get()))
      return ErrorCode::kErrorBusy;
    bool success = cwd->RemoveEntry(entry.get(), accessor_.get(), allocator_.get());
    if (!success)
      return ErrorCode::kErrorFormat;
    ReleaseEntry(entry);
    return ErrorCode::kSuccess;
  }
  catch (...) {
    return ExceptionHandler::ToErrorCode(std::current_exception());
  }
}

ErrorCode LinFS::CreateDirectory(const char* path_cstr) {
  ErrorCode error_code;

  try {
    Path path = Path::Normalize(path_cstr, error_code);
    if (error_code != ErrorCode::kSuccess)
      return error_code;

    std::shared_ptr<DirectoryEntry> cwd = GetDirectory(path.DirectoryName(), error_code);
    if (error_code != ErrorCode::kSuccess)
      return error_code;

    std::unique_lock<SharedMutex> lock = cwd->Lock();

    if (cwd->FindEntryByName(path.BaseName(), accessor_.get()))
      return ErrorCode::kErrorExists;

    std::unique_ptr<Entry> directory = AllocateEntry<DirectoryEntry>(path.BaseName());
    try {
      cwd->AddEntry(directory.get(), accessor_.get(), allocator_.get());
    } catch (...) {
      ReleaseEntry(directory);
      throw;
    }
    return ErrorCode::kSuccess;
  }
  catch (...) {
    return ExceptionHandler::ToErrorCode(std::current_exception());
  }
}

ErrorCode LinFS::RemoveDirectory(const char* path_cstr) {
  ErrorCode error_code;

  try {
    Path path = Path::Normalize(path_cstr, error_code);
    if (error_code != ErrorCode::kSuccess)
      return error_code;

    std::shared_ptr<DirectoryEntry> cwd = GetDirectory(path.DirectoryName(), error_code);
    if (error_code != ErrorCode::kSuccess)
      return error_code;

    std::unique_lock<SharedMutex> lock = cwd->Lock();

    std::unique_ptr<Entry> entry = cwd->FindEntryByName(path.BaseName(), accessor_.get());
    if (entry == nullptr)
      return ErrorCode::kErrorNotFound;
    if (entry->type() != Entry::Type::kDirectory)
      return ErrorCode::kErrorNotDirectory;
    if (cache_.EntryIsShared(entry.get()))
      return ErrorCode::kErrorBusy;

    // Safe to check without locking the entry's lock |entry->Lock()|
    // because no one refers to (and uses) it at the moment.
    bool has_entries = entry->As<DirectoryEntry>()->HasEntries(accessor_.get());
    if (has_entries)
      return ErrorCode::kErrorDirectoryNotEmpty;

    bool success = cwd->RemoveEntry(entry.get(), accessor_.get(), allocator_.get());
    if (!success)
      return ErrorCode::kErrorFormat;
    ReleaseEntry(entry);
    return ErrorCode::kSuccess;
  }
  catch (...) {
    return ExceptionHandler::ToErrorCode(std::current_exception());
  }
}

uint64_t LinFS::ListDirectory(const char* path_cstr, uint64_t cookie,
                              char* next_buf, ErrorCode* error_code) {
  try {
    Path path = Path::Normalize(path_cstr, *error_code);
    if (*error_code != ErrorCode::kSuccess)
      return 0;

    std::shared_ptr<DirectoryEntry> cwd = GetDirectory(path, *error_code);
    if (*error_code != ErrorCode::kSuccess)
      return 0;

    std::shared_lock<SharedMutex> lock = cwd->LockShared();
    return cwd->GetNextEntryName(cookie, accessor_.get(), next_buf);
  }
  catch (...) {
    *error_code = ExceptionHandler::ToErrorCode(std::current_exception());
    return 0;
  }
}

ErrorCode LinFS::CreateSymlink(const char* path_cstr, const char* target_cstr) {
  ErrorCode error_code;

  try {
    Path path = Path::Normalize(path_cstr, error_code);
    if (error_code != ErrorCode::kSuccess)
      return error_code;

    Path target = Path::Normalize(target_cstr, error_code);
    if (error_code != ErrorCode::kSuccess)
      return error_code;

    std::shared_ptr<DirectoryEntry> cwd = GetDirectory(path.DirectoryName(), error_code);
    if (error_code != ErrorCode::kSuccess)
      return error_code;

    std::unique_lock<SharedMutex> lock = cwd->Lock();

    if (cwd->FindEntryByName(path.BaseName(), accessor_.get()))
      return ErrorCode::kErrorExists;

    std::unique_ptr<Entry> symlink = AllocateEntry<SymlinkEntry>(
        path.BaseName(), target.Normalized(), allocator_.get());
    try {
      cwd->AddEntry(symlink.get(), accessor_.get(), allocator_.get());
    } catch (...) {
      ReleaseEntry(symlink);
      throw;
    }
    return ErrorCode::kSuccess;
  }
  catch (...) {
    return ExceptionHandler::ToErrorCode(std::current_exception());
  }
}

}  // namespace linfs

}  // namespace fs
