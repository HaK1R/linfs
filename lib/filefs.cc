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

FileFS::Load(const char *device_path) {
  device_.open(device_path, std::ios::binary | std::ios::in | std::ios::out);
  if (!device.is_open())
    return -1;

  device_ >> DeviceLayout::Header;
  DirectoryEntry* root = LoadEntry<DirectoryEntry>(device_);
}

int FileFS::Format(const char *device_path, uint64_t cluster_size) {
  std::fstream device(device_path, std::ios::binary | std::ios::out | std::ios::trunc);
  if (!device.is_open())
    return -1;

  device << DeviceLayout::Header(ClusterSize::k4Kb, sizeof(DeviceLayout::Header), 0)
         << EntryLayout::Header(Entry::Type::kDirectory)
         << SectionLayout::Header(cluster_size - sizeof(DeviceLayout::Header) - sizeof(EntryLayout::Header), 0,
                                  cluster_size - sizeof(DeviceLayout::Header) - sizeof(EntryLayout::Header) - sizeof(SectionLayout::Header));

  device.close();
  return 0;
}

int FileFS::CreateDirectory(const char *path_cstr) override {
  ErrorCode error_code;

  Path path = Path::Normalize(path_cstr, error_code);
  DirectoryEntry *cwd = GetDirectory(path.DirectoryName(), error_code);
  uint64_t entry_offset = cwd->GetEntryByName(path.BaseName(), error_code);
  if (entry_offset != 0)
    return ErrorCode::kErrorExist;

  DirectoryEntry *new_dir = AllocateEntry<DirectoryEntry>(error_code, path.BaseName());
  cwd->AddEntry(new_dir->base_offset(), error_code);
  return error_code;
}

int FileFS::RemoveDirectory(const char *path_cstr) override {
  ErrorCode error_code;

  Path path = Path::Normalize(path_cstr, error_code);
  DirectoryEntry *cwd = GetDirectory(path.DirectoryName(), error_code);
  uint64_t entry_offset = cwd->GetEntryByName(path.BaseName(), error_code);
  if (entry_offset == 0)
    return ErrorCode::kNotFound;

  cwd->RemoveEntry(entry_offset, error_code);
  ReleaseEntry<DirectoryEntry>(entry_offset);
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
