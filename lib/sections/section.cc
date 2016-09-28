#include "lib/sections/section.h"

#include "lib/layout/section_layout.h"
#include "lib/utils/byte_order.h"

namespace fs {

namespace linfs {

Section Section::Load(uint64_t section_offset, ReaderWriter* reader) {
  SectionLayout::Header header = reader->Read<SectionLayout::Header>(section_offset);
  return Section(section_offset, ByteOrder::Unpack(header.size),
                 ByteOrder::Unpack(header.next_offset));
}

Section Section::Create(uint64_t section_offset, uint64_t section_size,
                        ReaderWriter* writer) {
  SectionLayout::Header header = {ByteOrder::Pack(section_size), 0};
  writer->Write<SectionLayout::Header>(header, section_offset);
  return Section(section_offset, section_size, 0);
}

void Section::SetSize(uint64_t size, ReaderWriter* writer) {
  writer->Write<uint64_t>(size, base_offset() + offsetof(SectionLayout::Header, size));
  size_ = size;
}

void Section::SetNext(uint64_t next_offset, ReaderWriter* writer) {
  writer->Write<uint64_t>(next_offset,
                          base_offset() + offsetof(SectionLayout::Header, next_offset));
  next_offset_ = next_offset;
}

}  // namespace linfs

}  // namespace fs
