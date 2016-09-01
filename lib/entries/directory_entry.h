#pragma once

#include <cstdint>

#include "include/IFileSystem.h"
#include "lib/entries/entry.h"

namespace fs {

namespace ffs {

class DirectoryEntry : public Entry {
 public:
  DirectoryEntry(uint64_t base_offset, const char *name)
      : Section(Type::kDirectory, base_offset), name_(name) {}
  ~DirectoryEntry() override {}

  const char* name() const { return name_; }

  int AddEntry(uint64_t entry_offset);
  int RemoveEntry(uint64_t entry_offset);
  uint64_t FindEntryByName(const char *entry_name);

 protected:
  char name_[kNameMax];
};

}  // namespace ffs

}  // namespace fs
