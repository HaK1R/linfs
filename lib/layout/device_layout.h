#pragma once

#include <cstdint>

#include "fs/error_code.h"
#include "fs/filesystem_interface.h"
#include "lib/layout/entry_layout.h"
#include "lib/layout/section_layout.h"
#include "lib/utils/macros.h"
#include "lib/utils/reader_writer.h"

namespace fs {

namespace linfs {

class DeviceLayout {
 public:
  PACK(struct alignas(8) Header {
    Header() = default;
    Header(FilesystemInterface::ClusterSize cluster_size)
        : cluster_size_log2(static_cast<uint8_t>(cluster_size)) {}
    // ---
    char identifier[8] = {'\0', 'f', 'i', 'l', 'e', 'f', 's', '='};  // fs code
    PACK(struct {
      uint8_t major = 1;
      uint8_t minor = 0;
    }) version;                   // version (for backward compatibility)
    uint8_t cluster_size_log2;    // 2^n is actual cluster size
    uint8_t reserved0 = 0;        // reserved for future usage (but actually I'm
                                  // worry about alignment on ARM/SPARC etc.)
    uint16_t none_entry_offset =  // location of none entry in the file
        sizeof(Header) + offsetof(Body, none_entry);
    uint16_t root_entry_offset =  // location of "/" entry
        sizeof(Header) + offsetof(Body, root.entry);
    uint64_t total_clusters = 1;  // total number of allocated clusters
  });
  static_assert(SIZEOF_MEMBER(Header, cluster_size_log2) ==
                    sizeof(FilesystemInterface::ClusterSize),
                "DeviceLayout::Header requires ClusterSize be of size uint8_t");
  STATIC_ASSERT_STANDARD_LAYOUT_AND_TRIVIALLY_COPYABLE(Header);

  // The device's body looks as follows, and is used only to calculate offsets:
  PACK(struct alignas(8) Body {
    Body(const Header& header)
        : root({/*section=*/{(1 << header.cluster_size_log2) -
                                 header.root_entry_offset + sizeof root.section, 0}}) {}
    // ---
    const EntryLayout::NoneHeader none_entry{0};
    PACK(const struct {
      SectionLayout::Header section = {0, 0};
      EntryLayout::DirectoryHeader entry{""};
    }) root;
  });
  STATIC_ASSERT_STANDARD_LAYOUT(Body);

  static Header ParseHeader(ReaderWriter* reader, ErrorCode& error_code);
  static void WriteHeader(Header header, ReaderWriter* writer);
};

}  // namespace linfs

}  // namespace fs
