#pragma once

#include <cstdint>

#include "include/interfaces/IFileSystem.h"

namespace fs {

namespace ffs {

const char kFileFSIdentifier[] = "\0filefs=";
constexpr int kMajorVersion = 0;
constexpr int kMinorVersion = 1;

struct __attribute__((packed, aligned(4))) DeviceHeader {
  char identifier[sizeof(kFileFSIdentifier) - 1];  // "magic" string
  struct __attribute__((packed)) {
    uint8_t major;
    uint8_t minor;
  } version;                     // version (for backward compatibility)
  uint8_t log2_of_cluster_size;  // 2^n is actual cluster size
  uint8_t reserved0;             // reserved for future usage (but actually I'm
                                 // worry about alignment on ARM/SPARC etc.)
  uint16_t root_section_offset;  // location of "/" section
};

}  // namespace ffs

}  // namespace fs
