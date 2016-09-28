#pragma once

#include <cstdint>

#include "lib/sections/section.h"
#include "lib/utils/reader_writer.h"

namespace fs {

namespace linfs {

class SectionDirectory : public Section {
 public:
  typedef ReaderWriter::ReadIterator<uint64_t> Iterator;

  SectionDirectory(const Section& base) : Section(base) {}

  Iterator EntriesBegin(ReaderWriter* reader, uint64_t start_position = 0) {
    return Iterator(data_offset() + start_position, reader);
  }
  Iterator EntriesEnd() {
    return Iterator(data_offset() + data_size());
  }

  bool AddEntry(uint64_t entry_offset, ReaderWriter* reader_writer, uint64_t start_position = 0);
  bool RemoveEntry(uint64_t entry_offset, ReaderWriter* reader_writer, uint64_t start_position = 0);
  bool HasEntries(ReaderWriter* reader, uint64_t start_position = 0);
};

}  // namespace linfs

}  // namespace fs
