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

}  // namespace ffs

}  // namespace fs
