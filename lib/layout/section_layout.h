#pragma once

#include <cstdint>
#include <fstream>

#include "include/interfaces/IFileSystem.h"

namespace fs {

namespace ffs {

class SectionLayout {
 public:
  // TODO move to Section class
  enum SectionType : uint8_t {
    kNone = 0,       // unused section
    kDirectory = 1,  // section represents a directory
    kFile = 2        // section represents a file
  };

  struct __attribute__((packed, aligned(8))) Header {
    uint8_t type;              // type of this section
    uint8_t reserved0[7];      // say hello ARM64
    // TODO packed union?
    union {
      struct __attribute__((packed)) {
        uint64_t size;         // size of this unused section
        uint64_t next_offset;  // offset of the next unused section
      } none;                  // if type == kNone
      struct __attribute__((packed)) {
        uint64_t available;    // number of available bytes at the end of
                               // this section
        uint64_t next_offset;  // continuation of the current section
        char name[kNameMax];   // optional: directory name
      } directory;             // if type == kDirectory
      struct __attribute__((packed)) {
        uint64_t size;
        uint64_t next_offset;  // continuation of the current section
        char name[kNameMax];   // optional: file name
      } file;                  // if type == kFile
    } type_traits;
  };

  struct BodyDirectory {
    uint64_t entries_offests[];
  };

  struct BodyFile {
    uint8_t data[];
  };

  static bool WriteHeader(std::ofstream& file, const Header& header);
};

}  // namespace ffs

}  // namespace fs
