#include "lib/entries/none_entry.h"

#include "lib/layout/entry_layout.h"

namespace fs {

namespace linfs {

std::unique_ptr<NoneEntry> NoneEntry::Create(uint64_t entry_offset,
                                             ReaderWriter* writer,
                                             ErrorCode& error_code) {
  error_code = writer->Write<EntryLayout::NoneHeader>(EntryLayout::NoneHeader{0}, entry_offset);
  if (error_code != ErrorCode::kSuccess)
    return nullptr;

  return std::make_unique<NoneEntry>(entry_offset, 0);
}

Section NoneEntry::GetSection(uint64_t max_size, ReaderWriter* reader_writer, ErrorCode& error_code) {
  if (!HasSections())
    return Section{0,0,0}; // TODO throw?

  Section head = reader_writer->LoadSection(head_offset(), error_code);
  if (error_code != ErrorCode::kSuccess)
    return Section{0,0,0};

  if (head.size() > max_size) {
    Section result(head.base_offset() + head.size() - max_size, max_size, 0);
    error_code = reader_writer->SaveSection(result);
    if (error_code != ErrorCode::kSuccess)
      return Section{0,0,0};
    error_code = head.SetSize(head.size() - max_size, reader_writer);
    if (error_code != ErrorCode::kSuccess)
      return Section{0,0,0};
    return result;
  }
  else {
    error_code = SetHead(head.next_offset(), reader_writer);
    if (error_code != ErrorCode::kSuccess)
      return Section{0,0,0};
    return head;
  }
}

ErrorCode NoneEntry::PutSection(Section section, ReaderWriter* reader_writer) {
  // TODO lock-free?
  ErrorCode error_code = ErrorCode::kSuccess;
  Section last = section;
  while (last.next_offset() != 0) {
    last = reader_writer->LoadSection(last.next_offset(), error_code);
    if (error_code != ErrorCode::kSuccess)
      return error_code; // Leaked chain of sections
  }

  error_code = last.SetNext(head_offset(), reader_writer);
  if (error_code != ErrorCode::kSuccess)
    return error_code;  // Can't flush section with a new header; Drop broken section
  return SetHead(section.base_offset(), reader_writer);
}

ErrorCode NoneEntry::SetHead(uint64_t head_offset, ReaderWriter* reader_writer) {
  ErrorCode error_code = reader_writer->Write<uint64_t>(head_offset,
                                        base_offset() + offsetof(EntryLayout::NoneHeader, head_offset));
  if (error_code == ErrorCode::kSuccess)
    head_offset_ = head_offset;
  return error_code;
}

}  // namespace linfs

}  // namespace fs
