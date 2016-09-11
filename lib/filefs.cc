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
    return none_entry_->GetSection(cluster_size_, accessor_, error_code);
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
    error_code = none_entry->PutSection(section, accessor_);
  }
}

void FileFS::ReleaseSection(const Section& section) {
  ErrorCode error_code;
  ReleaseSection(section, error_code);
  if (error_code != ErrorCode::kSuccess)
    std::cerr << "Leaked section at " << std::hex << section.base_offset() << " of size " << std::dec << section.size();
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
  ReleaseSection(Section(entry->section_offset(), cluster_size_, 0));
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
  ErrorCode error_code = accessor_.Open(device_path, std::ios_base::in | std::ios_base::out);
  if (error_code != ErrorCode::kSuccess)
    return error_code;

  uint16_t none_entry_offset, root_entry_offset;
  error_code = DeviceLayout::ParseHeader(accessor_, cluster_size_,
                                         none_entry_offset,
                                         root_entry_offset);
  if (error_code != ErrorCode::kSuccess)
    return error_code;

  none_entry_ = accessor_.LoadEntry(none_entry_offset_, error_code);
  if (error_code != ErrorCode::kSuccess)
    return error_code;

  root_entry_ = accessor_.LoadEntry(root_entry_offset, error_code);
  if (error_code != ErrorCode::kSuccess)
    return error_code;
  //accessor_.StreamReader(error_code, none_entry_offset) >> none_entry_;
  //root_entry_.StreamReader(error_code, root_entry_offset) >> root_entry_;
}

int FileFS::Format(const char *device_path, uint64_t cluster_size) {
  struct __attribute__((packed)) EmptyDiskData {
    DeviceLayout::Header device_header;
    EntryLayout::NoneHeader none_entry_header;
    struct __attribute__((packed)) {
      SectionLayout::Header section_header;
      EntryLayout::DirectoryHeader entry_header;
    } root;
  };
  static_assert(std::is_standard_layout<EmptyDiskData>::value,
                "EmptyDiskData must be a standard-layout class");
  static EmptyDiskData data = {
      DeviceLayout::Header(ClusterSize::k4Kb,
                           offsetof(EmptyDiskData, none_entry_header),
                           offsetof(EmptyDiskData, root)),
      EntryLayout::NoneHeader(0),
      {SectionLayout::Header((1 << ClusterSize::k4Kb) - offsetof(EmptyDiskData, root), 0),
       EntryLayout::DirectoryHeader("/")}
  };

  ErrorCode error_code;
  ReaderWriter writer;
  error_code = writer.Open(device_path, std::ios_base::out | std::ios_base::trunc);
  if (error_code != ErrorCode::kSuccess)
    return error_code;

  writer << data.device_header
         << data.none_entry_header
         << data.root.section_header
         << data.root.entry_header;

  return ErrorCode::kSuccess;
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

  error_code = cwd->AddEntry(new_dir, accessor_);
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

  error_code = cwd->RemoveEntry(dir, accessor_);
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

  error_code = cwd->GetNextEntryName(prev, accessor_, next_buf);
  if (error_code != ErrorCode::kSuccess)
    return nullptr;

  return next_buf;
}

}  // namespace ffs

}  // namespace fs
