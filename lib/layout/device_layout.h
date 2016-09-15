#pragma once

#include <cstdint>

namespace fs {

namespace ffs {

class DeviceLayout {
  static_assert(std::is_trivially_copyable<Header>::value,
                "DeviceLayout::Header isn't trivially copyable");
  static_assert(std::is_standard_layout<Body>::value,
                "DeviceLayout::Body must be a standard-layout type");
 public:
  struct __attribute__((packed, aligned(8))) Header {
    char identifier[8] = {'\0', 'f', 'i', 'l', 'e', 'f', 's', '='};  // fs code
    struct __attribute__((packed)) {
      uint8_t major = 0;
      uint8_t minor = 1;
    } version;                    // version (for backward compatibility)
    uint8_t cluster_size_log2;    // 2^n is actual cluster size
    uint8_t reserved0 = {0};      // reserved for future usage (but actually I'm
                                  // worry about alignment on ARM/SPARC etc.)
    uint16_t none_entry_offset =  // location of none entry in the file
        sizeof(Header) + offsetof(Body, none_entry);
    uint16_t root_entry_offset =  // location of "/" entry
        sizeof(Header) + offsetof(Body, root.entry);
    uint64_t total_clusters = 1;  // total number of allocated clusters
  };

  // The device's body looks as follows, and used only to calculate offsets:
  struct __attribute__((packed)) Body {
    EntryLayout::NoneHeader none_entry;
    struct __attribute__((packed)) {
      SectionLayout::Header section;
      EntryLayout::DirectoryHeader entry;
    } root;
  };

  static ErrorCode ParseHeader(ReaderWriter* reader, Header& header);
};

// TODO? ReaderWriter& operator<<(ReaderWriter& writer, DeviceLayout::Header header);

}  // namespace ffs

}  // namespace fs
