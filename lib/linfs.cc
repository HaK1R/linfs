#include "lib/linfs.h"

#include <cstddef>
#include <utility>

#include "lib/entries/file_entry.h"
#include "lib/file_impl.h"
#include "lib/layout/device_format.h"

namespace fs {

namespace linfs {

LinFS::LinFS() {
}

LinFS::~LinFS() {
}

LinFS::Release() {
  delete this;
}

template<typename T>
std::unique_ptr<T> LinFS::AllocateEntry<T>(ErrorCode& error_code, Args&&... args) {
  Section place = allocator_->AllocateSection(error_code);
  if (error_code != ErrorCode::kSuccess) return nullptr;

  std::unique_ptr<T> entry = T::Create(place.data_offset(), &accessor_, error_code, std::forward<Args>(args)...);
  if (!entry)
    allocate_.ReleaseSection(place);
  return entry;
}

void LinFS::ReleaseEntry(std::shared_ptr<Entry> entry) {
  allocate_.ReleaseSection(Section(entry->section_offset(), cluster_size_, 0));
}

std::shared_ptr<DirectoryEntry> LinFS::GetDirectory(Path path, ErrorCode& error_code) {
  std::shared_ptr<DirectoryEntry> cwd = root_entry_;
  while (!path.Empty()) {
    std::shared_ptr<Entry> entry = dir->FindEntryByName(this, path.FirstName(), error_code);
    if (entry->type() != kDirectory)
      return ErrorCode::kErrorNotDirectory;

    dir = std::static_pointer_cast<DirectoryEntry>(entry);
    path = path.ExceptFirstName();
  }

  return dir;
}

LinFS::Load(const char *device_path) {
  ErrorCode error_code = accessor_.Open(device_path, std::ios_base::in | std::ios_base::out);
  if (error_code != ErrorCode::kSuccess)
    return error_code;

  DeviceLayout::Header header;
  error_code = DeviceLayout::ParseHeader(&accessor_, header, error_code);
  if (error_code != ErrorCode::kSuccess)
    return error_code;

  std::unique_ptr<NoneEntry> none_entry = accessor_.LoadEntry(header.none_entry_offset, error_code);
  if (error_code != ErrorCode::kSuccess)
    return error_code;
  allocator_ = std::make_unique<SectionAllocator>((1 << header.cluster_size),
                                                  header.total_clusters, std::move(none_entry));

  root_entry_ = accessor_.LoadEntry(header.root_entry_offset, error_code);
  if (error_code != ErrorCode::kSuccess)
    return error_code;
}

int LinFS::Format(const char *device_path, ClusterSize cluster_size) {
  ReaderWriter writer;
  ErrorCode error_code = writer.Open(device_path, std::ios_base::out | std::ios_base::trunc);
  if (error_code != ErrorCode::kSuccess)
    return error_code;

  DeviceLayout::Header header(cluster_size);
  error_code = DeviceLayout::Write(writer, header);
  if (error_code != ErrorCode::kSuccess)
    return error_code;
  //DeviceLayout::Body body(header);

  NoneEntry::Create(header.none_entry_offset, &writer, error_code);
  if (error_code != ErrorCode::kSuccess)
    return error_code;

  error_code = writer->SaveSection(Section(sizeof header + offsetof(DeviceLayout::Body, root),
                                           (1 << cluster_size) - sizeof header - offsetof(DeviceLayout::Body, root), 0));
  if (error_code != ErrorCode::kSuccess)
    return error_code;

  DirectoryEntry::Create(header.root_entry_offset, error_code, 0);
  if (error_code != ErrorCode::kSuccess)
    return error_code;

  return ErrorCode::kSuccess;
}

IFile* LinFS::OpenFile(const char *path_cstr, ErrorCode& error_code) override {
  Path path = Path::Normalize(path_cstr, error_code);
  if (error_code != ErrorCode::kSuccess)
    return nullptr;

  std::shared_ptr<DirectoryEntry> cwd = GetDirectory(path.DirectoryName(), error_code);
  if (error_code != ErrorCode::kSuccess)
    return nullptr;

  std::shared_ptr<FileEntry> file_entry = cwd->FindEntryByName(this, path.BaseName(), error_code);
  if (error_code == ErrorCode::kErrorNotFound) {
    file_entry = AllocateEntry<FileEntry>(error_code, path.BaseName());
    if (error_code != ErrorCode::kSuccess)
      return nullptr;

    error_code = cwd->AddEntry(file_entry, &accessor_);
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

ErrorCode LinFS::CreateDirectory(const char *path_cstr) override {
  ErrorCode error_code;

  Path path = Path::Normalize(path_cstr, error_code);
  if (error_code != ErrorCode::kSuccess)
    return error_code;

  std::shared_ptr<DirectoryEntry> cwd = GetDirectory(path.DirectoryName(), error_code);
  if (error_code != ErrorCode::kSuccess)
    return error_code;

  if (cwd->FindEntryByName(this, path.BaseName(), error_code))
    return ErrorCode::kErrorExist;

  std::shared_ptr<DirectoryEntry> new_dir = AllocateEntry<DirectoryEntry>(error_code, path.BaseName());
  if (error_code != ErrorCode::kSuccess)
    return error_code;

  error_code = cwd->AddEntry(new_dir, &accessor_);
  if (error_code != ErrorCode::kSuccess) {
    ReleaseEntry(new_dir);
    return error_code;
  }

  return ErrorCode::kSuccess;
}

ErrorCode LinFS::RemoveDirectory(const char *path_cstr) override {
  ErrorCode error_code;

  Path path = Path::Normalize(path_cstr, error_code);
  if (error_code != ErrorCode::kSuccess)
    return error_code;

  std::shared_ptr<DirectoryEntry> dir = GetDirectory(path, error_code);
  if (error_code != ErrorCode::kSuccess)
    return error_code;
  if (!dir->Empty())
    return ErrorCode::kErrorNotEmpty;

  error_code = cwd->RemoveEntry(dir, &accessor_);
  if (error_code != ErrorCode::kSuccess)
    return error_code;

  ReleaseEntry(dir);

  return ErrorCode::kSuccess;
}

const char* LinFS::ListDirectory(const char *path_cstr, const char *prev,
                          char *next_buf, ErrorCode& error_code) override {
  Path path = Path::Normalize(path_cstr, error_code);
  if (error_code != ErrorCode::kSuccess)
    return error_code;

  DirectoryEntry *cwd = GetDirectory(path, error_code);
  if (error_code != ErrorCode::kSuccess)
    return error_code;

  error_code = cwd->GetNextEntryName(prev, &accessor_, next_buf);
  if (error_code != ErrorCode::kSuccess)
    return nullptr;

  return next_buf;
}

}  // namespace linfs

}  // namespace fs
