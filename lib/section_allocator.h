#pragma once

#include <cstdint>
#include <ios>

#include "lib/section.h"

namespace fs {

namespace ffs {

class SectionAllocator {
 public:
  SectionAllocator() = default;
  SectionAllocator(uint64_t cluster_size, uint64_t total_clusters, std::shared_ptr<NoneEntry> none_entry)
      : cluster_size_(cluster_size), total_clusters_(total_clusters), none_entry_(std::move(none_entry)) {}
  SectionAllocator(const SectionAllocator&) = delete;
  SectionAllocator(SectionAllocator&&) = default;
  ~SectionAllocator() = default;

  template<typename T = Section>
  T AllocateSection(uint64_t size, ReaderWriter* reader_writer, ErrorCode& error_code) {
    if (none_entry_->HasSections()) {
      return none_entry_->GetSection(size, reader_writer, error_code);
    }

    // There is nothing in NoneEntry chain. Allocate a new cluster.
    uint64_t required_clusters = (size + cluster_size_ - 1) / cluster_size_;
    T section(total_clusters_ * cluster_size_, required_clusters * cluster_size_, 0);
    error_code = reader_writer->SaveSection(section);
    if (error_code == ErrorCode::kSuccess) {
      error_code = reader_writer->Write<uint8_t>(0, section.base_offset() - 1);
      if (error_code == ErrorCode::kSuccess)
        total_clusters_ += required_clusters;
    }
    return section;
  }

  void ReleaseSection(const Section& section, ReaderWriter* reader_writer, ErrorCode& error_code) {
    uint64_t last_cluster_offset = (total_clusters_ - 1) * cluster_size_;
    if (last_cluster_offset == section.base_offset()) {
      error_code = ErrorCode::kSuccess;
      --total_clusters_;
    } else {
      error_code = none_entry->PutSection(section, reader_writer);
    }
  }

  void ReleaseSection(const Section& section, ReaderWriter* reader_writer) {
    ErrorCode error_code;
    ReleaseSection(section, reader_writer, error_code);
    if (error_code != ErrorCode::kSuccess)
      std::cerr << "Leaked section at " << std::hex << section.base_offset() << " of size " << std::dec << section.size();
  }

 private:
  const uint64_t cluster_size_ = 0;
  uint64_t total_clusters_ = 0;
  std::shared_ptr<NoneEntry> none_entry_;
};

}  // namespace ffs

}  // namespace fs
