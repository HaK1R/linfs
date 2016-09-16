#pragma once

#include <cstdint>
#include <type_traits>

namespace fs {

namespace linfs {

class SectionLayout {
 public:
  struct __attribute__((packed)) Header {
    uint64_t size;         // size of this section
    uint64_t next_offset;  // offset of the next section
  };
  static_assert(std::is_trivially_copyable<Header>::value,
                "SectionLayout::Header isn't a trivially copyable type");
  static_assert(std::is_standard_layout<Header>::value,
                "SectionLayout::Header isn't a standard-layout class");

  // The section's body looks like:
  // struct Body {
  //  [EntryLayout::Header header;]? -- optional
  //   EntryLayout::Body body;
  // };
};

// TODO use?
//ReaderWriter::Stream& operator<<(ReaderWriter::Stream& stream,
//        SectionLayout::Header header) {
//  header.size = header.size;
//  header.next_offset = header.next_offset;
//  stream << header.size << header.next_offset;
//}

}  // namespace linfs

}  // namespace fs
