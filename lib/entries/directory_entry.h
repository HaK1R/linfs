#pragma once

#include <cstdint>
#include <memory>

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
                                                ReaderWriter* writer,
                                                ErrorCode& error_code,
                                                const char *name);

  DirectoryEntry(uint64_t base_offset) : Entry(Type::kDirectory, base_offset) {}
  ~DirectoryEntry() override = default;

  // TODO don't use shared_ptr
  ErrorCode AddEntry(std::shared_ptr<Entry> entry, ReaderWriter* reader_writer, SectionAllocator* allocator);
  ErrorCode RemoveEntry(std::shared_ptr<Entry> entry,
                        ReaderWriter* reader_writer,
                        SectionAllocator* allocator);
  bool HasEntries(ReaderWriter* reader_writer, ErrorCode& error_code);
  std::unique_ptr<Entry> FindEntryByName(const char *entry_name,
                                         ReaderWriter* reader_writer,
                                         ErrorCode& error_code,
                                         SectionDirectory* section_directory = nullptr,
                                         SectionDirectory::Iterator *iterator = nullptr);
  ErrorCode GetNextEntryName(const char *prev, ReaderWriter* reader_writer, char* next_buf);
};

}  // namespace linfs

}  // namespace fs
