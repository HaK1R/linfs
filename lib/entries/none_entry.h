#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>

#include "fs/error_code.h"
#include "lib/entries/entry.h"
#include "lib/reader_writer.h"
#include "lib/sections/section.h"

namespace fs {

namespace linfs {

class NoneEntry : public Entry {
 public:
  static std::unique_ptr<NoneEntry> Create(uint64_t entry_offset,
                                           uint64_t entry_size,
                                           ReaderWriter* writer,
                                           ErrorCode& error_code);

  NoneEntry(uint64_t base_offset, uint64_t head_offset)
      : Entry(Type::kNone, base_offset), head_offset_(head_offset) {}
  ~NoneEntry() override = default;

  uint64_t head_offset() const { return head_offset_; }
  // TODO section_offset() { throw std::logic_exception(); }

  Section GetSection(uint64_t max_size, ReaderWriter* reader_writer, ErrorCode& error_code);
  ErrorCode PutSection(Section section, ReaderWriter* reader_writer);
  bool HasSections() const { return head_offset() != 0; }

 private:
  ErrorCode SetHead(uint64_t head_offset, ReaderWriter* reader_writer);

  uint64_t head_offset_;
};

}  // namespace linfs

}  // namespace fs
