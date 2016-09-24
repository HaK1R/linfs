#include "lib/sections/section.h"

#include "lib/layout/section_layout.h"
#include "lib/reader_writer.h"

namespace fs {

namespace linfs {

void Section::SetSize(uint64_t size, ReaderWriter* reader_writer) {
  reader_writer->Write<uint64_t>(size, base_offset() + offsetof(SectionLayout::Header, size));
  size_ = size;
}

void Section::SetNext(uint64_t next_offset, ReaderWriter* reader_writer) {
  reader_writer->Write<uint64_t>(next_offset, base_offset() + offsetof(SectionLayout::Header, next_offset));
  next_offset_ = next_offset;
}

}  // namespace linfs

}  // namespace fs
