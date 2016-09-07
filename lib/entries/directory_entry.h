#pragma once

#include <cstdint>
#include <memory>

#include "include/IFileSystem.h"
#include "lib/entries/entry.h"

namespace fs {

namespace ffs {

class DirectoryEntry : public Entry {
 public:
  static shared_ptr<DirectoryEntry> Create(const Section& section, ErrorCode& error_code, const char *name);

  DirectoryEntry(uint64_t section_offset)
      : Section(Type::kDirectory, section_offset) {}
  ~DirectoryEntry() override;

  ErrorCode AddEntry(std::shared_ptr<Entry> entry);
  ErrorCode RemoveEntry(std::shared_ptr<Entry> entry);
  std::shared_ptr<Entry> FindEntryByName(const char *entry_name, ErrorCode& error_code);

  // TODO something better
  ErrorCode GetNextEntryName(const char *prev, char* next_buf);
};

}  // namespace ffs

}  // namespace fs
