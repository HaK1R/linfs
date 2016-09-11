#include "lib/entries/directory_entry.h"

namespace fs {

namespace ffs {

ErrorCode SectionDirectory::AddEntry(uint64_t entry_offset, ReaderWriter* reader_writer, uint64_t start_position) {
  ErrorCode error_code;
  uint64_t it_offset = sizeof(SectionLayout::Header) + start_position;
  for (Iterator it(base_offset() + it_offset, base_offset() + size(), error_code);
       error_code == ErrorCode::kSuccess && it != Iterator();
       ++it, it_offset += sizeof(uint64_t)) {
    if (*it == 0)
      return reader_writer->Write<uint64_t>(entry_offset, base_offset() + it_offset);
  }

  if (error_code != ErrorCode::kSuccess)
    return error_code;

  return ErrorCode::kErrorNoMemory;
}

bool SectionDirectory::RemoveEntry(uint64_t entry_offset, ReaderWriter* reader_writer, ErrorCode& error_code, uint64_t start_position) {
  bool removed = false;
  bool has_entries = false;
  uint64_t it_offset = sizeof(SectionLayout::Header) + start_position;
  for (Iterator it(base_offset() + it_offset, base_offset() + size(), error_code);
       error_code == ErrorCode::kSuccess && it != Iterator();
       ++it, it_offset += sizeof(uint64_t)) {
    if (*it == entry_offset) {
      error_code = reader_writer->Write<uint64_t>(0, base_offset() + it_offset);
      return true; // TODO return has_entries
    }
  }

  if (error_code != ErrorCode::kSuccess)
    return true;

  error_code = ErrorCode::kErrorNotFound;
  return true;
}

}  // namespace ffs

}  // namespace fs
