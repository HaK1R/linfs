#pragma once

#include <cstdint>

#include "fs/error_code.h"
#include "lib/entries/entry.h"
#include "lib/reader_writer.h"
#include "lib/sections/section.h"
#include "lib/sections/section_directory.h"
#include "lib/section_allocator.h"

namespace fs {

namespace linfs {

class DirectoryEntry : public Entry {
 public:
  static std::unique_ptr<DirectoryEntry> Create(uint64_t entry_offset,
                                                uint64_t entry_size,
                                                ReaderWriter* writer,
                                                ErrorCode& error_code,
                                                const char *name);

  DirectoryEntry(uint64_t base_offset) : Entry(Type::kDirectory, base_offset) {}
  ~DirectoryEntry() override = default;

  ErrorCode AddEntry(const Entry* entry, ReaderWriter* reader_writer, SectionAllocator* allocator);
  ErrorCode RemoveEntry(const Entry* entry,
                        ReaderWriter* reader_writer,
                        SectionAllocator* allocator);
  bool HasEntries(ReaderWriter* reader_writer, ErrorCode& error_code);
  std::unique_ptr<Entry> FindEntryByName(const char *entry_name,
                                         ReaderWriter* reader_writer,
                                         ErrorCode& error_code);
  ErrorCode GetNextEntryName(const char *prev, ReaderWriter* reader_writer, char* next_buf);

 private:
  static ErrorCode ClearEntries(uint64_t entries_offset, uint64_t entries_end, ReaderWriter* reader_writer);

  std::unique_ptr<Entry> FindEntryByName(const char *entry_name,
                                         ReaderWriter* reader_writer,
                                         ErrorCode& error_code,
                                         SectionDirectory* section_directory,
                                         SectionDirectory::Iterator *iterator);
};

}  // namespace linfs

}  // namespace fs
