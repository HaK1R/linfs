#pragma once

#include <cstdint>

#include "include/interfaces/IFileSystem.h"
#include "lib/section.h"

namespace fs {

namespace ffs {

class SectionDirectory : public Section {
 public:
  SectionDirectory(uint64_t base_offset, uint64_t available, uint64_t next_offset, const char *name)
      : Section(Type::kDirectory, base_offset), available_(available),
        next_offset_(next_offset), name_(name) {}
  ~SectionDirectory() override {}

  int AddEntry(uint64_t entry_offset);
  int RemoveEntry(uint64_t entry_offset);
  uint64_t FindEntryByName(const char *entry_name);

 protected:
  uint64_t available_;
  uint64_t next_offset_;
  char name_[kNameMax];
};

}  // namespace ffs

}  // namespace fs
