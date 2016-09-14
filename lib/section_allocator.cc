#include "lib/section_allocator.h"

#include <cstdint>
#include <ios>
#include <iostream>

namespace fs {

namespace linfs {

void SectionAllocator::ReleaseSection(const Section& section, ReaderWriter* reader_writer, ErrorCode& error_code) {
  uint64_t last_cluster_offset = (total_clusters_ - 1) * cluster_size_;
  if (last_cluster_offset == section.base_offset()) {
    error_code = ErrorCode::kSuccess;
    --total_clusters_;
  } else {
    error_code = none_entry_->PutSection(section, reader_writer);
  }
}

void SectionAllocator::ReleaseSection(const Section& section, ReaderWriter* reader_writer) {
  ErrorCode error_code;
  ReleaseSection(section, reader_writer, error_code);
  if (error_code != ErrorCode::kSuccess)
    std::cerr << "Leaked section at " << std::hex << section.base_offset() << " of size " << std::dec << section.size();
}

}  // namespace linfs

}  // namespace fs
