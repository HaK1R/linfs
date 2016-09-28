#pragma once

#include <cstdint>

#include "lib/layout/section_layout.h"
#include "lib/utils/reader_writer.h"

namespace fs {

namespace linfs {

class Section {
 public:
  static Section Load(uint64_t section_offset, ReaderWriter* reader);
  static Section Create(uint64_t section_offset, uint64_t section_size,
                        ReaderWriter* writer);

  Section(uint64_t base_offset, uint64_t size, uint64_t next_offset)
      : base_offset_(base_offset), size_(size), next_offset_(next_offset) {}

  uint64_t base_offset() const { return base_offset_; }
  uint64_t size() const { return size_; }
  uint64_t next_offset() const { return next_offset_; }

  uint64_t data_offset() const { return base_offset() + sizeof(SectionLayout::Header); }
  uint64_t data_size() const { return size() - sizeof(SectionLayout::Header); }

  void SetSize(uint64_t size, ReaderWriter* writer);
  void SetNext(uint64_t next_offset, ReaderWriter* writer);

 private:
  uint64_t base_offset_;
  uint64_t size_;
  uint64_t next_offset_;
};

}  // namespace linfs

}  // namespace fs
