#pragma once

#include <cstdint>
#include <type_traits>

#include "fs/error_code.h"
#include "fs/IFileSystem.h"
#include "lib/layout/entry_layout.h"
#include "lib/layout/section_layout.h"
#include "lib/reader_writer.h"

namespace fs {

namespace linfs {

class DeviceLayout {
 public:
  struct __attribute__((packed, aligned(8))) Header {
    Header() = default; // TODO? remove
    Header(IFileSystem::ClusterSize cluster_size) : cluster_size_log2(static_cast<uint8_t>(cluster_size)) {}
    // ---
    char identifier[8] = {'\0', 'f', 'i', 'l', 'e', 'f', 's', '='};  // fs code
    struct __attribute__((packed)) {
      uint8_t major = 0;
      uint8_t minor = 1;
    } version;                    // version (for backward compatibility)
    uint8_t cluster_size_log2;    // 2^n is actual cluster size
    uint8_t reserved0 = 0;        // reserved for future usage (but actually I'm
                                  // worry about alignment on ARM/SPARC etc.)
    uint16_t none_entry_offset =  // location of none entry in the file
        sizeof(Header) + offsetof(Body, none_entry);
    uint16_t root_entry_offset =  // location of "/" entry
        sizeof(Header) + offsetof(Body, root.entry);
    uint64_t total_clusters = 1;  // total number of allocated clusters
  };
  // TODO? update assert message; swap its arguments
  static_assert(sizeof(Header::cluster_size_log2) == sizeof(IFileSystem::ClusterSize),
                "DeviceLayout::Header requires ClusterSize be of size uint8_t");
  static_assert(std::is_trivially_copyable<Header>::value,
                "DeviceLayout::Header isn't trivially copyable");

  // The device's body looks as follows, and used only to calculate offsets:
  struct __attribute__((packed)) Body {
    Body(const Header& header)
      : root({{(1 << header.cluster_size_log2) - header.root_entry_offset + sizeof root.section, 0}}) {}
    // ---
    const EntryLayout::NoneHeader none_entry{0};
    const struct __attribute__((packed)) {
      SectionLayout::Header section = {0, 0};
      EntryLayout::DirectoryHeader entry{""};
    } root;
  };
  static_assert(std::is_standard_layout<Body>::value,
                "DeviceLayout::Body must be a standard-layout type");

  static Header ParseHeader(ReaderWriter* reader, ErrorCode& error_code);
  static void WriteHeader(Header header, ReaderWriter* writer);
};

// TODO? ReaderWriter& operator<<(ReaderWriter& writer, DeviceLayout::Header header);

}  // namespace linfs

}  // namespace fs
