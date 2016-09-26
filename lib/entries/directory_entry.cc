#include "lib/entries/directory_entry.h"

#include <cassert>

#include "lib/layout/entry_layout.h"

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
                                 SectionAllocator* allocator) {
  SectionDirectory sec_dir = reader_writer->LoadSection<SectionDirectory>(section_offset());

  bool success = sec_dir.RemoveEntry(entry->base_offset(), reader_writer,
                                     sizeof(EntryLayout::DirectoryHeader));
  while (!success && sec_dir.next_offset()) {
    SectionDirectory prev_sec_dir = sec_dir;
    sec_dir = reader_writer->LoadSection<SectionDirectory>(sec_dir.next_offset());

    success = sec_dir.RemoveEntry(entry->base_offset(), reader_writer);
    if (success) {
      try {
        if (!sec_dir.HasEntries(reader_writer)) {
          prev_sec_dir.SetNext(sec_dir.next_offset(), reader_writer);
          allocator->ReleaseSection(sec_dir, reader_writer);
        }
      }
      catch (...) { /* oh well... it doesn't matter */ }
    }
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
  return FindEntryByName(entry_name, reader, nullptr, nullptr);
}

std::unique_ptr<Entry> DirectoryEntry::FindEntryByName(const char* entry_name,
                                                       ReaderWriter* reader,
                                                       SectionDirectory* section_directory,
                                                       SectionDirectory::Iterator* iterator) {
  uint64_t start_position = sizeof(EntryLayout::DirectoryHeader);
  SectionDirectory sec_dir = reader->LoadSection<SectionDirectory>(section_offset());

  while (1) {
    for (SectionDirectory::Iterator it = sec_dir.EntriesBegin(reader, start_position);
         it != sec_dir.EntriesEnd(); ++it) {
      uint64_t it_offset = *it;
      if (it_offset == 0)
        continue;
      char it_name[kNameMax + 1];
      std::unique_ptr<Entry> it_entry = reader->LoadEntry(it_offset, it_name);
      if (entry_name == nullptr || strcmp(entry_name, it_name) == 0) {
        if (entry_name != nullptr && section_directory != nullptr) {
          *section_directory = sec_dir;
          *iterator = it;
        }
        return it_entry;
      }
    }

    if (!sec_dir.next_offset())
      return nullptr;

    sec_dir = reader->LoadSection<SectionDirectory>(sec_dir.next_offset());
    start_position = 0;
  }
}

const char* DirectoryEntry::GetNextEntryName(const char* prev, ReaderWriter* reader,
                                             char* next_buf) {
  // TODO? better
  SectionDirectory sec_dir = {0,0,0}; // TODO compile; remove
  SectionDirectory::Iterator it(0);
  std::unique_ptr<Entry> entry = FindEntryByName(prev, reader, &sec_dir, &it);
  if (entry == nullptr)
    // There are no entries in the directory.
    return nullptr;
  if (prev == nullptr) {
    // Get first entry
    reader->LoadEntry(entry->base_offset(), next_buf);
    return next_buf;
  }

  ++it;

  // Get the next entry
  while (1) {
    while (it != sec_dir.EntriesEnd()) {
      uint64_t it_offset = *it++;
      if (it_offset == 0)
        continue;
      reader->LoadEntry(it_offset, next_buf);
      return next_buf;
    }

    if (!sec_dir.next_offset())
      return nullptr;

    sec_dir = reader->LoadSection<SectionDirectory>(sec_dir.next_offset());
    it = sec_dir.EntriesBegin(reader);
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
