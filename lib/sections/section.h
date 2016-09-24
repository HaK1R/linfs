#pragma once

#include <cstddef>
#include <cstdint>

#include "fs/error_code.h"
#include "lib/layout/section_layout.h"

namespace fs {

namespace linfs {

class ReaderWriter;

class Section {
 public:
  Section() = delete;
  Section(uint64_t base_offset, uint64_t size, uint64_t next_offset)
      : base_offset_(base_offset), size_(size), next_offset_(next_offset) {}

  uint64_t base_offset() const { return base_offset_; }
  uint64_t size() const { return size_; }
  uint64_t next_offset() const { return next_offset_; }

  uint64_t data_offset() const { return base_offset() + sizeof(SectionLayout::Header); }
  uint64_t data_size() const { return size() - sizeof(SectionLayout::Header); }

 protected:
  // TODO? use methods instead of entire classes
  friend class DirectoryEntry;
  friend class FileEntry;
  friend class NoneEntry;
  void SetSize(uint64_t size, ReaderWriter* reader_writer);
  void SetNext(uint64_t next_offset, ReaderWriter* reader_writer);

 private:
  /*TODO? const*/uint64_t base_offset_;
  uint64_t size_;
  uint64_t next_offset_;
};

}  // namespace linfs

}  // namespace fs
