#pragma once

#include <cstdint>
#include <fstream>
#include <type_traits>

#include "fs/IFileSystem.h"
#include "lib/entries/entry.h"

namespace fs {

namespace linfs {

class EntryLayout {
  // TODO?class CHECK_LAYOUT_TYPE
  struct __attribute__((packed)) _Header {
    _Header(Entry::Type _type) : type(static_cast<uint8_t>(_type)) {}
    // ---
    uint8_t type;                // type of this section
    uint8_t reserved0[7] = {0};  // say hello ARM64
  };
  static_assert(sizeof(_Header::type) == sizeof(Entry::Type),
                "EntryLayout::_Header requires Entry::Type be of size uint8_t");

 public:
  struct __attribute__((packed)) NoneHeader {
    NoneHeader(uint64_t _head_offset) : head_offset(_head_offset) {}
    // ---
    _Header common{Entry::Type::kNone};
    uint64_t head_offset;        // points to the head of unused sections list
  };
  static_assert(std::is_trivially_copyable<NoneHeader>::value,
                "EntryLayout::NoneHeader isn't a trivially copyable type");
  static_assert(std::is_standard_layout<NoneHeader>::value,
                "EntryLayout::NoneHeader isn't a standard-layout type");

  struct __attribute__((packed)) DirectoryHeader {
    DirectoryHeader(const char* _name) {
      strncpy(name, _name, sizeof name);
    }
    // ---
    _Header common{Entry::Type::kDirectory};
    char name[kNameMax];         // directory name
  };
  static_assert(std::is_trivially_copyable<DirectoryHeader>::value,
                "EntryLayout::DirectoryHeader isn't a trivially copyable type");
  static_assert(std::is_standard_layout<DirectoryHeader>::value,
                "EntryLayout::DirectoryHeader isn't a standard-layout type");

  struct __attribute__((packed)) FileHeader {
    FileHeader(uint64_t _size, const char* _name) : size(_size) {
      strncpy(name, _name, sizeof name);
    }
    // ---
    _Header common{Entry::Type::kFile};
    uint64_t size;               // file size
    char name[kNameMax];         // file name
  };
  static_assert(std::is_trivially_copyable<FileHeader>::value,
                "EntryLayout::FileHeader isn't a trivially copyable type");
  static_assert(std::is_standard_layout<FileHeader>::value,
                "EntryLayout::FileHeader isn't a standard-layout type");

  union __attribute__((packed)) HeaderUnion {
    // Make compiler happy with default constructor
    HeaderUnion() {}
    // ---
    _Header common;
    NoneHeader none;
    DirectoryHeader directory;
    FileHeader file;
  };
  static_assert(std::is_trivially_copyable<FileHeader>::value,
                "EntryLayout::HeaderUnion isn't a trivially copyable type");

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

}  // namespace linfs

}  // namespace fs
