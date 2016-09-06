#pragma once

#include <cstdint>

#include "lib/entries/entry.h"

namespace fs {

namespace ffs {

class NoneEntry : public Entry {
 public:
  NoneEntry(uint64_t base_offset)
      : Entry(Type::kNone, base_offset) {}
  ~NoneEntry() override {}

  Section GetSection(uint64_t max_size, ErrorCode& error_code);
  ErrorCode PutSection(Section section);
};

}  // namespace ffs

}  // namespace fs
