#include "lib/layout/section_layout.h"

#include <cstddef>

namespace fs {

namespace ffs {

bool SectionLayout::WriteHeader(std::ofstream& file, const Header& header) {
  uint64_t header_size = offsetof(Header, type_traits);
  switch (header.type) {
    case kNone:
      header_size += sizeof header.type_traits.none;
      break;
    case kDirectory:
      header_size += sizeof header.type_traits.directory;
      break;
    case kFile:
      header_size += sizeof header.type_traits.file;
      break;
  }
  return header_size == file.write(&header, header_size);
}

}  // namespace ffs

}  // namespace fs
