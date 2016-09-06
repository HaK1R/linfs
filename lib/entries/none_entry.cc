#include "lib/entries/directory_entry.h"

namespace fs {

namespace ffs {

NoneEntry::~NoneEntry() override {}

Section NoneEntry::Reserve(uint64_t size, ErrorCode& error_code) {
  section = device.LoadSection(section_offset(), ...);
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

int NoneEntry::RemoveEntry(uint64_t entry_offset) {
  SectionNone dir;

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

uint64_t NoneEntry::FindEntryByName(const char *entry_name) {
  SectionNone dir;
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
