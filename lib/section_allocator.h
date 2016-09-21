#pragma once

#include <cstdint>
#include <memory>

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
  template<typename T = Section>
  T AllocateSection(uint64_t size, ReaderWriter* reader_writer, ErrorCode& error_code) {
    if (none_entry_->HasSections()) {
      // FIXME compile
      Section section = none_entry_->GetSection(size, reader_writer, error_code);
      return *static_cast<T*>(&section);
    }

    // There is nothing in NoneEntry chain. Allocate a new cluster.
    uint64_t required_clusters = (size + cluster_size_ - 1) / cluster_size_;
    T section(total_clusters_ * cluster_size_, required_clusters * cluster_size_, 0);
    error_code = reader_writer->SaveSection(section);
    if (error_code == ErrorCode::kSuccess) {
      error_code = reader_writer->Write<uint8_t>(0, section.base_offset() + section.size() - 1);
      if (error_code == ErrorCode::kSuccess)
        total_clusters_ += required_clusters;
    }
    return section;
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
