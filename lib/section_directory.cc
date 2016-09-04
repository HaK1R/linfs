#include "lib/entries/directory_entry.h"

namespace fs {

namespace ffs {

int SectionDirectory::AddEntry(uint64_t entry_offset) {
  if (available_ < sizeof entry_offset)
    return no_memory;

  ec = device.Write<uint64_t>(entry_offset, entries_offsets_offset() + entries_offsets_size() - available_);
  if (ec != ok)
    return ec;

  available_ -= sizeof(uint64_t);

  return ok;
}

int SectionDirectory::RemoveEntry(uint64_t entry_offset) {
  for (uint64_t tmp_offset = entries_offsets_offset(),
       end = entries_offsets_offset() + entries_offsets_size() - available_;
       tmp_offset != end; tmp_offset += sizeof(uint64_t)) {
    uint64_t tmp;
    ec = device.Read<uint64_t>(tmp, tmp_offset);
    if (tmp == entry_offset) {
      uint64_t last;
      ec = device.Read<uint64_t>(last, entries_offsets_offset() + entries_offsets_size() - available_ - sizeof(uint64_t));
      ec = device.Write<uint64_t>(last, tmp_offset);
      available_ += sizeof(uint64_t);
      return ok;
    }
  }

  return not_found
}

uint64_t DirectoryEntry::FindEntryByName(const char *entry_name) {
  load
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
