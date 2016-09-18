#include "lib/entries/directory_entry.h"

#include "lib/layout/entry_layout.h"

namespace fs {

namespace linfs {

std::unique_ptr<DirectoryEntry> DirectoryEntry::Create(uint64_t entry_offset,
                                                       uint64_t entry_size,
                                                       ReaderWriter* writer,
                                                       ErrorCode& error_code,
                                                       const char *name) {
  error_code = writer->Write<EntryLayout::DirectoryHeader>(EntryLayout::DirectoryHeader{name}, entry_offset);
  if (error_code != ErrorCode::kSuccess)
    return nullptr;

  error_code = ClearEntries(entry_offset + sizeof(EntryLayout::DirectoryHeader),
                            entry_offset + entry_size, writer);
  if (error_code != ErrorCode::kSuccess)
    return nullptr;

  return std::make_unique<DirectoryEntry>(entry_offset);
}

ErrorCode DirectoryEntry::AddEntry(std::shared_ptr<Entry> entry,
                                   ReaderWriter* reader_writer,
                                   SectionAllocator* allocator) {
  ErrorCode error_code;

  SectionDirectory sec_dir = reader_writer->LoadSection<SectionDirectory>(section_offset(), error_code);
  if (error_code != ErrorCode::kSuccess)
    return error_code;

  error_code = sec_dir.AddEntry(entry->base_offset(), reader_writer, sizeof(EntryLayout::DirectoryHeader));
  while (error_code == ErrorCode::kErrorNoMemory && sec_dir.next_offset()) {
    sec_dir = reader_writer->LoadSection<SectionDirectory>(sec_dir.next_offset(), error_code);
    if (error_code != ErrorCode::kSuccess)
      return error_code;

    error_code = sec_dir.AddEntry(entry->base_offset(), reader_writer);
  }

  if (error_code != ErrorCode::kErrorNoMemory)
    return error_code; // kSuccess or any of kError*

  SectionDirectory next_sec_dir = allocator->AllocateSection<SectionDirectory>(1, reader_writer, error_code);
  if (error_code != ErrorCode::kSuccess)
    return error_code;

  error_code = ClearEntries(next_sec_dir.data_offset(),
                            next_sec_dir.data_size(),
                            reader_writer);
  if (error_code != ErrorCode::kSuccess) {
    allocator->ReleaseSection(next_sec_dir, reader_writer);
    return error_code;
  }

  error_code = next_sec_dir.AddEntry(entry->base_offset(), reader_writer);
  if (error_code != ErrorCode::kSuccess) {
    allocator->ReleaseSection(next_sec_dir, reader_writer);
    return error_code;
  }

  // Update next directory entry stored in |sec_dir.next_offset()|
  error_code = sec_dir.SetNext(next_sec_dir.base_offset(), reader_writer);
  if (error_code != ErrorCode::kSuccess)
    allocator->ReleaseSection(next_sec_dir, reader_writer);

  return error_code;
}

ErrorCode DirectoryEntry::RemoveEntry(std::shared_ptr<Entry> entry,
                                      ReaderWriter* reader_writer,
                                      SectionAllocator* allocator) {
  ErrorCode error_code;

  SectionDirectory sec_dir = reader_writer->LoadSection<SectionDirectory>(section_offset(), error_code);
  if (error_code != ErrorCode::kSuccess)
    return error_code;

  error_code = sec_dir.RemoveEntry(entry->base_offset(), reader_writer, sizeof(EntryLayout::DirectoryHeader));
  while (error_code == ErrorCode::kErrorNotFound && sec_dir.next_offset()) {
    SectionDirectory prev_sec_dir = sec_dir;
    sec_dir = reader_writer->LoadSection<SectionDirectory>(sec_dir.next_offset(), error_code);
    if (error_code != ErrorCode::kSuccess)
      return error_code;

    error_code = sec_dir.RemoveEntry(entry->base_offset(), reader_writer);
    if (error_code == ErrorCode::kSuccess) {
      ErrorCode error_code_has_entries;
      bool has_entries = sec_dir.HasEntries(reader_writer, error_code_has_entries);
      if (error_code_has_entries == ErrorCode::kSuccess && !has_entries &&
          prev_sec_dir.SetNext(sec_dir.next_offset(), reader_writer) != ErrorCode::kSuccess)
        allocator->ReleaseSection(sec_dir, reader_writer);
      break;
    }
  }

  return error_code;
}

bool DirectoryEntry::HasEntries(ReaderWriter* reader_writer, ErrorCode& error_code) {
  SectionDirectory sec_dir = reader_writer->LoadSection<SectionDirectory>(section_offset(), error_code);
  if (error_code != ErrorCode::kSuccess)
    return false;

  bool has_entries = sec_dir.HasEntries(reader_writer, error_code, sizeof(EntryLayout::DirectoryHeader));
  while (error_code == ErrorCode::kSuccess && !has_entries && sec_dir.next_offset()) {
    sec_dir = reader_writer->LoadSection<SectionDirectory>(sec_dir.next_offset(), error_code);
    if (error_code != ErrorCode::kSuccess)
      return false;

    has_entries = sec_dir.HasEntries(reader_writer, error_code);
  }

  return has_entries;
}

std::unique_ptr<Entry> DirectoryEntry::FindEntryByName(const char *entry_name,
                                                       ReaderWriter* reader_writer,
                                                       ErrorCode& error_code,
                                                       SectionDirectory* section_directory,
                                                       SectionDirectory::Iterator* iterator) {
  uint64_t start_position = sizeof(EntryLayout::DirectoryHeader);
  SectionDirectory sec_dir = reader_writer->LoadSection<SectionDirectory>(section_offset(), error_code);
  if (error_code != ErrorCode::kSuccess)
    return nullptr;

  while (1) {
    for (SectionDirectory::Iterator it = sec_dir.EntriesBegin(reader_writer, error_code, start_position);
         it != sec_dir.EntriesEnd(); ++it) {
      if (error_code != ErrorCode::kSuccess)
        return nullptr;
      if (*it == 0)
        continue;
      char it_name[kNameMax + 1];
      std::unique_ptr<Entry> it_entry = reader_writer->LoadEntry(*it, error_code, it_name);
      if (error_code != ErrorCode::kSuccess)
        return nullptr;

      if (strcmp(entry_name, it_name) == 0) {
        *section_directory = sec_dir;
        *iterator = it;
        return it_entry;
      }
    }

    if (!sec_dir.next_offset()) {
      error_code = ErrorCode::kErrorNotFound;
      return nullptr;
    }

    sec_dir = reader_writer->LoadSection<SectionDirectory>(sec_dir.next_offset(), error_code);
    if (error_code != ErrorCode::kSuccess)
      return nullptr;

    start_position = 0;
  }
}

ErrorCode DirectoryEntry::GetNextEntryName(const char *prev, ReaderWriter* reader_writer, char* next_buf) {
  ErrorCode error_code;
  SectionDirectory sec_dir = {0,0,0}; // TODO compile; remove
  SectionDirectory::Iterator it(0);

  FindEntryByName(prev, reader_writer, error_code, &sec_dir, &it);
  while (error_code == ErrorCode::kSuccess) {
    while (++it != sec_dir.EntriesEnd()) {
      if (error_code != ErrorCode::kSuccess)
        return error_code;
      if (*it == 0)
        continue;
      reader_writer->LoadEntry(*it, error_code, next_buf);
      return error_code;
    }

    if (!sec_dir.next_offset())
      return ErrorCode::kErrorNotFound;

    sec_dir = reader_writer->LoadSection<SectionDirectory>(sec_dir.next_offset(), error_code);
  }

  return error_code;
}

ErrorCode DirectoryEntry::ClearEntries(uint64_t entries_offset, uint64_t entries_end, ReaderWriter* reader_writer) {
  while (entries_offset != entries_end) {
    ErrorCode error_code = reader_writer->Write<uint64_t>(0, entries_offset);
    if (error_code != ErrorCode::kSuccess)
      return error_code;

    entries_offset += sizeof(uint64_t);
  }

  return ErrorCode::kSuccess;
}

}  // namespace linfs

}  // namespace fs
