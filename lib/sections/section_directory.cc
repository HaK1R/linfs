#include "lib/sections/section_directory.h"

namespace fs {

namespace linfs {

ErrorCode SectionDirectory::AddEntry(uint64_t entry_offset, ReaderWriter* reader_writer, uint64_t start_position) {
  ErrorCode error_code;
  //std::tie(it, end) = reader_writer->ReadRange(data_offset() + start_position, data_size() - start_position, error_code);
  for (Iterator it = EntriesBegin(reader_writer, error_code, start_position); it != EntriesEnd(); ++it) {
    if (error_code != ErrorCode::kSuccess)
      return error_code;
    if (*it == 0) {
      error_code = reader_writer->Write(entry_offset, it.position());
      return error_code;
    }
  }

  return ErrorCode::kErrorNoMemory;
}

ErrorCode SectionDirectory::RemoveEntry(uint64_t entry_offset, ReaderWriter* reader_writer, uint64_t start_position) {
  ErrorCode error_code;
  for (Iterator it = EntriesBegin(reader_writer, error_code, start_position); it != EntriesEnd(); ++it) {
    if (error_code != ErrorCode::kSuccess)
      return error_code;
    if (*it == entry_offset) {
      error_code = reader_writer->Write(entry_offset, it.position());
      return error_code;
    }
  }

  return ErrorCode::kErrorNotFound;
}

bool SectionDirectory::HasEntries(ReaderWriter* reader_writer, ErrorCode& error_code, uint64_t start_position) {
  for (Iterator it = EntriesBegin(reader_writer, error_code, start_position); it != EntriesEnd(); ++it) {
    if (error_code != ErrorCode::kSuccess)
      return false;
    if (*it != 0)
      return true;
  }

  return false;
}

}  // namespace linfs

}  // namespace fs
