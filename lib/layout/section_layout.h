#pragma once

#include <cstdint>
#include <fstream>

#include "include/interfaces/IFileSystem.h"

namespace fs {

namespace ffs {

class SectionLayout {
 public:
  struct __attribute__((packed)) Header {
    uint64_t size;         // size of this section
    uint64_t next_offset;  // offset of the next section
  };

  // The section's body looks like:
  // struct Body {
  //  [EntryLayout::Header header;]? -- optional
  //   EntryLayout::Body body;
  // };

  static bool WriteHeader(std::ofstream& file, const Header& header);
};

}  // namespace ffs

}  // namespace fs
