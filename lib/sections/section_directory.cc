#include "lib/sections/section_directory.h"

namespace fs {

namespace linfs {

bool SectionDirectory::AddEntry(uint64_t entry_offset, ReaderWriter* reader_writer,
                                uint64_t start_position) {
  for (Iterator it = EntriesBegin(reader_writer, start_position); it != EntriesEnd(); ++it)
    if (*it == 0) {
      reader_writer->Write<uint64_t>(entry_offset, it.position());
      return true;
    }

  return false;
}

bool SectionDirectory::RemoveEntry(uint64_t entry_offset, ReaderWriter* reader_writer,
                                   uint64_t start_position) {
  for (Iterator it = EntriesBegin(reader_writer, start_position); it != EntriesEnd(); ++it)
    if (*it == entry_offset) {
      reader_writer->Write<uint64_t>(0, it.position());
      return true;
    }

  return false;
}

bool SectionDirectory::HasEntries(ReaderWriter* reader, uint64_t start_position) {
  for (Iterator it = EntriesBegin(reader, start_position); it != EntriesEnd(); ++it)
    if (*it != 0)
      return true;

  return false;
}

}  // namespace linfs

}  // namespace fs
