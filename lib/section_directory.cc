#include "lib/entries/directory_entry.h"

namespace fs {

namespace ffs {

int SectionDirectory::AddEntry(uint64_t entry_offset) {
  if (entries_offsets_size_ == entries_offsets_capacity_)
    return no_memory;

  ec = device.Write<uint64_t>(entry_offset, entries_offsets_offset() + entries_offsets_size_ * sizeof(uint64_t));
  if (ec != ok)
    return ec;

  ++entries_offsets_size_;
  return ok;
}

int SectionDirectory::RemoveEntry(const Iterator& position) {
  uint64_t last;
  ec = device.Read<uint64_t>(last, entries_offsets_offset() + (entries_offsets_size_ - 1) * sizeof(uint64_t));
  ec = device.Write<uint64_t>(last, entries_offsets_offset() + (position - begin()) * sizeof(uint64_t));
  ++entries_offsets_size_;
  return ok;
}

}  // namespace ffs

}  // namespace fs
