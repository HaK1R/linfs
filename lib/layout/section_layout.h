#pragma once

#include <cstdint>

#include "lib/utils/macros.h"

namespace fs {

namespace linfs {

class SectionLayout {
 public:
  PACK(struct alignas(8) Header {
    uint64_t size;         // size of this section
    uint64_t next_offset;  // offset of the next section
  });
  STATIC_ASSERT_STANDARD_LAYOUT_AND_TRIVIALLY_COPYABLE(Header);

  // The section's body looks like:
  // struct Body {
  //  [EntryLayout::Header header;] -- optional
  //   EntryLayout::Body body;
  // };
};

}  // namespace linfs

}  // namespace fs
