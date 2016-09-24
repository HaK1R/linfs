#include "lib/entry_cache.h"

#include <mutex>

namespace fs {

namespace linfs {

std::shared_ptr<Entry> EntryCache::GetSharedEntry(std::unique_ptr<Entry> entry) {
  std::shared_ptr<Entry> shared_entry = std::move(entry);

  auto it = shared_.emplace(shared_entry->base_offset(), std::weak_ptr<Entry>(shared_entry));
  if (!it.second) {
    std::shared_ptr<Entry> cached_entry = it.first->second.lock();
    if (cached_entry)
      return cached_entry;

    it.first->second = std::weak_ptr<Entry>(shared_entry);
  }

  RemoveExpiredEntries();

  return shared_entry;
}

bool EntryCache::EntryIsShared(Entry* entry) noexcept {
  auto it = shared_.find(entry->base_offset());
  return it != shared_.end() && !it->second.expired();
}

//void EntryCache::ResetSharedEntry(std::shared_ptr<Entry>& entry) {
//  uint64_t entry_offset = entry->base_offset();
//
//  entry.reset();
//
//  std::lock_guard<std::mutex> lock(mutex_);
//  auto it = shared_.find(entry_offset);
//  if (it != shared_.end())
//    if (it->second.expired())
//      shared_.erase(it);
//}

void EntryCache::RemoveExpiredEntries() noexcept {
  auto it = shared_.begin();
  while (it != shared_.end())
    if (it->second.expired())
      it = shared_.erase(it);
    else
      ++it;
}

}  // namespace linfs

}  // namespace fs
