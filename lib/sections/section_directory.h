#pragma once

#include <cstdint>

#include "fs/error_code.h"
#include "lib/reader_writer.h"
#include "lib/sections/section.h"

namespace fs {

namespace linfs {

class SectionDirectory : public Section {
 public:
  typedef ReaderWriter::ReadIterator<uint64_t> Iterator;

  SectionDirectory() = delete;
  using Section::Section;

  Iterator EntriesBegin(ReaderWriter* reader_writer, uint64_t start_position = 0) {
    return Iterator(data_offset() + start_position, reader_writer);
  }
  Iterator EntriesEnd() {
    return Iterator(data_offset() + data_size());
  }

  bool AddEntry(uint64_t entry_offset, ReaderWriter* reader_writer, uint64_t start_position = 0);
  bool RemoveEntry(uint64_t entry_offset, ReaderWriter* reader_writer, uint64_t start_position = 0);
  bool HasEntries(ReaderWriter* reader_writer, uint64_t start_position = 0);
};

}  // namespace linfs

}  // namespace fs
