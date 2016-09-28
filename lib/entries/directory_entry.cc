#include "lib/entries/directory_entry.h"

#include <cassert>

#include "lib/layout/entry_layout.h"
#include "lib/utils/format_exception.h"

namespace fs {

namespace linfs {

std::unique_ptr<DirectoryEntry> DirectoryEntry::Create(uint64_t entry_offset,
                                                       uint64_t entry_size,
                                                       ReaderWriter* writer,
                                                       const char* name) {
  writer->Write<EntryLayout::DirectoryHeader>(EntryLayout::DirectoryHeader(name), entry_offset);
  ClearEntries(entry_offset + sizeof(EntryLayout::DirectoryHeader), entry_offset + entry_size,
               writer);
  return std::make_unique<DirectoryEntry>(entry_offset);
}

void DirectoryEntry::AddEntry(const Entry* entry,
                              ReaderWriter* reader_writer,
                              SectionAllocator* allocator) {
  SectionDirectory sec_dir = reader_writer->LoadSection<SectionDirectory>(section_offset());

  bool success = sec_dir.AddEntry(entry->base_offset(), reader_writer,
                                  sizeof(EntryLayout::DirectoryHeader));
  while (!success && sec_dir.next_offset()) {
    sec_dir = reader_writer->LoadSection<SectionDirectory>(sec_dir.next_offset());
    success = sec_dir.AddEntry(entry->base_offset(), reader_writer);
  }
  if (success)
    return;

  SectionDirectory next_sec_dir = allocator->AllocateSection<SectionDirectory>(1, reader_writer);
  try {
    ClearEntries(next_sec_dir.data_offset(), next_sec_dir.data_offset() + next_sec_dir.data_size(),
                 reader_writer);
    success = next_sec_dir.AddEntry(entry->base_offset(), reader_writer);
    assert(success && "no space in the just allocated section");

    // Update next directory entry stored in |sec_dir.next_offset()|
    sec_dir.SetNext(next_sec_dir.base_offset(), reader_writer);
  }
  catch (...) {
    allocator->ReleaseSection(next_sec_dir, reader_writer);
    throw;
  }
}

bool DirectoryEntry::RemoveEntry(const Entry* entry,
                                 ReaderWriter* reader_writer,
                                 SectionAllocator*) {
  SectionDirectory sec_dir = reader_writer->LoadSection<SectionDirectory>(section_offset());

  bool success = sec_dir.RemoveEntry(entry->base_offset(), reader_writer,
                                     sizeof(EntryLayout::DirectoryHeader));
  while (!success && sec_dir.next_offset()) {
    sec_dir = reader_writer->LoadSection<SectionDirectory>(sec_dir.next_offset());
    success = sec_dir.RemoveEntry(entry->base_offset(), reader_writer);
  }

  return success;
}

bool DirectoryEntry::HasEntries(ReaderWriter* reader) {
  SectionDirectory sec_dir = reader->LoadSection<SectionDirectory>(section_offset());

  bool has_entries = sec_dir.HasEntries(reader, sizeof(EntryLayout::DirectoryHeader));
  while (!has_entries && sec_dir.next_offset()) {
    sec_dir = reader->LoadSection<SectionDirectory>(sec_dir.next_offset());
    has_entries = sec_dir.HasEntries(reader);
  }

  return has_entries;
}

std::unique_ptr<Entry> DirectoryEntry::FindEntryByName(const char* entry_name,
                                                       ReaderWriter* reader) {
  SectionDirectory sec_dir = reader->LoadSection<SectionDirectory>(section_offset());
  SectionDirectory::Iterator it =
      sec_dir.EntriesBegin(reader, sizeof(EntryLayout::DirectoryHeader));

  while (1) {
    for (; it != sec_dir.EntriesEnd(); ++it) {
      if (*it == 0)
        continue;

      char it_name[kNameMax + 1];
      std::unique_ptr<Entry> it_entry = reader->LoadEntry(*it, it_name);
      if (strcmp(entry_name, it_name) == 0)
        return it_entry;
    }

    if (!sec_dir.next_offset())
      return nullptr;

    sec_dir = reader->LoadSection<SectionDirectory>(sec_dir.next_offset());
    it = sec_dir.EntriesBegin(reader);
  }
}

SectionDirectory DirectoryEntry::CursorToSection(uint64_t& cursor, ReaderWriter* reader) {
  SectionDirectory sec_dir = reader->LoadSection<SectionDirectory>(section_offset());
  cursor += sizeof(EntryLayout::DirectoryHeader);
  while (cursor >= sec_dir.data_size() && sec_dir.next_offset()) {
    cursor -= sec_dir.data_size();
    sec_dir = reader->LoadSection<SectionDirectory>(sec_dir.next_offset());
  }

  if (cursor > sec_dir.data_size())
    throw FormatException();

  return sec_dir;
}

uint64_t DirectoryEntry::GetNextEntryName(uint64_t cursor, ReaderWriter* reader,
                                          char* next_buf) {
  uint64_t start_position = cursor;
  SectionDirectory sec_dir = CursorToSection(start_position, reader);
  if (start_position % sizeof(SectionDirectory::Iterator::value_type) != 0)
    throw FormatException();  // cursor is not aligned

  while (1) {
    for (SectionDirectory::Iterator it = sec_dir.EntriesBegin(reader, start_position);
         it != sec_dir.EntriesEnd(); ++it) {
      if (*it == 0)
        continue;

      reader->LoadEntry(*it, next_buf);
      return cursor + ((++it).position() - (sec_dir.data_offset() + start_position));
    }

    if (!sec_dir.next_offset())
      return 0;

    cursor += sec_dir.data_size() - start_position;
    sec_dir = reader->LoadSection<SectionDirectory>(sec_dir.next_offset());
    start_position = 0;
  }
}

void DirectoryEntry::ClearEntries(uint64_t entries_offset, uint64_t entries_end,
                                  ReaderWriter* writer) {
  while (entries_offset != entries_end) {
    writer->Write<uint64_t>(0, entries_offset);
    entries_offset += sizeof(uint64_t);
  }
}

}  // namespace linfs

}  // namespace fs
