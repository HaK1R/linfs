#pragma once

#include <cstdint>

#include "lib/layout/section_layout.h"

namespace fs {

namespace linfs {

class Entry {
 public:
  enum class Type : uint8_t {
    kNone = 0,       // for unused sections
    kDirectory = 1,  // entry's sections represent a directory
    kFile = 2        // entry's sections represent a file
  };

  virtual ~Entry() = default;

  // TODO? GetAsyncReader();

  Type type() const { return type_; }
  uint64_t base_offset() const { return base_offset_; }
  uint64_t section_offset() const { return base_offset() - sizeof(SectionLayout::Header); }

 protected:
  Entry(Type type, uint64_t base_offset)
      : type_(type), base_offset_(base_offset) {}

  // Thread safety is guaranteed by this guy.
  //std::mutex mutex_;

 private:
  const Type type_;
  const uint64_t base_offset_;
};

}  // namespace linfs

}  // namespace fs
