#pragma once

#include <cstdint>

#include "include/interfaces/IFileSystem.h"

namespace fs {

namespace ffs {

enum SectionType : uint8_t {
  kEmpty = 0,
  kDirectory = 1,
  kFile = 2
};

struct __attribute__((packed, aligned(8))) SectionHeader {
  uint8_t type;                  // type of this section
  uint8_t reserved0[7];          // say hello to ARM64
  uint64_t next_section_offset;  // location of the next section (if any)
  char name[kNameMax];           // its name (valid only for kDirectory, kFile)
};

}  // namespace ffs

}  // namespace fs
