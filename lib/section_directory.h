#pragma once

#include <cstddef>
#include <cstdint>

#include <iterator>

namespace fs {

namespace linfs {

class SectionDirectory : Section {
 public:
  typedef ReaderWriter::ReadIterator<uint64_t> Iterator;

  using Section::Section;
  SectionDirectory() = delete;
  ~SectionDirectory() = default;

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
