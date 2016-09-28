#pragma once

#include <cstddef>
#include <cstdint>

#include "lib/sections/section.h"
#include "lib/utils/reader_writer.h"

namespace fs {

namespace linfs {

class SectionFile : public Section {
 public:
  SectionFile(const Section& base) : Section(base) {}

  size_t Read(uint64_t cursor, char* buf, size_t buf_size, ReaderWriter* reader);
  size_t Write(uint64_t cursor, const char* buf, size_t buf_size, ReaderWriter* writer);
};

}  // namespace linfs

}  // namespace fs
