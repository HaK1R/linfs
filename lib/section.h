#pragma once

#include <cstdint>

namespace fs {

namespace ffs {

class Section {
 public:
  enum class Type : uint8_t {
    kNone = 0,       // unused section
    kDirectory = 1,  // section represents a directory
    kFile = 2        // section represents a file
  };

  Section(Type type, uint64_t base_offset)
      : type_(type), base_offset_(base_offset) {}
  virtual ~Section() = 0;

  Type type() const { return type_; }
  uint64_t base_offset() const { return base_offset_; }

  template<typename T>
  T* As<T>() { return static_cast<T*>(this); }

 protected:
  Type type_;
  uint64_t base_offset_;
};

}  // namespace ffs

}  // namespace fs
