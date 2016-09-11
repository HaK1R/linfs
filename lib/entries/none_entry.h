#pragma once

#include <cstdint>

#include "lib/entries/entry.h"

namespace fs {

namespace ffs {

class NoneEntry : public Entry {
 public:
  NoneEntry(uint64_t base_offset, uint64_t head_offset)
      : Entry(Type::kNone, base_offset), head_offset_(head_offset) {}
  ~NoneEntry() override {}

  uint64_t head_offset() const { return head_offset_; }
  // TODO section_offset() { throw std::logic_exception(); }

  Section GetSection(uint64_t max_size, ReaderWriter* reader_writer, ErrorCode& error_code);
  ErrorCode PutSection(Section section, ReaderWriter* reader_writer);
  bool HasSections() const { return head_offset_ != 0; }

 protected:
  ErrorCode SetHead(uint64_t head_offset, ReaderWriter* reader_writer) {
    ErrorCode error_code = reader_writer->Write<uint64_t>(head_offset,
                                          base_offset() + offsetof(EntryLayout::NoneHeader, head_offset));
    if (error_code == ErrorCode::kSuccess)
      head_offset_ = head_offset;
    return error_code;
  }

  uint64_t head_offset_;
};

}  // namespace ffs

}  // namespace fs
