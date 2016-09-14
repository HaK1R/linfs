#pragma once

#include <cstdint>

#include "fs/error_code.h"
#include "lib/reader_writer.h"
#include "lib/section.h"

namespace fs {

namespace linfs {

class SectionDirectory : Section {
 public:
  typedef ReaderWriter::ReadIterator<uint64_t> Iterator;

  SectionDirectory() = delete;
  using Section::Section;

  Iterator EntriesBegin(ReaderWriter* reader_writer, ErrorCode& error_code, uint64_t start_position = 0) {
    return Iterator(data_offset() + start_position, reader_writer, error_code);
  }

  Iterator EntriesEnd() {
    return Iterator(data_offset() + data_size());
  }

  ErrorCode AddEntry(uint64_t entry_offset, ReaderWriter* reader_writer, uint64_t start_position = 0);
  ErrorCode RemoveEntry(uint64_t entry_offset, ReaderWriter* reader_writer, uint64_t start_position = 0);
  bool HasEntries(ReaderWriter* reader_writer, ErrorCode& error_code);
};

}  // namespace linfs

}  // namespace fs
