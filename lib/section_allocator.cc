#include "lib/section_allocator.h"

#include <cstdint>
#include <ios>
#include <iostream>

#include "lib/layout/device_layout.h"

namespace fs {

namespace linfs {

Section SectionAllocator::AllocateSection(uint64_t size, ReaderWriter* reader_writer) {
  // SectionAllocator owns and entirely depends on the NoneEntry and just
  // expands its functionality. Therefore we can use only one mutex for both of
  // them.
  std::unique_lock<std::mutex> lock = none_entry_->Lock();

  if (none_entry_->HasSections())
    return none_entry_->GetSection(size, reader_writer);

  // There is nothing in NoneEntry chain. Allocate a new cluster.
  uint64_t required_clusters = (size + cluster_size_ - 1) / cluster_size_;
  Section section(total_clusters_ * cluster_size_, required_clusters * cluster_size_, 0);
  reader_writer->SaveSection(section);
  reader_writer->Write<uint8_t>(0, section.base_offset() + section.size() - 1);
  SetTotalClusters(total_clusters_ + required_clusters, reader_writer);
  return section;
}

void SectionAllocator::ReleaseSection(const Section& section,
                                      ReaderWriter* reader_writer) noexcept {
  std::unique_lock<std::mutex> lock = none_entry_->Lock();

  uint64_t last_cluster_offset = (total_clusters_ - 1) * cluster_size_;
  try {
    if (last_cluster_offset == section.base_offset())
      SetTotalClusters(total_clusters_ - section.size() / cluster_size_, reader_writer);
    else
      none_entry_->PutSection(section, reader_writer);
  }
  catch (...) {
    // TODO? don't use std::cerr in shared libraries
    std::cerr << "Leaked section at " << std::hex << section.base_offset()
              << " of size " << std::dec << section.size() << std::endl;
  }
}

void SectionAllocator::ReleaseSection(uint64_t section_offset,
                                      ReaderWriter* reader_writer) noexcept {
  try {
    Section section = reader_writer->LoadSection(section_offset);
    ReleaseSection(section, reader_writer);
  }
  catch (...) {
    std::cerr << "Leaked section at " << std::hex << section_offset << std::endl;
  }
}

void SectionAllocator::SetTotalClusters(uint64_t total_clusters, ReaderWriter* reader_writer) {
  reader_writer->Write<uint64_t>(total_clusters, offsetof(DeviceLayout::Header, total_clusters));
  total_clusters_ = total_clusters;
}

}  // namespace linfs

}  // namespace fs
