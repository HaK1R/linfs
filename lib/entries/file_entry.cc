#include "lib/entries/file_entry.h"

#include "lib/layout/entry_layout.h"

namespace fs {

namespace linfs {

std::unique_ptr<FileEntry> FileEntry::Create(uint64_t base_offset,
                                             ReaderWriter* writer,
                                             ErrorCode& error_code,
                                             const char *name) {
  error_code = writer->Write(EntryLayout::FileHeader(Entry::Type::kFile, 0, name), base_offset);
  if (error_code != ErrorCode::kSuccess)
    return nullptr;

  return std::make_unique<FileEntry>(base_offset, 0);
}

SectionFile FileEntry::CursorToSection(uint64_t& cursor, ReaderWriter* reader_writer, ErrorCode& error_code) {
  SectionFile sec_file = reader_writer->LoadSection<SectionFile>(section_offset(), error_code);
  if (error_code != ErrorCode::kSuccess)
    return SectionFile();

  cursor += sizeof(EntryLayout::FileHeader);
  while (cursor > sec_file.data_size() && sec_file.next_offset()) {
    cursor -= sec_file.data_size();
    sec_file = reader_writer->LoadSection<SectionFile>(sec_file.next_offset(), error_code);
    if (error_code != ErrorCode::kSuccess)
      return SectionFile();
  }

  if (cursor < sec_file.data_size())
    return sec_file;

  error_code = ErrorCode::kErrorNoData;
  return SectionFile();
}

size_t FileEntry::Read(uint64_t cursor, char *buf, size_t buf_size, ReaderWriter* reader_writer, ErrorCode& error_code) {
  size_t read = 0;
  SectionFile sec_file = CursorToSection(cursor, reader_writer, error_code);
  while (error_code != ErrorCode::kSuccess) {
    size_t rc = sec_file.Read(cursor, buf, buf_size, reader_writer, error_code);
    if (error_code != ErrorCode::kSuccess)
      break;

    read += rc;
    buf += rc;
    buf_size -= rc;
    if (buf_size == 0)
      break;

    if (!sec_file.next_offset()) {
      error_code = ErrorCode::kErrorBrokenFile; // TODO? kErrorFile
      break;
    }

    sec_file = reader_writer->LoadSection<SectionFile>(sec_file.next_offset(), error_code);
    cursor = 0;
  }

  return read;
}

size_t FileEntry::Write(uint64_t cursor, const char *buf, size_t buf_size, ReaderWriter* reader_writer, SectionAllocator* allocator, ErrorCode& error_code) {
  size_t written = 0;
  SectionFile sec_file = CursorToSection(cursor, reader_writer, error_code);
  while (error_code != ErrorCode::kSuccess) {
    size_t rc = sec_file.Write(cursor, buf, buf_size, reader_writer, error_code);
    if (error_code != ErrorCode::kSuccess)
      break;

    written += rc;
    buf += rc;
    buf_size -= rc;
    if (buf_size == 0)
      break;

    if (!sec_file.next_offset()) {
      SectionFile next_sec_file = allocator->AllocateSection<SectionFile>(buf_size, reader_writer, error_code);
      if (error_code != ErrorCode::kSuccess)
        break;

      error_code = sec_file.SetNext(next_sec_file.base_offset(), reader_writer);
      if (error_code != ErrorCode::kSuccess)
        allocator->ReleaseSection(next_sec_file, reader_writer);
    }
    else
      sec_file = reader_writer->LoadSection<SectionFile>(sec_file.next_offset(), error_code);
    cursor = 0;
  }

  return written;
}

}  // namespace linfs

}  // namespace fs
