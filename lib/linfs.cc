#include <cstddef>

#include "include/interfaces/IFileSystem.h"
#include "include/LinFS.h"
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
std::shared_ptr<T> LinFS::AllocateEntry<T>(ErrorCode& error_code, Args&&... args) {
  Section place = allocator_.AllocateSection(error_code);
  if (error_code != ErrorCode::kSuccess) return error_code;

  std::shared_ptr<T> entry = T::Create(place, error_code, std::forward<Args>(args)...);
  if (!entry)
    allocate_.ReleaseSection(place);
  return error_code;
}

void LinFS::ReleaseEntry(shared_ptr<Entry> entry) {
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

  // TODO? DeviceLayout::Header header;
  uint64_t cluster_size, total_clusters;
  uint16_t none_entry_offset, root_section_offset;
  error_code = DeviceLayout::ParseHeader(&accessor_, cluster_size_,
                                         none_entry_offset,
                                         root_entry_offset,
                                         total_clusters,
                                         error_code);
  if (error_code != ErrorCode::kSuccess)
    return error_code;

  std::shared_ptr<NoneEntry> none_entry = accessor_.LoadEntry(none_entry_offset_, error_code);
  if (error_code != ErrorCode::kSuccess)
    return error_code;

  allocator_ = SectionAllocator(cluster_size, total_clusters, std::move(none_entry));

  root_entry_ = accessor_.LoadEntry(root_entry_offset, error_code);
  if (error_code != ErrorCode::kSuccess)
    return error_code;
  // TODO? accessor_.StreamReader(error_code, none_entry_offset) >> none_entry_;
  // TODO? root_entry_.StreamReader(error_code, root_entry_offset) >> root_entry_;
}

int LinFS::Format(const char *device_path, uint64_t cluster_size) {
  // Used only to calculate offsets
  struct __attribute__((packed)) EmptyLayout {
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

  error_code = DeviceLayout::Write(writer, ClusterSize::k4Kb,
                                   offsetof(EmptyLayout, none_entry_header),
                                   offsetof(EmptyLayout, root));
  std::shared_ptr<NoneEntry> none_entry = NoneEntry::Create(offsetof(EmptyLayout, none_entry_header), error_code, 0);

  Section root_section(offsetof(EmptyLayout, root), (1 << ClusterSize::k4Kb) - offsetof(EmptyLayout, root), 0);
  error_code = writer->SaveSection(root_section);

  std::shared_ptr<DirectoryEntry> root_entry = DirectoryEntry::Create(root_section, error_code, 0);

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

  IFile* file = new (std::nothrow) FileImpl(file_entry, &accessor_, &allocator_);
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
