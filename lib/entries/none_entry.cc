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

  // We don't want to use the head because of exception safety, but using its
  // tail we can provide strong exception guarantee.
  Section tail = Section::Load(head_offset(), reader_writer), prev = tail;
  while (tail.next_offset() != 0) {
    prev = tail;
    tail = Section::Load(tail.next_offset(), reader_writer);
  }

  if (tail.size() > max_size) {
    Section result = Section::Create(tail.base_offset() + tail.size() - max_size,
                                     max_size, reader_writer);
    tail.SetSize(tail.size() - max_size, reader_writer);
    return result;
  }
  else {
    if (head_offset() == tail.base_offset())
      SetHead(0, reader_writer);
    else
      prev.SetNext(0, reader_writer);
    return tail;
  }
}

void NoneEntry::PutSection(Section section, ReaderWriter* reader_writer) {
  Section last = section;
  while (last.next_offset() != 0)
    last = Section::Load(last.next_offset(), reader_writer);
  last.SetNext(head_offset(), reader_writer);
  SetHead(section.base_offset(), reader_writer);
}

void NoneEntry::SetHead(uint64_t head_offset, ReaderWriter* writer) {
  writer->Write<uint64_t>(head_offset,
                          base_offset() + offsetof(EntryLayout::NoneHeader, head_offset));
  head_offset_ = head_offset;
}

}  // namespace linfs

}  // namespace fs
