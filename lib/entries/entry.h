#pragma once

#include <cstdint>
#include <mutex>
#include <memory>

#include "lib/layout/section_layout.h"
#include "lib/sections/section.h"
#include "lib/utils/shared_mutex.h"  // for std::shared_lock, SharedMutex
#include "lib/utils/macros.h"

namespace fs {

namespace linfs {

class ReaderWriter;

class Entry {
 public:
  enum class Type : uint8_t {
    kNone = 0,       // for unused sections
    kDirectory = 1,  // entry's sections represent a directory
    kFile = 2,       // entry's sections represent a file
    kSymlink = 3,    // entry's sections represent a symlink
  };

  // Loads an entry from the device.
  static std::unique_ptr<Entry> Load(uint64_t entry_offset,
                                     ReaderWriter* reader,
                                     char* name_buf = nullptr);

  // Creates an entry on the device.  See *Entry classes.
  // static std::unique_ptr<Entry> Create(...);

  virtual ~Entry() = default;

  Type type() const { return type_; }
  uint64_t base_offset() const { return base_offset_; }
  uint64_t section_offset() const {
    return base_offset() - sizeof(SectionLayout::Header);
  }

  // Cast to derived class.
  template <typename T>
  T* As() {
    STATIC_ASSERT_BASE_OF(Entry, T);
    return static_cast<T*>(this);
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

  Section CursorToSection(uint64_t& cursor, ReaderWriter* reader,
                          uint64_t start_position, bool check_cursor = true);

  // Thread safety is guaranteed by this guy.
  SharedMutex mutex_;

 private:
  const Type type_;
  const uint64_t base_offset_;
};

}  // namespace linfs

}  // namespace fs
