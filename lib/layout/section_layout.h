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
    union {
      /* struct { } none; */   // if Entry::type == kNone
      struct __attribute__((packed)) {
        uint64_t available;    // number of available bytes at the end of
                               // this section
      } directory;             // if Entry::type == kDirectory
      /* struct { } file; */   // if Entry::type == kFile
    // TODO rename to u
    } type_traits;
  };

  // The directory's body looks like:
  // struct BodyDirectory {
  //   uint64_t entries_offsets[];
  // };
  //
  // and file's body is:
  // struct BodyFile {
  //   uint8_t data[];
  // };

  static bool WriteHeader(std::ofstream& file, const Header& header);
};

}  // namespace ffs

}  // namespace fs
