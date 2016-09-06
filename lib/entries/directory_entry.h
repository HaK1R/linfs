#pragma once

#include <cstdint>

#include "include/IFileSystem.h"
#include "lib/entries/entry.h"

namespace fs {

namespace ffs {

class DirectoryEntry : public Entry {
 public:
  static shared_ptr<DirectoryEntry> CreateEntry(ErrorCode& error_code, uint64_t base_offset, const char *name);

  DirectoryEntry(uint64_t base_offset, const char *name)
      : Section(Type::kDirectory, base_offset), name_(name) {}
  ~DirectoryEntry() override {}

  const char* name() const { return name_; }

  class Iterator : public std::iterator<std::input_iterator_tag, const char*> {
   public:
    ;
   private:
    uint64_t position_;
  };

  int AddEntry(uint64_t entry_offset);
  int RemoveEntry(uint64_t entry_offset);
  uint64_t FindEntryByName(const char *entry_name);

 protected:
  char name_[kNameMax + 1];
};

}  // namespace ffs

}  // namespace fs
