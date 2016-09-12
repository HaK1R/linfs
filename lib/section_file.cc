#include "lib/entries/directory_entry.h"

namespace fs {

namespace ffs {

size_t SectionFile::Read(uint64_t cursor, char *buf, size_t buf_size, ReaderWriter* reader_writer, ErrorCode& error_code) {
  uint64_t can_read = std::min(data_size() - cursor, buf_size);
  return reader_writer->Read(section.data_offset() + cursor, buf, can_read, error_code);
}

size_t SectionFile::Write(uint64_t cursor, const char *buf, size_t buf_size, ReaderWriter* reader_writer, ErrorCode& error_code) {
  uint64_t can_write = std::min(data_size() - cursor, buf_size);
  return reader_writer->Write(section.data_offset() + cursor, buf, can_write, error_code);
}

}  // namespace ffs

}  // namespace fs
