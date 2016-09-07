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

Section FileFS::AllocateSection(ErrorCode& error_code) {
  if (none_entry_->HasSections()) {
    return none_entry_->GetSection(cluster_size_, error_code);
  }

  // There is nothing in NoneEntry chain. Allocate a new cluster.
  uint64_t cluster_offset = total_clusters_ * cluster_size_;
  device_.seekp(cluster_offset + cluster_size_ - 1);
  device_.write("", 1);
  ++total_clusters_;
  return Section(cluster_offset, cluster_size_, 0);
}

void FileFS::ReleaseSection(const Section& section, ErrorCode& error_code) {
  uint64_t last_cluster_offset = (total_clusters_ - 1) * cluster_size_;
  if (last_cluster_offset == section.base_offset()) {
    error_code = ErrorCode::kSuccess;
    --total_clusters_;
  } else {
    error_code = none_entry->PutSection(section);
  }
}

void FileFS::ReleaseSection(const Section& section) {
  ErrorCode error_code;
  ReleaseSection(section, error_code);
  if (error_code != ErrorCode::kSuccess)
    std::cerr << "Leaked section at " << std::hex << section.base_offset() << " of size " << std::dec << section.size();
}

void FileFS::UnlinkAndReleaseSection(const Section& section, uint64_t prev_offset) {
  ErrorCode error_code = device.Write<uint64_t>(section->next_offset(), prev_offset + offsetof(SectionLayout::Header, next_offset));
  if (error_code != ErrorCode::kSuccess)
    std::cerr << "Broken section's memory at " << std::hex << prev_offset;
  else
    ReleaseSection(section);
}

template<typename T>
std::shared_ptr<T> FileFS::AllocateEntry<T>(ErrorCode& error_code, Args&&... args) {
  Section place = AllocateSection(error_code);
  if (error_code != ErrorCode::kSuccess) return error_code;

  std::shared_ptr<T> entry = T::Create(place, error_code, std::forward<Args>(args)...);
  if (!entry)
    ReleaseSection(place);
  return error_code;
}

void FileFS::ReleaseEntry(shared_ptr<Entry> entry) {
  ReleaseSection(Section(entry->base_offset(), cluster_size_, 0));
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

  device << DeviceLayout::Header(ClusterSize::k4Kb, sizeof(DeviceLayout::Header) + sizeof(EntryLayout::Header), 
                                 sizeof(DeviceLayout::Header))
         << EntryLayout::Header(Entry::Type::kNone)
         << EntryLayout::Header(Entry::Type::kDirectory, "/")
         << SectionLayout::Header(cluster_size - sizeof(DeviceLayout::Header) - sizeof(EntryLayout::Header), 0,
                                  cluster_size - sizeof(DeviceLayout::Header) - sizeof(EntryLayout::Header) - sizeof(SectionLayout::Header));

  device.close();
  return 0;
}

int FileFS::CreateDirectory(const char *path_cstr) override {
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

  error_code = cwd->AddEntry(new_dir);
  if (error_code != ErrorCode::kSuccess) {
    ReleaseEntry(new_dir);
    return error_code;
  }

  return ErrorCode::kSuccess;
}

int FileFS::RemoveDirectory(const char *path_cstr) override {
  ErrorCode error_code;

  Path path = Path::Normalize(path_cstr, error_code);
  if (error_code != ErrorCode::kSuccess)
    return error_code;

  std::shared_ptr<DirectoryEntry> dir = GetDirectory(path, error_code);
  if (error_code != ErrorCode::kSuccess)
    return error_code;
  if (!dir->Empty())
    return ErrorCode::kErrorNotEmpty;

  error_code = cwd->RemoveEntry(dir);
  if (error_code != ErrorCode::kSuccess)
    return error_code;

  ReleaseEntry(dir->base_offset());

  return ErrorCode::kSuccess;
}

const char* FileFS::ListDirectory(const char *path_cstr, const char *prev,
                          char *next_buf, ErrorCode& error_code) override {
  Path path = Path::Normalize(path_cstr, error_code);
  if (error_code != ErrorCode::kSuccess)
    return error_code;

  DirectoryEntry *cwd = GetDirectory(path, error_code);
  if (error_code != ErrorCode::kSuccess)
    return error_code;

  error_code = cwd->GetNextEntryName(prev, next_buf, error_code);
  if (error_code != ErrorCode::kSuccess)
    return nullptr;

  return next_buf;
}

}  // namespace ffs

}  // namespace fs
