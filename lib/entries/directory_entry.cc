#include "lib/entries/directory_entry.h"

namespace fs {

namespace ffs {

std::shared_ptr<DirectoryEntry> DirectoryEntry::Create(const Section& section,
                                                       ReaderWriter* writer,
                                                       ErrorCode& error_code,
                                                       const char *name) {
  error_code = writer->Write(EntryLayout::DirectoryHeader(Entry::Type::kDirectory, name), section.data_offset());
  if (error_code != ErrorCode::kSuccess)
    return std::shared_ptr<DirectoryEntry>();

  return make_shared<DirectoryEntry>(section.data_offset());
}

DirectoryEntry::DirectoryEntry(uint64_t base_offset)
  : Section(Type::kDirectory, base_offset) {}

DirectoryEntry::~DirectoryEntry() override {}

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

  SectionDirectory next_sec_dir = allocator->AllocateSection<SectionDirectory>(1, error_code);
  if (error_code != ErrorCode::kSuccess)
    return error_code;

  error_code = next_sec_dir.Clear(reader_writer);
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

  sec_dir.RemoveEntry(entry->base_offset(), reader_writer, error_code, sizeof(EntryLayout::DirectoryHeader));
  while (error_code == ErrorCode::kErrorNotFound && sec_dir.next_offset()) {
    SectionDirectory prev_sec_dir = sec_dir;
    sec_dir = reader_writer->LoadSection<SectionDirectory>(sec_dir.next_offset(), error_code);
    if (error_code != ErrorCode::kSuccess)
      return error_code;

    error_code = sec_dir.RemoveEntry(entry->base_offset(), reader_writer);
    if (error_code == ErrorCode::kSuccess) {
      ErrorCode error_code_has_entries;
      bool has_entries = sec_dir.HasEntries(reader_writer, error_code_has_entries);
      if (error_code_has_entries == ErrorCode::kSuccess && has_entries &&
          prev_sec_dir.SetNext(sec_dir.next_offset(), reader_writer) != ErrorCode::kSuccess)
        allocator->ReleaseSection(next_sec_dir, reader_writer);
      break;
    }
  }

  return error_code;
}

std::shared_ptr<Entry> DirectoryEntry::FindEntryByName(const char *entry_name,
                                                       ReaderWriter* reader_writer,
                                                       ErrorCode& error_code,
                                                       SectionDirectory* section_directory,
                                                       SectionDirectory::Iterator* iterator) {
  uint64_t start_position = sizeof(EntryLayout::DirectoryHeader);
  SectionDirectory sec_dir = reader_writer->LoadSection<SectionDirectory>(sectiion_offset(), error_code);
  if (error_code != ErrorCode::kSuccess)
    return std::shared_ptr<Entry>();

  while (1) {
    for (SectionDirectory::Iterator it = sec_dir.EntriesBegin(start_position, reader_writer, error_code);
         it != sec_dir.EntriesEnd(); ++it) {
      if (error_code != ErrorCode::kSuccess)
        return std::shared_ptr<Entry>();
      if (*it == 0)
        continue;
      uint64_t it_offset = *it;
      char it_name[kNameMax + 1];
      std::shared_ptr<Entry> entry = reader_writer->LoadEntry(it_offset, it_name, error_code);
      if (error_code != ErrorCode::kSuccess)
        return std::shared_ptr<Entry>();

      if (!strcmp(entry_name, it_name)) {
        *section_directory = sec_dir;
        *iterator = it;
        return entry;
      }
    }

    if (!sec_dir.next_offset()) {
      error_code = ErrorCode::kErrorNotFound;
      return std::shared_ptr<Entry>();
    }

    sec_dir = reader_writer->LoadSection<SectionDirectory>(sec_dir.next_offset(), error_code);
    if (error_code != ErrorCode::kSuccess)
      return std::shared_ptr<Entry>();

    start_position = 0;
  }
}

ErrorCode DirectoryEntry::GetNextEntryName(const char *prev, ReaderWriter* reader_writer, char* next_buf) {
  ErrorCode error_code;
  SectionDirectory sec_dir;
  SectionDirectory::Iterator it;

  FindEntryByName(prev, reader_writer, error_code, &sec_dir, &it);
  while (error_code == ErrorCode::kSuccess) {
    while (++it != sec_dir.EntriesEnd()) {
      if (error_code != ErrorCode::kSuccess)
        return error_code;
      if (*it == 0)
        continue;
      reader_writer->LoadEntry(*it, next_buf, error_code);
      return error_code;
    }

    if (!sec_dir.next_offset())
      return ErrorCode::kErrorNotFound;

    sec_dir = reader_writer->LoadSection<SectionDirectory>(sec_dir.next_offset(), error_code);
  }

  return error_code;
}

}  // namespace ffs

}  // namespace fs
