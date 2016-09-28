#pragma once

#include <cstdint>
#include <map>
#include <memory>

#include "lib/entries/entry.h"
#include "lib/utils/shared_mutex.h"

namespace fs {

namespace linfs {

class EntryCache {
 public:
  std::shared_ptr<Entry> GetSharedEntry(std::unique_ptr<Entry> entry);
  bool EntryIsShared(const Entry* entry) const noexcept;

 private:
  void RemoveExpiredEntries() noexcept;

  std::map<uint64_t, std::weak_ptr<Entry>> shared_;
  mutable SharedMutex mutex_;
};

}  // namespace linfs

}  // namespace fs
