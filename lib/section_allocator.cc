#include "lib/section_allocator.h"

#include <cstdint>
#include <ios>
#include <iostream>

#include "lib/layout/device_layout.h"

namespace fs {

namespace linfs {

void SectionAllocator::ReleaseSection(const Section& section, ReaderWriter* reader_writer) {
  uint64_t last_cluster_offset = (total_clusters_ - 1) * cluster_size_;
  if (last_cluster_offset == section.base_offset()) {
    total_clusters_ -= section.size() / cluster_size_;
  } else {
    ErrorCode error_code = none_entry_->PutSection(section, reader_writer);
    if (error_code != ErrorCode::kSuccess)
      std::cerr << "Leaked section at " << std::hex << section.base_offset() << " of size " << std::dec << section.size() << std::endl;
  }
}

void SectionAllocator::ReleaseSection(uint64_t section_offset, ReaderWriter* reader_writer) {
  ErrorCode error_code;
  Section section = reader_writer->LoadSection(section_offset, error_code);
  if (error_code != ErrorCode::kSuccess)
    std::cerr << "Leaked section at " << std::hex << section_offset << std::endl;

  ReleaseSection(section, reader_writer);
}

ErrorCode SectionAllocator::SetTotalClusters(uint64_t total_clusters, ReaderWriter* reader_writer) {
  ErrorCode error_code = reader_writer->Write<uint64_t>(total_clusters,
                                                        offsetof(DeviceLayout::Header, total_clusters));
  if (error_code == ErrorCode::kSuccess)
    total_clusters_ = total_clusters;
  return error_code;
}

}  // namespace linfs

}  // namespace fs
