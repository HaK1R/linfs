#include "lib/sections/section_file.h"

#include <algorithm>

namespace fs {

namespace linfs {

size_t SectionFile::Read(uint64_t cursor, char *buf, size_t buf_size, ReaderWriter* reader) {
  uint64_t can_read = std::min(data_size() - cursor, buf_size);
  return reader->Read(data_offset() + cursor, buf, can_read);
}

size_t SectionFile::Write(uint64_t cursor, const char *buf, size_t buf_size, ReaderWriter* writer) {
  uint64_t can_write = std::min(data_size() - cursor, buf_size);
  return writer->Write(buf, can_write, data_offset() + cursor);
}

}  // namespace linfs

}  // namespace fs
