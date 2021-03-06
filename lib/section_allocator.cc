#include "lib/section_allocator.h"

#include <ios>

#include "lib/layout/device_layout.h"

#ifndef NDEBUG
#include <iostream>
#endif

namespace fs {

namespace linfs {

Section SectionAllocator::AllocateSection(uint64_t size, ReaderWriter* reader_writer) {
  // SectionAllocator owns and entirely depends on the NoneEntry and just
  // expands its functionality.  Therefore we can use only one mutex for both of
  // them.
  std::unique_lock<SharedMutex> lock = none_entry_->Lock();

  // Round up to the |cluster_size_| boundary.  It's correct because
  // |cluster_size_| is always equal to power of 2.
  size = (size + cluster_size_ - 1) & ~(cluster_size_ - 1);

  if (none_entry_->HasSections())
    return none_entry_->GetSection(size, reader_writer);

  // There is nothing in NoneEntry chain.  Allocate a new cluster.
  uint64_t required_clusters = size / cluster_size_;
  Section section = Section::Create(total_clusters_ * cluster_size_,
                                    required_clusters * cluster_size_, reader_writer);
  reader_writer->Write<uint8_t>(0, section.base_offset() + section.size() - 1);
  SetTotalClusters(total_clusters_ + required_clusters, reader_writer);
  return section;
}

void SectionAllocator::ReleaseSection(const Section& section,
                                      ReaderWriter* reader_writer) noexcept {
  std::unique_lock<SharedMutex> lock = none_entry_->Lock();

  try {
    // Actually in some cases we can decrease |total_clusters_| instead of putting
    // the released sections to NoneEntry, but for now it doesn't matter.
    none_entry_->PutSection(section, reader_writer);
  }
  catch (...) {
#ifndef NDEBUG
    std::cerr << "Leaked section(s) at " << std::hex << section.base_offset()
              << " of size " << std::dec << section.size() << std::endl;
#endif
  }
}

void SectionAllocator::ReleaseSection(uint64_t section_offset,
                                      ReaderWriter* reader_writer) noexcept {
  try {
    Section section = Section::Load(section_offset, reader_writer);
    ReleaseSection(section, reader_writer);
  }
  catch (...) {
#ifndef NDEBUG
    std::cerr << "Leaked section(s) at " << std::hex << section_offset << std::endl;
#endif
  }
}

void SectionAllocator::SetTotalClusters(uint64_t total_clusters, ReaderWriter* reader_writer) {
  reader_writer->Write<uint64_t>(total_clusters, offsetof(DeviceLayout::Header, total_clusters));
  total_clusters_ = total_clusters;
}

}  // namespace linfs

}  // namespace fs
