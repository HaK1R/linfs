#pragma once

#include <cstdint>

namespace fs {

namespace ffs {

class Section {
 public:
  Section(uint64_t base_offset) : base_offset_(base_offset) {}

  Section(const Section& section) = delete;
  void operator=(const Section& section) = delete;

  uint64_t base_offset() const { return base_offset_; }
  uint64_t size() const { return size_; }
  uint64_t next_offset() const { return next_offset_; }

 private:
  uint64_t base_offset_;
  uint64_t size_;
  uint64_t next_offset_;
};

}  // namespace ffs

}  // namespace fs
