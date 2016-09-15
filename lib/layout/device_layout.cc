#include "lib/layout/device_layout.h"

#include <cstring>

namespace fs {

namespace ffs {

ErrorCode DeviceLayout::ParseHeader(ReaderWriter* reader, Header& header) {
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

  // TODO fix endianness
  header = from_file;
  return ErrorCode::kSuccess;
}

}  // namespace ffs

}  // namespace fs
