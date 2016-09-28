#pragma once

#include <cstdint>
#include <mutex>

#include "lib/layout/section_layout.h"
#include "lib/utils/shared_mutex.h"  // for std::shared_lock, SharedMutex

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

  Type type() const { return type_; }
  uint64_t base_offset() const { return base_offset_; }
  uint64_t section_offset() const {
    return base_offset() - sizeof(SectionLayout::Header);
  }

  // Acquires exclusive ownership for read-write access.
  std::unique_lock<SharedMutex> Lock() {
    return std::unique_lock<SharedMutex>(mutex_);
  }

  // Acquires shared ownership for read-only access.
  std::shared_lock<SharedMutex> LockShared() {
    return std::shared_lock<SharedMutex>(mutex_);
  }

 protected:
  Entry(Type type, uint64_t base_offset)
      : type_(type), base_offset_(base_offset) {}

  // Thread safety is guaranteed by this guy.
  SharedMutex mutex_;

 private:
  const Type type_;
  const uint64_t base_offset_;
};

}  // namespace linfs

}  // namespace fs
