#pragma once

#include <cstdint>
#include <map>
#include <memory>

#include "fs/error_code.h"
#include "lib/entries/entry.h"

namespace fs {

namespace linfs {

class EntryCache {
 public:
  std::shared_ptr<Entry> GetSharedEntry(std::unique_ptr<Entry> entry);
  bool EntryIsShared(Entry* entry) noexcept;

 private:
  void RemoveExpiredEntries() noexcept;

  std::map<uint64_t, std::weak_ptr<Entry>> shared_;
};

}  // namespace linfs

}  // namespace fs
