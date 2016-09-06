#include "lib/entries/directory_entry.h"

namespace fs {

namespace ffs {

shared_ptr<DirectoryEntry> DirectoryEntry::CreateEntry(ErrorCode& error_code, uint64_t base_offset, const char *name) {
  ;
}

DirectoryEntry::~DirectoryEntry() override {}

int DirectoryEntry::AddEntry(uint64_t entry_offset) {
  uint64_t section_directory_offset = base_offset() +
      offsetof(EntryLayout::Header, type_traits.directory) + sizeof(EntryLayout::Header::type_traits::directory);

  SectionDirectory dir;
  
  dir = device.LoadSection(section_offset(), dir);
  if (!dir->IsFull()) {
    dir->AddEntry(entry_offset);
  }
  else if (dir->HasNext()) {
    dir = device.LoadSection(dir.next_offset(), dir);
  }
  else {
    new_dir = device.AllocateSection(1);
    dir->SetNext(new_dir);
    dir = new_dir;
  }
}

int DirectoryEntry::RemoveEntry(uint64_t entry_offset) {
  SectionDirectory dir;

  ec = device.LoadSection(section_offset(), dir);
  do {
  ec = dir->RemoveEntry(entry_offset);
  if (ec != not_fiund)
    return ec;
  if (!dir->HasNext())
    return not_found;
  ec = device.LoadSection(dir.next_offset(), dir);
  } while (true);
}

uint64_t DirectoryEntry::FindEntryByName(const char *entry_name) {
  SectionDirectory dir;
  ec = device.LoadSection(section_offset(), dir);

  while (1) {
    for (auto entry_offset : dir->entries_offsets) {
      Entry* sec = device.LoadEntry(entry_offset);
      if (!strcmp(entry_name, sec->name()))
        return entry_offset;
    }
    if (!dir->HasNext())
      return not_found;
    ec = device.LoadSection(dir.next_offset(), dir);
    if (ec != ok)
      return ec;
  }
}

}  // namespace ffs

}  // namespace fs
