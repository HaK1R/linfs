#include "lib/sections/section_file.h"

#include <algorithm>

namespace fs {

namespace linfs {

size_t SectionFile::Read(uint64_t cursor, char *buf, size_t buf_size, ReaderWriter* reader_writer, ErrorCode& error_code) {
  uint64_t can_read = std::min(data_size() - cursor, buf_size);
  return reader_writer->Read(data_offset() + cursor, buf, can_read, error_code);
}

size_t SectionFile::Write(uint64_t cursor, const char *buf, size_t buf_size, ReaderWriter* reader_writer, ErrorCode& error_code) {
  uint64_t can_write = std::min(data_size() - cursor, buf_size);
  return reader_writer->Write(buf, can_write, data_offset() + cursor, error_code);
}

}  // namespace linfs

}  // namespace fs
