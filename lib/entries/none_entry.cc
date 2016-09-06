#include "lib/entries/directory_entry.h"

namespace fs {

namespace ffs {

NoneEntry::~NoneEntry() override {}

Section NoneEntry::GetSection(uint64_t max_size, ErrorCode& error_code) {
  Section section = device.LoadSection(section_offset(), error_code);
  if (section.size() > max_size) {
    Section rest(section.base_offset() + max_size, section.size() - max_size, section.next_offset());
    error_code = device.FlushSection(rest);
    if (error_code != ErrorCode::kSuccess)
      return Section();
    section.size(max_size);
    section.next_offset(0);
    section_offset(rest.base_offset());
    return section;
  }
  else {
    section.next_offset(0);
    section_offset(section.next_offset());
  }
  return section;
}

ErrorCode NoneEntry::PutSection(Section section) {
  // TODO lock-free?
  ErrorCode error_code = ErrorCode::kSuccess;
  Section last = section;
  while (last.next_offset() != 0) {
    last = device.LoadSection(last.next_offset(), error_code);
    if (error_code != ErrorCode::kSuccess) return error_code; // Leaked chain of sections
  }

  last.next_offset(section_offset());
  ErrorCode error_code = device.FlushSection(section);
  if (error_code != ErrorCode::kSuccess)
    return error_code;  // Can't flush section with a new header; Drop broken section
  section_offset(section.base_offset());
  return ErrorCode::kSuccess;
}

}  // namespace ffs

}  // namespace fs
