#include "lib/layout/device_layout.h"

#include <cstring>

#include "lib/utils/byte_order.h"

namespace fs {

namespace linfs {

DeviceLayout::Header DeviceLayout::ParseHeader(ReaderWriter* reader, ErrorCode& error_code) {
  Header from_file = reader->Read<DeviceLayout::Header>(0);

  Header default_header;
  if (memcmp(from_file.identifier, default_header.identifier, sizeof from_file.identifier) != 0) {
    error_code = ErrorCode::kErrorInvalidSignature;
    return from_file;
  }

  if (from_file.version.major > default_header.version.major ||
      (from_file.version.major == default_header.version.major &&
       from_file.version.minor > default_header.version.minor)) {
    error_code = ErrorCode::kErrorNotSupported;
    return from_file;
  }

  // Convert values to host's byte order.
  from_file.none_entry_offset = ByteOrder::Unpack(from_file.none_entry_offset);
  from_file.root_entry_offset = ByteOrder::Unpack(from_file.root_entry_offset);
  from_file.total_clusters = ByteOrder::Unpack(from_file.total_clusters);

  error_code = ErrorCode::kSuccess;
  return from_file;
}

void DeviceLayout::WriteHeader(Header header, ReaderWriter* writer) {
  // Convert values to device's byte order.
  header.none_entry_offset = ByteOrder::Pack(header.none_entry_offset);
  header.root_entry_offset = ByteOrder::Pack(header.root_entry_offset);
  header.total_clusters = ByteOrder::Pack(header.total_clusters);

  writer->Write<DeviceLayout::Header>(header, 0);
}

}  // namespace linfs

}  // namespace fs
