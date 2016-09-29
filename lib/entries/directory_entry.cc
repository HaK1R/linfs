#include "lib/entries/directory_entry.h"

#include <cassert>

#include "lib/layout/entry_layout.h"
#include "lib/sections/section_directory.h"

namespace fs {

namespace linfs {

std::unique_ptr<DirectoryEntry> DirectoryEntry::Create(uint64_t entry_offset,
                                                       uint64_t entry_size,
                                                       ReaderWriter* writer,
                                                       const char* name) {
  writer->Write<EntryLayout::DirectoryHeader>(EntryLayout::DirectoryHeader(name), entry_offset);
  ClearEntries(entry_offset + sizeof(EntryLayout::DirectoryHeader),
               entry_offset + entry_size, writer);
  return std::make_unique<DirectoryEntry>(entry_offset);
}

void DirectoryEntry::AddEntry(const Entry* entry, ReaderWriter* reader_writer,
                              SectionAllocator* allocator) {
  SectionDirectory sec_dir = Section::Load(section_offset(), reader_writer);

  bool success = sec_dir.AddEntry(entry->base_offset(), reader_writer,
                                  sizeof(EntryLayout::DirectoryHeader));
  while (!success && sec_dir.next_offset()) {
    sec_dir = Section::Load(sec_dir.next_offset(), reader_writer);
    success = sec_dir.AddEntry(entry->base_offset(), reader_writer);
  }
  if (success)
    return;

  SectionDirectory next_sec_dir = allocator->AllocateSection(1, reader_writer);
  try {
    ClearEntries(next_sec_dir.data_offset(),
                 next_sec_dir.data_offset() + next_sec_dir.data_size(),
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

bool DirectoryEntry::RemoveEntry(const Entry* entry, ReaderWriter* reader_writer,
                                 SectionAllocator* allocator) {
  SectionDirectory sec_dir = Section::Load(section_offset(), reader_writer);
  SectionDirectory last_used_sec_dir = sec_dir;  // It's always used.

  bool success = sec_dir.RemoveEntry(entry->base_offset(), reader_writer,
                                     sizeof(EntryLayout::DirectoryHeader));
  while (!success && sec_dir.next_offset()) {
    sec_dir = Section::Load(sec_dir.next_offset(), reader_writer);
    success = sec_dir.RemoveEntry(entry->base_offset(), reader_writer);

    // Track the last non-empty section and release unused ones when possible.
    try {
      if (sec_dir.HasEntries(reader_writer))
        last_used_sec_dir = sec_dir;
      else if (!sec_dir.next_offset()) {
        // Note that |success| is usually true but can also be
        // false if at the last time there was an exception.
        Section unused = Section::Load(last_used_sec_dir.next_offset(), reader_writer);
        last_used_sec_dir.SetNext(0, reader_writer);
        allocator->ReleaseSection(unused, reader_writer);
      }
    }
    catch (...) {
      /* Oh well... it doesn't matter. Let's just say sec_dir has entries. */
      last_used_sec_dir = sec_dir;
    }
  }

  return success;
}

bool DirectoryEntry::HasEntries(ReaderWriter* reader) {
  SectionDirectory sec_dir = Section::Load(section_offset(), reader);

  bool has_entries = sec_dir.HasEntries(reader, sizeof(EntryLayout::DirectoryHeader));
  while (!has_entries && sec_dir.next_offset()) {
    sec_dir = Section::Load(sec_dir.next_offset(), reader);
    has_entries = sec_dir.HasEntries(reader);
  }

  return has_entries;
}

std::unique_ptr<Entry> DirectoryEntry::FindEntryByName(const char* entry_name,
                                                       ReaderWriter* reader) {
  SectionDirectory sec_dir = Section::Load(section_offset(), reader);
  SectionDirectory::Iterator it = sec_dir.EntriesBegin(reader, sizeof(EntryLayout::DirectoryHeader));

  while (1) {
    for (; it != sec_dir.EntriesEnd(); ++it) {
      if (*it == 0)
        continue;

      char it_name[kNameMax + 1];
      std::unique_ptr<Entry> it_entry = Entry::Load(*it, reader, it_name);
      if (strcmp(entry_name, it_name) == 0)
        return it_entry;
    }

    if (!sec_dir.next_offset())
      return nullptr;

    sec_dir = Section::Load(sec_dir.next_offset(), reader);
    it = sec_dir.EntriesBegin(reader);
  }
}

uint64_t DirectoryEntry::GetNextEntryName(uint64_t cursor, ReaderWriter* reader,
                                          char* next_buf) {
  uint64_t start_position = cursor;
  SectionDirectory sec_dir =
      CursorToSection(start_position, reader, sizeof(EntryLayout::DirectoryHeader),
                      false);  // check_cursor -- We will check it by ourselves.
  if (start_position > sec_dir.data_size())
    return 0;  // Some sections were probably released.  Ignore it.

  if (start_position % sizeof(SectionDirectory::Iterator::value_type) != 0)
    return 0;  // Invalid cursor.  Ignore it.

  while (1) {
    for (SectionDirectory::Iterator it = sec_dir.EntriesBegin(reader, start_position);
         it != sec_dir.EntriesEnd(); ++it) {
      if (*it == 0)
        continue;

      Entry::Load(*it++, reader, next_buf);
      uint64_t begin_position = sec_dir.data_offset() + start_position;
      return cursor + it.position() - begin_position;
    }

    if (!sec_dir.next_offset())
      return 0;

    cursor += sec_dir.data_size() - start_position;
    sec_dir = Section::Load(sec_dir.next_offset(), reader);
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
