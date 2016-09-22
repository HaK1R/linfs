#include "lib/linfs.h"

#include <cstddef>
#include <mutex>
#include <utility>

#include "lib/entries/file_entry.h"
#include "lib/file_impl.h"
#include "lib/layout/device_layout.h"

namespace fs {

namespace linfs {

namespace {

// Standard static_pointer_cast for std::shared_ptr.
using std::static_pointer_cast;

// Handwritten static_pointer_cast for std::unique_ptr.
template<typename T, typename U>
std::unique_ptr<T>
static_pointer_cast(std::unique_ptr<U>&& u) {
  return std::unique_ptr<T>(static_cast<T*>(u.release()));
}

}  // namespace

void LinFS::Release() {
  delete this;
}

template<typename T, typename... Args>
std::unique_ptr<T> LinFS::AllocateEntry(ErrorCode& error_code, Args&&... args) {
  Section place = allocator_->AllocateSection(1, &accessor_, error_code);
  if (error_code != ErrorCode::kSuccess) return nullptr;

  std::unique_ptr<T> entry = T::Create(place.data_offset(), place.data_size(),
                                       &accessor_, error_code, std::forward<Args>(args)...);
  if (!entry)
    allocator_->ReleaseSection(place, &accessor_);
  return entry;
}

void LinFS::ReleaseEntry(std::shared_ptr<Entry> entry) {
    // TODO? use unique_ptr
  allocator_->ReleaseSection(entry->section_offset(), &accessor_);
}

std::shared_ptr<DirectoryEntry> LinFS::GetDirectory(Path path, ErrorCode& error_code) {
  std::shared_ptr<DirectoryEntry> dir = root_entry_;
  while (!path.Empty()) {
    std::shared_ptr<Entry> entry = dir->FindEntryByName(path.FirstName(), &accessor_, error_code);
    if (error_code != ErrorCode::kSuccess)
      return nullptr;
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
  ErrorCode error_code = accessor_.Open(device_path, std::ios_base::in | std::ios_base::out | std::ios_base::ate);
  if (error_code != ErrorCode::kSuccess)
    return error_code;

  DeviceLayout::Header header;
  error_code = DeviceLayout::ParseHeader(&accessor_, header);
  if (error_code != ErrorCode::kSuccess)
    return error_code;

  std::unique_ptr<NoneEntry> none_entry = static_pointer_cast<NoneEntry>(accessor_.LoadEntry(header.none_entry_offset, error_code));
  if (error_code != ErrorCode::kSuccess)
    return error_code;
  allocator_ = std::make_unique<SectionAllocator>((1 << header.cluster_size_log2),
                                                  uint64_t(header.total_clusters), std::move(none_entry));

  root_entry_ = static_pointer_cast<DirectoryEntry>(accessor_.LoadEntry(header.root_entry_offset, error_code));
  if (error_code != ErrorCode::kSuccess)
    return error_code;

  return ErrorCode::kSuccess;
}

ErrorCode LinFS::Format(const char *device_path, ClusterSize cluster_size) const {
  ReaderWriter writer;
  ErrorCode error_code = writer.Open(device_path, std::ios_base::out | std::ios_base::trunc);
  if (error_code != ErrorCode::kSuccess)
    return error_code;

  DeviceLayout::Header header(cluster_size);
  error_code = DeviceLayout::WriteHeader(header, &writer);
  if (error_code != ErrorCode::kSuccess)
    return error_code;

  DeviceLayout::Body body(header);

  NoneEntry::Create(header.none_entry_offset, sizeof body.none_entry, &writer, error_code);
  if (error_code != ErrorCode::kSuccess)
    return error_code;

  Section root_section(header.root_entry_offset - sizeof body.root.section,
                       body.root.section.size, body.root.section.next_offset);
  error_code = writer.SaveSection(root_section);
  if (error_code != ErrorCode::kSuccess)
    return error_code;

  DirectoryEntry::Create(header.root_entry_offset, root_section.data_size(),
                         &writer, error_code, "/");
  if (error_code != ErrorCode::kSuccess)
    return error_code;

  return ErrorCode::kSuccess;
}

IFile* LinFS::OpenFile(const char *path_cstr, ErrorCode& error_code) {
  std::lock_guard<std::mutex> lock(mutex_);

  Path path = Path::Normalize(path_cstr, error_code);
  if (error_code != ErrorCode::kSuccess)
    return nullptr;

  std::shared_ptr<DirectoryEntry> cwd = GetDirectory(path.DirectoryName(), error_code);
  if (error_code != ErrorCode::kSuccess)
    return nullptr;

  std::unique_ptr<Entry> file = cwd->FindEntryByName(path.BaseName(), &accessor_, error_code);
  if (error_code == ErrorCode::kErrorNotFound) {
    file = AllocateEntry<FileEntry>(error_code, path.BaseName());
    if (error_code != ErrorCode::kSuccess)
      return nullptr;

    error_code = cwd->AddEntry(file.get(), &accessor_, allocator_.get());
    if (error_code != ErrorCode::kSuccess) {
      ReleaseEntry(std::shared_ptr<Entry>(file.release()));
      return nullptr;
    }
  }
  else if (error_code != ErrorCode::kSuccess)
    return nullptr;
  else if (file->type() == Entry::Type::kDirectory) {
    error_code = ErrorCode::kErrorIsDirectory;
    return nullptr;
  }

  std::shared_ptr<FileEntry> shared_file = static_pointer_cast<FileEntry>(cache_.GetSharedEntry(std::move(file), error_code));
  if (error_code != ErrorCode::kSuccess)
    return nullptr;

  IFile* file_desc = new (std::nothrow) FileImpl(shared_file, &accessor_, allocator_.get());
  if (file_desc == nullptr)
    error_code = ErrorCode::kErrorNoMemory;

  return file_desc;
}

ErrorCode LinFS::RemoveFile(const char *path_cstr) {
  ErrorCode error_code;
  std::lock_guard<std::mutex> lock(mutex_);

  Path path = Path::Normalize(path_cstr, error_code);
  if (error_code != ErrorCode::kSuccess)
    return error_code;

  std::shared_ptr<DirectoryEntry> cwd = GetDirectory(path.DirectoryName(), error_code);
  if (error_code != ErrorCode::kSuccess)
    return error_code;

  std::shared_ptr<Entry> entry = cwd->FindEntryByName(path.BaseName(), &accessor_, error_code);
  if (error_code != ErrorCode::kSuccess)
    return error_code;
  if (entry->type() != Entry::Type::kFile)
    return ErrorCode::kErrorIsDirectory;

  error_code = cwd->RemoveEntry(entry.get(), &accessor_, allocator_.get());
  if (error_code != ErrorCode::kSuccess)
    return error_code;

  ReleaseEntry(entry);

  return ErrorCode::kSuccess;
}

ErrorCode LinFS::CreateDirectory(const char *path_cstr) {
  ErrorCode error_code;
  std::lock_guard<std::mutex> lock(mutex_);

  Path path = Path::Normalize(path_cstr, error_code);
  if (error_code != ErrorCode::kSuccess)
    return error_code;

  std::shared_ptr<DirectoryEntry> cwd = GetDirectory(path.DirectoryName(), error_code);
  if (error_code != ErrorCode::kSuccess)
    return error_code;

  if (cwd->FindEntryByName(path.BaseName(), &accessor_, error_code))
    return ErrorCode::kErrorExists;

  std::shared_ptr<DirectoryEntry> new_dir = AllocateEntry<DirectoryEntry>(error_code, path.BaseName());
  if (error_code != ErrorCode::kSuccess)
    return error_code;

  error_code = cwd->AddEntry(new_dir.get(), &accessor_, allocator_.get());
  if (error_code != ErrorCode::kSuccess) {
    ReleaseEntry(new_dir);
    return error_code;
  }

  return ErrorCode::kSuccess;
}

ErrorCode LinFS::RemoveDirectory(const char *path_cstr) {
  ErrorCode error_code;
  std::lock_guard<std::mutex> lock(mutex_);

  Path path = Path::Normalize(path_cstr, error_code);
  if (error_code != ErrorCode::kSuccess)
    return error_code;

  std::shared_ptr<DirectoryEntry> cwd = GetDirectory(path.DirectoryName(), error_code);
  if (error_code != ErrorCode::kSuccess)
    return error_code;

  std::shared_ptr<Entry> entry = cwd->FindEntryByName(path.BaseName(), &accessor_, error_code);
  if (error_code != ErrorCode::kSuccess)
    return error_code;
  if (entry->type() != Entry::Type::kDirectory)
    return ErrorCode::kErrorNotDirectory;

  // TODO? static_cast<>
  bool has_entries = static_pointer_cast<DirectoryEntry>(entry)->HasEntries(&accessor_, error_code);
  if (error_code != ErrorCode::kSuccess)
    return error_code;
  if (has_entries)
    return ErrorCode::kErrorDirectoryNotEmpty;

  error_code = cwd->RemoveEntry(entry.get(), &accessor_, allocator_.get());
  if (error_code != ErrorCode::kSuccess)
    return error_code;

  ReleaseEntry(entry);

  return ErrorCode::kSuccess;
}

const char* LinFS::ListDirectory(const char *path_cstr, const char *prev,
                          char *next_buf, ErrorCode& error_code) {
  std::lock_guard<std::mutex> lock(mutex_);

  Path path = Path::Normalize(path_cstr, error_code);
  if (error_code != ErrorCode::kSuccess)
    return nullptr;

  std::shared_ptr<DirectoryEntry> cwd = GetDirectory(path, error_code);
  if (error_code != ErrorCode::kSuccess)
    return nullptr;

  error_code = cwd->GetNextEntryName(prev, &accessor_, next_buf);
  if (error_code != ErrorCode::kSuccess) {
    if (error_code == ErrorCode::kErrorNotFound)
      error_code = ErrorCode::kSuccess;
    return nullptr;
  }

  return next_buf;
}

}  // namespace linfs

}  // namespace fs
