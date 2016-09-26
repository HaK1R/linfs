#include "lib/entries/none_entry.h"

#include <cassert>

#include "lib/layout/entry_layout.h"

namespace fs {

namespace linfs {

std::unique_ptr<NoneEntry> NoneEntry::Create(uint64_t entry_offset,
                                             uint64_t /* entry_size */,
                                             ReaderWriter* writer) {
  writer->Write<EntryLayout::NoneHeader>(EntryLayout::NoneHeader(0), entry_offset);
  return std::make_unique<NoneEntry>(entry_offset, 0);
}

Section NoneEntry::GetSection(uint64_t max_size, ReaderWriter* reader_writer) {
  assert(HasSections() && "there are no sections in NoneEntry");

  Section head = reader_writer->LoadSection(head_offset());
  if (head.size() > max_size) {
    Section result(head.base_offset() + head.size() - max_size, max_size, 0);
    reader_writer->SaveSection(result);
    head.SetSize(head.size() - max_size, reader_writer);
    return result;
  }
  else {
    SetHead(head.next_offset(), reader_writer);
    return head;
  }
}

void NoneEntry::PutSection(Section section, ReaderWriter* reader_writer) {
  Section last = section;
  while (last.next_offset() != 0)
    last = reader_writer->LoadSection(last.next_offset());
  last.SetNext(head_offset(), reader_writer);
  SetHead(section.base_offset(), reader_writer);
}

void NoneEntry::SetHead(uint64_t head_offset, ReaderWriter* writer) {
  writer->Write<uint64_t>(head_offset, base_offset() + offsetof(EntryLayout::NoneHeader, head_offset));
  head_offset_ = head_offset;
}

}  // namespace linfs

}  // namespace fs
