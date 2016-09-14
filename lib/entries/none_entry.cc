#include "lib/entries/none_entry.h"

namespace fs {

namespace linfs {

std::shared_ptr<NoneEntry> NoneEntry::Create(uint64_t base_offset,
                                             ReaderWriter* writer,
                                             ErrorCode& error_code) {
  error_code = device.Write(EntryLayout::NoneHeader(Entry::Type::kNone, 0), base_offset);
  if (error_code != ErrorCode::kSuccess)
    return std::shared_ptr<NoneEntry>();

  return std::make_shared<NoneEntry>(base_offset, 0);
}

Section NoneEntry::GetSection(uint64_t max_size, ReaderWriter* reader_writer, ErrorCode& error_code) {
  if (!HasSections())
    return Section(); // TODO throw?

  Section head = reader_writer->LoadSection(head_offset(), error_code);
  if (error_code != ErrorCode::kSuccess)
    return Section();

  if (head.size() > max_size) {
    Section result(head.base_offset() + head.size() - max_size, max_size, 0);
    error_code = reader_writer->SaveSection(result);
    if (error_code != ErrorCode::kSuccess)
      return Section();
    error_code = head.SetSize(head.size() - max_size, reader_writer);
    if (error_code != ErrorCode::kSuccess)
      return Section();
    return result;
  }
  else {
    error_code = SetHead(head.next_offset(), reader_writer);
    if (error_code != ErrorCode::kSuccess)
      return Section();
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

ErrorCode SetHead(uint64_t head_offset, ReaderWriter* reader_writer) {
  ErrorCode error_code = reader_writer->Write<uint64_t>(head_offset,
                                        base_offset() + offsetof(EntryLayout::NoneHeader, head_offset));
  if (error_code == ErrorCode::kSuccess)
    head_offset_ = head_offset;
  return error_code;
}

}  // namespace linfs

}  // namespace fs
