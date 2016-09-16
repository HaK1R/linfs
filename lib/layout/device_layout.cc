#include "lib/layout/device_layout.h"

#include <cstring>

namespace fs {

namespace linfs {

ErrorCode DeviceLayout::ParseHeader(ReaderWriter* reader, Header& header) {
  ErrorCode error_code;
  Header from_file = reader->Read<DeviceLayout::Header>(0, error_code);
  if (error_code != ErrorCode::kSuccess)
    return error_code;

  Header default_header;
  if (memcmp(from_file.identifier, default_header.identifier, sizeof from_file.identifier) != 0)
    return ErrorCode::kErrorInvalidSignature;

  if (from_file.version.major > default_header.version.major ||
      (from_file.version.major == default_header.version.major &&
       from_file.version.minor > default_header.version.minor))
    return ErrorCode::kErrorNotSupported;

  // TODO fix endianness
  header = from_file;
  return ErrorCode::kSuccess;
}

ErrorCode DeviceLayout::WriteHeader(ReaderWriter* writer, Header header) {
  // TODO fix endianness
  return writer->Write<DeviceLayout::Header>(header, 0);
}

}  // namespace linfs

}  // namespace fs
