#pragma once

#include <cstdint>
#include <cstring>
#include <type_traits>

#include "fs/limits.h"
#include "lib/entries/entry.h"
#include "lib/utils/macros.h"

namespace fs {

namespace linfs {

class EntryLayout {
  // Common header
  PACK(struct _Header {
    _Header(Entry::Type _type) : type(static_cast<uint8_t>(_type)) {}
    // ---
    uint8_t type;                // type of this section
    uint8_t reserved0[7] = {0};  // say hello ARM64
  });
  static_assert(SIZEOF_MEMBER(_Header, type) == sizeof(Entry::Type),
                "EntryLayout::_Header requires Entry::Type be of size uint8_t");

 public:
  PACK(struct alignas(8) NoneHeader {
    NoneHeader(uint64_t _head_offset) : head_offset(_head_offset) {}
    // ---
    _Header common{Entry::Type::kNone};
    uint64_t head_offset;        // points to the head of unused sections list
  });
  STATIC_ASSERT_STANDARD_LAYOUT_AND_TRIVIALLY_COPYABLE(NoneHeader);

  PACK(struct DirectoryHeader {
    DirectoryHeader(const char* _name) {
      strncpy(name, _name, sizeof name);
    }
    // ---
    _Header common{Entry::Type::kDirectory};
    char name[kNameMax];         // directory name
  });
  STATIC_ASSERT_STANDARD_LAYOUT_AND_TRIVIALLY_COPYABLE(DirectoryHeader);

  PACK(struct alignas(8) FileHeader {
    FileHeader(uint64_t _size, const char* _name) : size(_size) {
      strncpy(name, _name, sizeof name);
    }
    // ---
    _Header common{Entry::Type::kFile};
    uint64_t size;               // file size
    char name[kNameMax];         // file name
  });
  STATIC_ASSERT_STANDARD_LAYOUT_AND_TRIVIALLY_COPYABLE(FileHeader);

  PACK(struct SymlinkHeader {
    SymlinkHeader(const char* _name) {
      strncpy(name, _name, sizeof name);
    }
    // ---
    _Header common{Entry::Type::kSymlink};
    char name[kNameMax];         // symlink name
  });
  STATIC_ASSERT_STANDARD_LAYOUT_AND_TRIVIALLY_COPYABLE(SymlinkHeader);

  using HeaderStorage = std::aligned_union_t<0, NoneHeader, DirectoryHeader,
                                             FileHeader, SymlinkHeader>;
  PACK(union alignas(HeaderStorage) HeaderUnion {
    NoneHeader none;
    DirectoryHeader directory;
    FileHeader file;
    SymlinkHeader symlink;
  });
  STATIC_ASSERT_TRIVIALLY_COPYABLE(HeaderUnion);

  // The directory's body looks like:
  // struct BodyDirectory {
  //   uint64_t entries_offsets[];
  // };
  //
  // file's body is:
  // struct BodyFile {
  //   uint8_t data[];
  // };
  //
  // and symlink's body is:
  // struct BodySymlink {
  //   uint8_t target[];
  // };
};

}  // namespace linfs

}  // namespace fs
