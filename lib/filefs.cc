#include <cstddef>

#include "include/interfaces/IFileSystem.h"
#include "include/FileFS.h"
#include "lib/layout/device_format.h"

namespace fs {

namespace ffs {

FileFS::FileFS() {
}

FileFS::~FileFS() {
}

FileFS::Release() {
  delete this;
}

uint64_t FileFS::AllocateCluster(ErrorCode& error_code) {
  if (none_entry_) {
    if (none_entry_->size() > cluster_size_) {
      device_ << Entry;
    }
    none_entry_->next_offset();
    ;;
    return address;
  }

  // There is nothing in NoneEntry chain. Allocate a new cluster.
  uint64_t cluster_offset = total_clusters_ * cluster_size_;
  device_.
  device_.seekp(cluster_offset + cluster_size_ - 1);
  device_.write("", 1);
  total_clusters_++;
  return cluster_offset;
}

template<typename T>
std::shared_ptr<T> FileFS::AllocateEntry<T>(ErrorCode& error_code, Args&&... args) {
  uint64_t entry_offset = AllocateCluster(error_code);
  if (error_code != ErrorCode::kSuccess) return error_code;
  std::shared_ptr<T> entry = T::CreateEntry(error_code, entry_offset, std::forward<Args>(args)...);
  if (!entry) {
    ErrorCode release_error_code;
    ReleaseCluster(entry_offset, release_error_code);
    if (release_error_code)
        std::cerr << "Can't release a cluster; it leaked" << std::endl;
  }
  return error_code;
}

Entry* FileFS::LoadEntry(uint64_t offset) {
  EntryLayout::Header entry_header;
  device_.seekg(offset);
  device_.read(&entry_header, sizeof entry_header);

  switch (static_cast<Entry::Type>(entry_header.type)) {
    case Entry::Type::kDirectory:
      return new DirectoryEntry(offset, std::string(entry_header.name, sizeof entry_header.name));
    case Entry::Type::File:
      return new FileEntry(offset, std::string(entry_header.name, sizeof entry_header.name));
    case Entry::Type::kNone:
      return new NoneEntry(offset);
  }
}

std::shared_ptr<DirectoryEntry> FileFS::GetDirectory(Path path, ErrorCode& error_code) {
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

FileFS::Load(const char *device_path) {
  device_.open(device_path, std::ios::binary | std::ios::in | std::ios::out);
  if (!device.is_open())
    return -1;

  DeviceLayout::Header device_header;
  uint16_t root_entry_offset;
  DeviceLayout::ParseHeader(device_, cluster_size_, root_entry_offset);

  root_entry_ = LoadEntry<DirectoryEntry>(root_entry_offset);
  if (none_entry_offset)
    none_entry_ = LoadEntry<DirectoryEntry>(root_entry_offset);
}

int FileFS::Format(const char *device_path, uint64_t cluster_size) {
  std::fstream device(device_path, std::ios::binary | std::ios::out | std::ios::trunc);
  if (!device.is_open())
    return -1;

  device << DeviceLayout::Header(ClusterSize::k4Kb, sizeof(DeviceLayout::Header), 0)
         << EntryLayout::Header(Entry::Type::kDirectory, "/")
         << SectionLayout::Header(cluster_size - sizeof(DeviceLayout::Header) - sizeof(EntryLayout::Header), 0,
                                  cluster_size - sizeof(DeviceLayout::Header) - sizeof(EntryLayout::Header) - sizeof(SectionLayout::Header));

  device.close();
  return 0;
}

int FileFS::CreateDirectory(const char *path_cstr) override {
  ErrorCode error_code;

  Path path = Path::Normalize(path_cstr, error_code);
  std::shared_ptr<DirectoryEntry> cwd = GetDirectory(path.DirectoryName(), error_code);
  if (cwd->FindEntryByName(this, path.BaseName(), error_code))
    return ErrorCode::kErrorExist;

  std::shared_ptr<DirectoryEntry> new_dir = AllocateEntry<DirectoryEntry>(error_code, path.BaseName());
  cwd->AddEntry(new_dir->base_offset(), error_code);
  return error_code;
}

int FileFS::RemoveDirectory(const char *path_cstr) override {
  ErrorCode error_code;

  Path path = Path::Normalize(path_cstr, error_code);
  std::shared_ptr<DirectoryEntry> dir = GetDirectory(path, error_code);
  if (!dir->Empty())
    return ErrorCode::kErrorNotEmpty;

  cwd->RemoveEntry(dir->base_offset(), error_code);
  ReleaseEntry<DirectoryEntry>(dir->base_offset());
  return error_code;
}

const char* FileFS::ListDirectory(const char *path_cstr, const char *prev,
                          char *next_buf, ErrorCode& error_code) override {
  ErrorCode error_code;

  Path path = Path::Normalize(path_cstr, error_code);
  DirectoryEntry *cwd = GetDirectory(path, error_code);

  cwd->GetNext(prev, next_buf, error_code);
  if (error_code != ErrorCode::kSuccess)
    return nullptr;

  return next_buf;
}

}  // namespace ffs

}  // namespace fs
