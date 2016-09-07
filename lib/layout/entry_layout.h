#pragma once

#include <cstdint>
#include <fstream>

#include "include/interfaces/IFileSystem.h"

namespace fs {

namespace ffs {

class EntryLayout {
 public:
  struct __attribute__((packed)) Header {
    uint8_t type;                // type of this section
    uint8_t reserved0[7] = {0};  // say hello ARM64
    // TODO packed union?
    union {
      /* struct { } none; */     // if type == kNone
      struct __attribute__((packed)) {
        char name[kNameMax];     // directory name
      } directory;               // if type == kDirectory
      struct __attribute__((packed)) {
        char name[kNameMax];     // file name
      } file;                    // if type == kFile
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
