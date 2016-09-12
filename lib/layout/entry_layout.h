#pragma once

#include <cstdint>
#include <fstream>

#include "include/interfaces/IFileSystem.h"

namespace fs {

namespace ffs {

class EntryLayout {
  static_assert(std::is_trivially_copyable<Header>::value,
                "EntryLayout::Header isn't trivially copyable");
  static_assert(std::is_trivially_copyable<NoneHeader>::value,
                "EntryLayout::NoneHeader isn't trivially copyable");
  static_assert(std::is_trivially_copyable<DirectoryHeader>::value,
                "EntryLayout::DirectoryHeader isn't trivially copyable");
  static_assert(std::is_trivially_copyable<FileHeader>::value,
                "EntryLayout::FileHeader isn't trivially copyable");
 public:
  // TODO make EntryLayout::template
  struct __attribute__((packed)) Header {
    uint8_t type;                // type of this section
    uint8_t reserved0[7] = {0};  // say hello ARM64
    // TODO packed union?
    union {
      struct __attribute__((packed)) {
        uint64_t head_offset;    // points to the head of unused sections list
      } none;                    // if type == kNone
      struct __attribute__((packed)) {
        char name[kNameMax];     // directory name
      } directory;               // if type == kDirectory
      struct __attribute__((packed)) {
        char name[kNameMax];     // file name
      } file;                    // if type == kFile
    // TODO rename to u
    } type_traits;
  };

  // TODO remove this
  struct __attribute__((packed)) _Header {
    uint8_t type;                // type of this section
    uint8_t reserved0[7] = {0};  // say hello ARM64
  };

  struct __attribute__((packed)) NoneHeader : _Header {
    uint64_t head_offset = 0;    // points to the head of unused sections list
  };

  struct __attribute__((packed)) DirectoryHeader : _Header {
    char name[kNameMax];         // directory name
  };

  struct __attribute__((packed)) FileHeader : _Header {
    uint64_t size;               // file size
    char name[kNameMax];         // file name
  };

  union HeaderUnion {
    _Header common;
    NoneHeader none;
    DirectoryHeader directory;
    FileHeader file;
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
};

}  // namespace ffs

}  // namespace fs
