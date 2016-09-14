#include "lib/entries/directory_entry.h"

namespace fs {

namespace linfs {

std::shared_ptr<FileEntry> FileEntry::Create(const Section& section,
                                             ReaderWriter* writer,
                                             ErrorCode& error_code,
                                             const char *name) {
  error_code = writer->Write(EntryLayout::FileHeader(Entry::Type::kFile, 0, name), section.data_offset());
  if (error_code != ErrorCode::kSuccess)
    return std::shared_ptr<FileEntry>();

  return make_shared<FileEntry>(section.data_offset(), 0);
}

FileEntry::~FileEntry() override {}

std::tuple<SectionFile, uint64_t> FileEntry::CursorToSection(uint64_t cursor, ReaderWriter* reader_writer, ErrorCode& error_code) {
  SectionFile sec_file = reader_writer->LoadSection<SectionFile>(section_offset(), error_code);
  if (error_code != ErrorCode::kSuccess)
    return std::make_tuple(sec_file, 0);

  cursor += sizeof(EntryLayout::FileHeader);
  while (cursor > sec_file.data_size() && sec_file.next_offset()) {
    cursor -= sec_file.data_size();
    sec_file = reader_writer->LoadSection<SectionFile>(sec_file.next_offset(), error_code);
    if (error_code != ErrorCode::kSuccess)
      return std::make_tuple(sec_file, 0);
  }

  if (cursor < sec_file.data_size())
    return std::make_tuple(sec_file, cursor);

  error_code = ErrorCode::kErrorNoData;
  return std::make_tuple(sec_file, 0);
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
