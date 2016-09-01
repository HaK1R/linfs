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
  ;
}

int FileFS::Format(const char *device_path, uint64_t cluster_size) {
  std::fstream device(device_path, std::ios::binary | std::ios::out | std::ios::trunc);
  if (!device.is_open())
    return -1;

  DeviceLayout::Header device_header;
  device_header.section_size_log2 = 12;
  device_header.root_section_offset = sizeof device_header;
  DeviceLayout::WriteHeader(device, device_header);

  SectionLayout::Header section_header;
  section_header.type = SectionFormat::kDirectory;
  section_header.type_traits.directory.available = (1 << device_header.cluster_size_log2) - sizeof device_header;
  section_header.type_traits.directory.next_offset = 0;
  section_header.type_traits.directory.name = "/";
  SectionLayout::WriteHeader(device, section_header);

  device.close();
  return 0;
}

bool FileFS::ListDirectory(const char *base_path, const char *prev_file,
                           char next_file[kNameMax]) override {
  SectionDirectory* dir = /* ... */;

  do {
    uint64_t entry_offset = dir->FindEntryByName(/* 1st name in base_path*/());
    base_path = strchr(base_path, '/');
    Section entry* = device->LoadSection(entry_offset);
    if (entry->type() == Section::Type::kFile)
      return false;
    dir = entry->As<SectionDirectory>();
  } while (base_path != nullptr);

  uint64_t prev_offset = dir->FindEntryByName(prev_file);
  uint64_t next_file-> dir->GetNextEntry(
}

}  // namespace ffs

}  // namespace fs
