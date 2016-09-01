#pragma once

#include <cstdint>

#include "include/IFileSystem.h"
#include "lib/entries/entry.h"

namespace fs {

namespace ffs {

class FileEntry : public Entry {
 public:
  FileEntry(uint64_t base_offset, const char *name)
      : Entry(Type::kFile, base_offset), name_(name) {}
  ~FileEntry() override {}

  const char* name() const { return name_; }

  size_t Read(uint64_t position, char *buffer, size_t buffer_size);
  size_t Write(uint64_t position, const char *buffer, size_t buffer_size);

 protected:
  char name_[kNameMax];
};

}  // namespace ffs

}  // namespace fs
