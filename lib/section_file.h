#pragma once

#include <cstdint>

#include "lib/section.h"

namespace fs {

namespace ffs {

class SectionFile : public Section {
 public:
  SectionFile(uint64_t base_offset, uint64_t size, uint64_t next_offset, const char *name)
      : Section(Type::kFile, base_offset), size_(size),
        next_offset_(next_offset), name_(name) {}
  ~SectionFile() override {}

  size_t Read(uint64_t position, char *buffer, size_t buffer_size);
  size_t Write(uint64_t position, const char *buffer, size_t buffer_size);

 protected:
  uint64_t size_;
  uint64_t next_offset_;
  char name_[kNameMax];
};

}  // namespace ffs

}  // namespace fs
