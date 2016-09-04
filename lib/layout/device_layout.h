#pragma once

#include <cstdint>
#include <fstream>

namespace fs {

namespace ffs {

class DeviceLayout {
 public:
  struct __attribute__((packed, aligned(4))) Header {
    char identifier[8] = {'\0', 'f', 'i', 'l', 'e', 'f', 's', '='};  // fs code
    struct __attribute__((packed)) {
      uint8_t major = 0;
      uint8_t minor = 1;
    } version;                   // version (for backward compatibility)
    uint8_t cluster_size_log2;   // 2^n is actual cluster size
    uint8_t reserved0 = 0;       // reserved for future usage (but actually I'm
                                 // worry about alignment on ARM/SPARC etc.)
    uint16_t root_entry_offset;  // location of "/" section
  };

  static bool ParseHeader(std::ifstream& file, uint64_t& cluster_size,
                          uint16_t& root_section_offset);
  static bool WriteHeader(std::ofstream& file, const Header& header);
};

}  // namespace ffs

}  // namespace fs
