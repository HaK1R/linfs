#include "lib/layout/device_layout.h"

#include <cstring>

namespace fs {

namespace ffs {

ErrorCode DeviceLayout::ParseHeader(ReaderWriter* reader, uint64_t& cluster_size, uint16_t& none_entry_offset, uint16_t& root_section_offset, uint64_t& total_clusters) {
  Header from_file = reader->Read<DeviceLayout::Header>(0, error_code);
  if (error_code != ErrorCode::kSuccess)
    return error_code;

  Header default_header;
  if (memcmp(from_file.identifier, default_header.identifier))
    return ErrorCode::kErrorInvalidSignature;

  if (from_file.version.major > default_header.version.major ||
      from_file.version.major == default_header.version.major &&
      from_file.version.minor > default_header.version.minor)
    return ErrorCode::kErrorVersionNotSupported;

  cluster_size = 1 << from_file.section_size_log2;
  // TODO fix endianness
  none_entry_offset = from_file.none_entry_offset;
  root_section_offset = from_file.root_section_offset;
  total_clusters = from_file.total_clusters;
  return ErrorCode::kSuccess;
}

}  // namespace ffs

}  // namespace fs
