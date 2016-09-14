#pragma once

#include <cstddef>
#include <cstdint>

#include <iterator>

namespace fs {

namespace linfs {

class SectionFile : Section {
 public:
  using Section::Section;
  SectionFile() = delete;
  ~SectionFile() = default;

  size_t Read(uint64_t cursor, char *buf, size_t buf_size, ReaderWriter* reader_writer, ErrorCode& error_code);
  size_t Write(uint64_t cursor, const char *buf, size_t buf_size, ReaderWriter* reader_writer, ErrorCode& error_code);
};

}  // namespace linfs

}  // namespace fs
