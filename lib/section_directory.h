#pragma once

#include <cstddef>
#include <cstdint>

#include <iterator>

namespace fs {

namespace ffs {

class SectionDirectory : Section {
 public:
  // TODO class iterator? and use offsets?
  typedef const uint64_t* Iterator;

  SectionDirectory(uint64_t base_offset, uint64_t size, uint64_t next_offset);
  ~SectionDirectory() = default;

  Iterator begin() const { return entries_offsets_.get(); }
  Iterator end() const { return begin() + entries_offsets_size_; }

  int AddEntry(uint64_t entry_offset);
  int RemoveEntry(const Iterator& position);

  constexpr size_t header_size() const { return sizeof(SectionLayout::Header<Entry::Type::kDirectory>); }
  uint64_t entries_offsets_offset() const { return base_offset() + header_size(); }
  uint64_t entries_offsets_size() const { return size() - header_size(); }

 private:
  uint64_t entries_offsets_capacity_;
  uint64_t entries_offsets_size_;
};

}  // namespace ffs

}  // namespace fs
