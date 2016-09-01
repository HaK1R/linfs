#include "lib/entries/directory_entry.h"

namespace fs {

namespace ffs {

DirectoryEntry::~DirectoryEntry() override {}

int DirectoryEntry::AddEntry(uint64_t entry_offset) {
  ;
}

int DirectoryEntry::RemoveEntry(uint64_t entry_offset) {
  ;
}

uint64_t DirectoryEntry::FindEntryByName(const char *entry_name) {
  ;
}

}  // namespace ffs

}  // namespace fs
