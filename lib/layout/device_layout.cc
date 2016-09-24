#include "lib/layout/device_layout.h"

#include <cstring>

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

  // TODO fix endianness
  error_code = ErrorCode::kSuccess;
  return from_file;
}

void DeviceLayout::WriteHeader(Header header, ReaderWriter* writer) {
  // TODO fix endianness
  writer->Write<DeviceLayout::Header>(header, 0);
}

}  // namespace linfs

}  // namespace fs
