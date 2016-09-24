#include "lib/linfs.h"

#include <cstddef>
#include <exception>
#include <mutex>
#include <utility>

#include "lib/entries/file_entry.h"
#include "lib/file_impl.h"
#include "lib/layout/device_layout.h"
#include "lib/util/exception_handler.h"

namespace fs {

namespace linfs {

namespace {

// Standard static_pointer_cast for std::shared_ptr.
using std::static_pointer_cast;

// Handwritten static_pointer_cast for std::unique_ptr.
template<typename T, typename U>
std::unique_ptr<T>
static_pointer_cast(std::unique_ptr<U>&& u) noexcept {
  return std::unique_ptr<T>(static_cast<T*>(u.release()));
}

}  // namespace

void LinFS::Release() {
  delete this;
}

template<typename T, typename... Args>
std::unique_ptr<T> LinFS::AllocateEntry(Args&&... args) {
  Section place = allocator_->AllocateSection(1, &accessor_);

  std::unique_ptr<T> entry;
  try {
    entry = T::Create(place.data_offset(), place.data_size(),
                      &accessor_, std::forward<Args>(args)...);
  } catch (...) {
    allocator_->ReleaseSection(place, &accessor_);
    throw;
  }
  return entry;
}

void LinFS::ReleaseEntry(std::unique_ptr<Entry>& entry) noexcept {
  allocator_->ReleaseSection(entry->section_offset(), &accessor_);
  entry.reset();
}

std::shared_ptr<DirectoryEntry> LinFS::GetDirectory(Path path, ErrorCode& error_code) {
  std::shared_ptr<DirectoryEntry> dir = root_entry_;
  while (!path.Empty()) {
    std::shared_ptr<Entry> entry = dir->FindEntryByName(path.FirstName(), &accessor_);
    if (entry == nullptr) {
      error_code = ErrorCode::kErrorNotFound;
      return nullptr;
    }
    if (entry->type() != Entry::Type::kDirectory) {
      error_code = ErrorCode::kErrorNotDirectory;
      return nullptr;
    }

    dir = static_pointer_cast<DirectoryEntry>(entry);
    path = path.ExceptFirstName();
  }

  return dir;
}

ErrorCode LinFS::Load(const char *device_path) {
  if (root_entry_)
    // |root_entry_| is a perfect indicator that
    // filesystem has already been loaded.
    return ErrorCode::kErrorDeviceUnknown;

  ErrorCode error_code = accessor_.Open(device_path, std::ios_base::in | std::ios_base::out);
  if (error_code != ErrorCode::kSuccess)
    return error_code;

  try {
    DeviceLayout::Header header = DeviceLayout::ParseHeader(&accessor_, error_code);
    if (error_code != ErrorCode::kSuccess)
      return error_code;

    std::unique_ptr<NoneEntry> none_entry = static_pointer_cast<NoneEntry>(accessor_.LoadEntry(header.none_entry_offset));
    allocator_ = std::make_unique<SectionAllocator>((1 << header.cluster_size_log2),
                                                   uint64_t(header.total_clusters), std::move(none_entry));
    root_entry_ = static_pointer_cast<DirectoryEntry>(accessor_.LoadEntry(header.root_entry_offset));
    return ErrorCode::kSuccess;
  } catch (...) {
    allocator_.reset();
    root_entry_.reset();
    return ExceptionHandler::ToErrorCode(std::current_exception());
  }
}

ErrorCode LinFS::Format(const char *device_path, ClusterSize cluster_size) const {
  ReaderWriter writer;

  ErrorCode error_code = writer.Open(device_path, std::ios_base::out | std::ios_base::trunc);
  if (error_code != ErrorCode::kSuccess)
    return error_code;

  try {
    DeviceLayout::Header header(cluster_size);
    DeviceLayout::WriteHeader(header, &writer);

    DeviceLayout::Body body(header);
    NoneEntry::Create(header.none_entry_offset, sizeof body.none_entry, &writer);
    Section root_section(header.root_entry_offset - sizeof body.root.section,
                         body.root.section.size, body.root.section.next_offset);
    writer.SaveSection(root_section);
    DirectoryEntry::Create(header.root_entry_offset, root_section.data_size(),
                           &writer, "/");
    return ErrorCode::kSuccess;
  }
  catch (...) {
    return ExceptionHandler::ToErrorCode(std::current_exception());
  }
}

IFile* LinFS::OpenFile(const char *path_cstr, ErrorCode& error_code) {
  std::lock_guard<std::mutex> lock(mutex_);

  try {
    Path path = Path::Normalize(path_cstr, error_code);
    if (error_code != ErrorCode::kSuccess)
      return nullptr;

    std::shared_ptr<DirectoryEntry> cwd = GetDirectory(path.DirectoryName(), error_code);
    if (error_code != ErrorCode::kSuccess)
      return nullptr;

    std::unique_ptr<Entry> file = cwd->FindEntryByName(path.BaseName(), &accessor_);
    if (!file) {
      file = AllocateEntry<FileEntry>(path.BaseName());
      try {
        cwd->AddEntry(file.get(), &accessor_, allocator_.get());
      } catch (...) {
        ReleaseEntry(file);
        throw;
      }
    }
    else if (file->type() == Entry::Type::kDirectory) {
      error_code = ErrorCode::kErrorIsDirectory;
      return nullptr;
    }
    std::shared_ptr<FileEntry> shared_file = static_pointer_cast<FileEntry>(cache_.GetSharedEntry(std::move(file)));
    return new FileImpl(shared_file, &accessor_, allocator_.get());
  } catch (...) {
    error_code = ExceptionHandler::ToErrorCode(std::current_exception());
    return nullptr;
  }
}

ErrorCode LinFS::RemoveFile(const char *path_cstr) {
  ErrorCode error_code;
  std::lock_guard<std::mutex> lock(mutex_);

  try {
    Path path = Path::Normalize(path_cstr, error_code);
    if (error_code != ErrorCode::kSuccess)
      return error_code;

    std::shared_ptr<DirectoryEntry> cwd = GetDirectory(path.DirectoryName(), error_code);
    if (error_code != ErrorCode::kSuccess)
      return error_code;

    std::unique_ptr<Entry> entry = cwd->FindEntryByName(path.BaseName(), &accessor_);
    if (entry == nullptr)
      return ErrorCode::kErrorNotFound;
    if (entry->type() != Entry::Type::kFile)
      return ErrorCode::kErrorIsDirectory;
    if (cache_.EntryIsShared(entry.get()))
      return ErrorCode::kErrorFileBusy;
    bool success = cwd->RemoveEntry(entry.get(), &accessor_, allocator_.get());
    if (!success)
      return ErrorCode::kErrorFormat;
    ReleaseEntry(entry);
    return ErrorCode::kSuccess;
  } catch (...) {
    return ExceptionHandler::ToErrorCode(std::current_exception());
  }
}

ErrorCode LinFS::CreateDirectory(const char *path_cstr) {
  ErrorCode error_code;
  std::lock_guard<std::mutex> lock(mutex_);

  try {
    Path path = Path::Normalize(path_cstr, error_code);
    if (error_code != ErrorCode::kSuccess)
      return error_code;

    std::shared_ptr<DirectoryEntry> cwd = GetDirectory(path.DirectoryName(), error_code);
    if (error_code != ErrorCode::kSuccess)
      return error_code;

    if (cwd->FindEntryByName(path.BaseName(), &accessor_))
      return ErrorCode::kErrorExists;

    std::unique_ptr<Entry> directory = AllocateEntry<DirectoryEntry>(path.BaseName());
    try {
      cwd->AddEntry(directory.get(), &accessor_, allocator_.get());
    } catch (...) {
      ReleaseEntry(directory);
      throw;
    }
    return ErrorCode::kSuccess;
  } catch (...) {
    return ExceptionHandler::ToErrorCode(std::current_exception());
  }
}

ErrorCode LinFS::RemoveDirectory(const char *path_cstr) {
  ErrorCode error_code;
  std::lock_guard<std::mutex> lock(mutex_);

  try {
    Path path = Path::Normalize(path_cstr, error_code);
    if (error_code != ErrorCode::kSuccess)
      return error_code;

    std::shared_ptr<DirectoryEntry> cwd = GetDirectory(path.DirectoryName(), error_code);
    if (error_code != ErrorCode::kSuccess)
      return error_code;

    std::unique_ptr<Entry> entry = cwd->FindEntryByName(path.BaseName(), &accessor_);
    if (entry == nullptr)
      return ErrorCode::kErrorNotFound;
    if (entry->type() != Entry::Type::kDirectory)
      return ErrorCode::kErrorNotDirectory;

    bool has_entries = static_cast<DirectoryEntry*>(entry.get())->HasEntries(&accessor_);
    if (has_entries)
      return ErrorCode::kErrorDirectoryNotEmpty;

    bool success = cwd->RemoveEntry(entry.get(), &accessor_, allocator_.get());
    if (!success)
      return ErrorCode::kErrorFormat;
    ReleaseEntry(entry);
    return ErrorCode::kSuccess;
  } catch (...) {
    return ExceptionHandler::ToErrorCode(std::current_exception());
  }
}

const char* LinFS::ListDirectory(const char *path_cstr, const char *prev,
                                 char *next_buf, ErrorCode* error_code) {
  std::lock_guard<std::mutex> lock(mutex_);

  try {
    Path path = Path::Normalize(path_cstr, *error_code);
    if (*error_code != ErrorCode::kSuccess)
      return nullptr;

    std::shared_ptr<DirectoryEntry> cwd = GetDirectory(path, *error_code);
    if (*error_code != ErrorCode::kSuccess)
      return nullptr;

    return cwd->GetNextEntryName(prev, &accessor_, next_buf);
  } catch (...) {
    *error_code = ExceptionHandler::ToErrorCode(std::current_exception());
    return nullptr;
  }
}

}  // namespace linfs

}  // namespace fs
