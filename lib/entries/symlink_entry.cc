#include "lib/entries/symlink_entry.h"

#include "lib/layout/entry_layout.h"
#include "lib/sections/section_symlink.h"
#include "lib/utils/format_exception.h"

namespace fs {

namespace linfs {

std::unique_ptr<SymlinkEntry> SymlinkEntry::Create(uint64_t entry_offset,
                                                   uint64_t /* entry_size */,
                                                   ReaderWriter* reader_writer,
                                                   const char* name,
                                                   const char* target,
                                                   SectionAllocator* allocator) {
  reader_writer->Write<EntryLayout::SymlinkHeader>(EntryLayout::SymlinkHeader(name),
                                                   entry_offset);
  std::unique_ptr<SymlinkEntry> symlink(new SymlinkEntry(entry_offset));
  symlink->SetTarget(target, reader_writer, allocator);
  return symlink;
}

Path SymlinkEntry::GetTarget(ReaderWriter* reader) {
  std::string buf(kPathMax + 1, '\0');
  uint64_t cursor = sizeof(EntryLayout::SymlinkHeader);
  SectionSymlink sec_slnk = Section::Load(section_offset(), reader);
  size_t read = 0;
  while (1) {
    read += sec_slnk.Read(cursor, &buf[read], buf.size() - read, reader);
    if (read == buf.size())
      break;

    if (!sec_slnk.next_offset())
      break;

    sec_slnk = Section::Load(sec_slnk.next_offset(), reader);
    cursor = 0;
  }

  ErrorCode error_code;
  Path target = Path::Normalize(buf.c_str(), error_code);
  if (error_code != ErrorCode::kSuccess)
    throw FormatException();  // I'm sure the path was normalized before storing.
  return target;
}

void SymlinkEntry::SetTarget(const char* target, ReaderWriter* reader_writer,
                             SectionAllocator* allocator) {
  size_t target_size = strlen(target) + 1;
  uint64_t cursor = sizeof(EntryLayout::SymlinkHeader);
  SectionSymlink sec_slnk = Section::Load(section_offset(), reader_writer);
  while (1) {
    size_t written = sec_slnk.Write(cursor, target, target_size, reader_writer);
    target += written;
    target_size -= written;
    if (target_size == 0)
      break;

    SectionSymlink next_sec_slnk = allocator->AllocateSection(target_size,
                                                              reader_writer);
    try {
      sec_slnk.SetNext(next_sec_slnk.base_offset(), reader_writer);
    }
    catch (...) {
      allocator->ReleaseSection(next_sec_slnk, reader_writer);
      throw;
    }
    sec_slnk = next_sec_slnk;
    cursor = 0;
  }
}

}  // namespace linfs

}  // namespace fs
