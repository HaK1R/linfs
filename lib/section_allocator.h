#pragma once

#include <cstdint>
#include <memory>

#include "lib/entries/none_entry.h"
#include "lib/sections/section.h"
#include "lib/utils/reader_writer.h"

namespace fs {

namespace linfs {

class SectionAllocator {
 public:
  SectionAllocator(uint64_t cluster_size, uint64_t total_clusters,
                   std::unique_ptr<NoneEntry> none_entry)
      : cluster_size_(cluster_size), total_clusters_(total_clusters),
        none_entry_(std::move(none_entry)) {}

  // Allocates section of the preferred |size|.
  // Note that the size of the allocated section may be less than |size|.
  Section AllocateSection(uint64_t size, ReaderWriter* reader_writer);

  void ReleaseSection(const Section& section, ReaderWriter* reader_writer) noexcept;
  void ReleaseSection(uint64_t section_offset, ReaderWriter* reader_writer) noexcept;

 private:
  void SetTotalClusters(uint64_t total_clusters, ReaderWriter* reader_writer);

  const uint64_t cluster_size_;
  uint64_t total_clusters_;
  std::unique_ptr<NoneEntry> none_entry_;
};

}  // namespace linfs

}  // namespace fs
