#include "lib/layout/device_layout.h"

#include <cstring>

namespace fs {

namespace ffs {

bool DeviceLayout::ParseHeader(std::ifstream& file, uint64_t& section_size,
                               uint16_t& root_section_offset) {
  Header from_file;
  if (sizeof from_file != file.read(&from_file, sizeof from_file))
    return false;

  Header default_header;
  if (memcmp(from_file.identifier, default_header.identifier))
    return false;

  if (from_file.version.major > default_header.version.major ||
      from_file.version.major == default_header.version.major &&
      from_file.version.minor > default_header.version.minor)
    return false;

  section_size = 1 << from_file.section_size_log2;
  root_section_offset = from_file.root_section_offset;
  return true;
}

bool DeviceLayout::WriteHeader(std::ofstream& file, const Header& header) {
  return sizeof header == file.write(&header, sizeof header);
}

}  // namespace ffs

}  // namespace fs
