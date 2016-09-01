#pragma once

#include <cstdint>

#include "lib/section.h"

namespace fs {

namespace ffs {

class SectionNone : public Section {
 public:
  SectionNone(uint64_t base_offset, uint64_t size = 0, uint64_t next_offset = 0)
      : Section(Type::kNone, base_offset), size_(size),
        next_offset_(next_offset) {}
  ~SectionNone() override {}

  uint64_t size() { return size_; }

  bool Empty() const { return size_ == 0; }
  int Reserve(uint64_t size);

 protected:
  uint64_t size_;
  uint64_t next_offset_;
};

}  // namespace ffs

}  // namespace fs
