#include <cstddef>

#include "include/interfaces/IFileSystem.h"
#include "include/FileFS.h"
#include "lib/layout/device_header_format.h"

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

  static constexpr struct {
    DeviceHeader dev_hdr;
    SectionHeader root_section;
  } initial_info = {{kFileFSIdentifier, {kMajorVersion, kMinorVersion},
                     12, 0, &initial_info.root_section - &initial_info},
                    {KDirectory, {0}, 0, "/"}};
  device.write(&initial_info, sizeof initial_info);
  device.close();
  return 0;
}

}  // namespace ffs

}  // namespace fs
