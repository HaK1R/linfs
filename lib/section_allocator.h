#pragma once

#include <cstdint>
#include <memory>
#include <type_traits>

#include "fs/error_code.h"
#include "lib/entries/none_entry.h"
#include "lib/reader_writer.h"
#include "lib/sections/section.h"

namespace fs {

namespace linfs {

class SectionAllocator {
 public:
  SectionAllocator() = delete;
  SectionAllocator(uint64_t cluster_size, uint64_t total_clusters, std::unique_ptr<NoneEntry> none_entry)
      : cluster_size_(cluster_size), total_clusters_(total_clusters), none_entry_(std::move(none_entry)) {}

  // Allocates section of the preferred |size|.
  // Note that the size of the allocated section may be less than |size|.
  Section AllocateSection(uint64_t size, ReaderWriter* reader_writer, ErrorCode& error_code);
  template<typename T>
  T AllocateSection(uint64_t size, ReaderWriter* reader_writer, ErrorCode& error_code) {
      static_assert(std::is_base_of<Section, T>::value,
                    "T must be derived from Section");
      Section section = AllocateSection(size, reader_writer, error_code);
      return T(section.base_offset(), section.size(), section.next_offset());
  }

  void ReleaseSection(const Section& section, ReaderWriter* reader_writer);
  void ReleaseSection(uint64_t section_offset, ReaderWriter* reader_writer);

 private:
  ErrorCode SetTotalClusters(uint64_t total_clusters, ReaderWriter* reader_writer);

  const uint64_t cluster_size_;
  uint64_t total_clusters_;
  std::unique_ptr<NoneEntry> none_entry_;
};

}  // namespace linfs

}  // namespace fs
