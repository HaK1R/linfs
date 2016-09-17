#include "lib/linfs.h"

#include <cstddef>
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

  std::unique_ptr<T> entry = T::Create(place.data_offset(), &accessor_, error_code, std::forward<Args>(args)...);
  if (!entry)
    allocator_->ReleaseSection(place, &accessor_);
  return entry;
}

void LinFS::ReleaseEntry(std::shared_ptr<Entry> entry) {
  allocator_->ReleaseSection(entry->section_offset(), &accessor_);
}

std::shared_ptr<DirectoryEntry> LinFS::GetDirectory(Path path, ErrorCode& error_code) {
  std::shared_ptr<DirectoryEntry> dir = root_entry_;
  while (!path.Empty()) {
    std::shared_ptr<Entry> entry = dir->FindEntryByName(path.FirstName(), &accessor_, error_code);
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
  ErrorCode error_code = accessor_.Open(device_path, std::ios_base::in | std::ios_base::out);
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

  // TODO () or {}?
  DeviceLayout::Header header(cluster_size);
  error_code = DeviceLayout::WriteHeader(&writer, header);
  if (error_code != ErrorCode::kSuccess)
    return error_code;
  //DeviceLayout::Body body(header);

  NoneEntry::Create(header.none_entry_offset, &writer, error_code);
  if (error_code != ErrorCode::kSuccess)
    return error_code;

  error_code = writer.SaveSection(Section(sizeof header + offsetof(DeviceLayout::Body, root),
                                          (1 << static_cast<uint8_t>(cluster_size)) - sizeof header - offsetof(DeviceLayout::Body, root), 0)); // TODO compile
  if (error_code != ErrorCode::kSuccess)
    return error_code;

  DirectoryEntry::Create(header.root_entry_offset, &writer, error_code, "/");
  if (error_code != ErrorCode::kSuccess)
    return error_code;

  return ErrorCode::kSuccess;
}

IFile* LinFS::OpenFile(const char *path_cstr, ErrorCode& error_code) {
  Path path = Path::Normalize(path_cstr, error_code);
  if (error_code != ErrorCode::kSuccess)
    return nullptr;

  std::shared_ptr<DirectoryEntry> cwd = GetDirectory(path.DirectoryName(), error_code);
  if (error_code != ErrorCode::kSuccess)
    return nullptr;

  std::shared_ptr<FileEntry> file_entry = static_pointer_cast<FileEntry>(cwd->FindEntryByName(path.BaseName(), &accessor_, error_code));
  // TODO add checks if it's not a file
  if (error_code == ErrorCode::kErrorNotFound) {
    file_entry = AllocateEntry<FileEntry>(error_code, path.BaseName());
    if (error_code != ErrorCode::kSuccess)
      return nullptr;

    error_code = cwd->AddEntry(static_pointer_cast<Entry>(file_entry), &accessor_, allocator_.get());
    if (error_code != ErrorCode::kSuccess) {
      ReleaseEntry(file_entry);
      return nullptr;
    }
  }
  if (error_code != ErrorCode::kSuccess)
    return nullptr;

  IFile* file = new (std::nothrow) FileImpl(file_entry, &accessor_, allocator_.get());
  if (file == nullptr)
    error_code = ErrorCode::kErrorNoMemory;

  return file;
}

ErrorCode LinFS::RemoveFile(const char *path_cstr) {
  (void)path_cstr;
  // TODO compile
  return ErrorCode::kErrorNotSupported;
}

ErrorCode LinFS::CreateDirectory(const char *path_cstr) {
  ErrorCode error_code;

  Path path = Path::Normalize(path_cstr, error_code);
  if (error_code != ErrorCode::kSuccess)
    return error_code;

  std::shared_ptr<DirectoryEntry> cwd = GetDirectory(path.DirectoryName(), error_code);
  if (error_code != ErrorCode::kSuccess)
    return error_code;

  if (cwd->FindEntryByName(path.BaseName(), &accessor_, error_code))
    return ErrorCode::kErrorExist;

  std::shared_ptr<DirectoryEntry> new_dir = AllocateEntry<DirectoryEntry>(error_code, path.BaseName());
  if (error_code != ErrorCode::kSuccess)
    return error_code;

  error_code = cwd->AddEntry(static_pointer_cast<Entry>(new_dir), &accessor_, allocator_.get());
  if (error_code != ErrorCode::kSuccess) {
    ReleaseEntry(new_dir);
    return error_code;
  }

  return ErrorCode::kSuccess;
}

ErrorCode LinFS::RemoveDirectory(const char *path_cstr) {
  ErrorCode error_code;

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

  error_code = cwd->RemoveEntry(entry, &accessor_, allocator_.get());
  if (error_code != ErrorCode::kSuccess)
    return error_code;

  ReleaseEntry(entry);

  return ErrorCode::kSuccess;
}

const char* LinFS::ListDirectory(const char *path_cstr, const char *prev,
                          char *next_buf, ErrorCode& error_code) {
  Path path = Path::Normalize(path_cstr, error_code);
  if (error_code != ErrorCode::kSuccess)
    return nullptr;

  std::shared_ptr<DirectoryEntry> cwd = GetDirectory(path, error_code);
  if (error_code != ErrorCode::kSuccess)
    return nullptr;

  error_code = cwd->GetNextEntryName(prev, &accessor_, next_buf);
  if (error_code != ErrorCode::kSuccess)
    return nullptr;

  return next_buf;
}

}  // namespace linfs

}  // namespace fs
