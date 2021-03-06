#include "lib/entries/file_entry.h"

#include "lib/layout/entry_layout.h"
#include "lib/sections/section_file.h"
#include "lib/utils/format_exception.h"

namespace fs {

namespace linfs {

std::unique_ptr<FileEntry> FileEntry::Create(uint64_t entry_offset,
                                             uint64_t /* entry_size */,
                                             ReaderWriter* writer,
                                             const char* name) {
  writer->Write<EntryLayout::FileHeader>(EntryLayout::FileHeader(0, name), entry_offset);
  return std::make_unique<FileEntry>(entry_offset, 0);
}

size_t FileEntry::Read(uint64_t cursor, char* buf, size_t buf_size, ReaderWriter* reader) {
  size_t read = 0;
  SectionFile sec_file = CursorToSection(cursor, reader, sizeof(EntryLayout::FileHeader));
  while (1) {
    size_t rc = sec_file.Read(cursor, buf, buf_size, reader);
    read += rc;
    buf += rc;
    buf_size -= rc;
    if (buf_size == 0)
      break;

    if (!sec_file.next_offset())
      throw FormatException();  // file size is smaller than expected

    sec_file = Section::Load(sec_file.next_offset(), reader);
    cursor = 0;
  }

  return read;
}

size_t FileEntry::Write(uint64_t cursor, const char* buf, size_t buf_size,
                        ReaderWriter* reader_writer, SectionAllocator* allocator) {
  size_t written = 0;
  uint64_t old_cursor = cursor;
  SectionFile sec_file = CursorToSection(cursor, reader_writer, sizeof(EntryLayout::FileHeader));
  while (1) {
    size_t rc = sec_file.Write(cursor, buf, buf_size, reader_writer);
    written += rc;
    buf += rc;
    buf_size -= rc;
    if (buf_size == 0)
      break;

    if (!sec_file.next_offset()) {
      SectionFile next_sec_file = allocator->AllocateSection(buf_size, reader_writer);
      try {
        sec_file.SetNext(next_sec_file.base_offset(), reader_writer);
      }
      catch (...) {
        allocator->ReleaseSection(next_sec_file, reader_writer);
        throw;
      }
      sec_file = next_sec_file;
    }
    else
      sec_file = Section::Load(sec_file.next_offset(), reader_writer);
    cursor = 0;
  }

  if (old_cursor + written > size())
    SetSize(old_cursor + written, reader_writer);

  return written;
}

void FileEntry::SetSize(uint64_t size, ReaderWriter* writer) {
  writer->Write<uint64_t>(size, base_offset() + offsetof(EntryLayout::FileHeader, size));
  size_ = size;
}

}  // namespace linfs

}  // namespace fs
