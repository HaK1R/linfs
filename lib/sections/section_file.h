#pragma once

#include <cstddef>
#include <cstdint>

#include "fs/error_code.h"
#include "lib/reader_writer.h"
#include "lib/sections/section.h"

namespace fs {

namespace linfs {

class SectionFile : public Section {
 public:
  SectionFile() = delete;
  using Section::Section;

  size_t Read(uint64_t cursor, char *buf, size_t buf_size, ReaderWriter* reader_writer, ErrorCode& error_code);
  size_t Write(uint64_t cursor, const char *buf, size_t buf_size, ReaderWriter* reader_writer, ErrorCode& error_code);
};

}  // namespace linfs

}  // namespace fs
