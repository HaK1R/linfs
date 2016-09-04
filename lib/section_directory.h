#pragma once

#include <cstddef>
#include <cstdint>

namespace fs {

namespace ffs {

class SectionDirectory : Section {
 public:
  ~SectionDirectory() = default;

  class Iterator {
   public:
    Iterator(uint64_t val) : val_(val) {}
    Iterator operator++() { Iterator ret(val); val_++; return ret; }
    Iterator& operator++(int) { val_++; return *this; }

   private:
    uint64_t val_;
  };

  Iterator Begin();

  int AddEntry(uint64_t entry_offset);
  int RemoveEntry(uint64_t entry_offset);
  uint64_t FindEntryByName(const char *entry_name);

  constexpr size_t header_size() const { return sizeof(SectionLayout::Header<Entry::Type::kDirectory>); }
  uint64_t entries_offsets_offset() const { return base_offset() + header_size(); }
  uint64_t entries_offsets_size() const { return size() - header_size(); }

 private:
  uint64_t available_;
};

}  // namespace ffs

}  // namespace fs
