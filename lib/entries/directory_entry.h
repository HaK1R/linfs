#pragma once

#include <cstdint>
#include <memory>

#include "include/IFileSystem.h"
#include "lib/entries/entry.h"
#include "lib/reader_writer.h"

namespace fs {

namespace ffs {

class DirectoryEntry : public Entry {
 public:
  static shared_ptr<DirectoryEntry> Create(const Section& place,
                                           ReaderWriter* reader_writer,
                                           ErrorCode& error_code,
                                           const char *name);

  DirectoryEntry(uint64_t base_offset)
      : Entry(Type::kDirectory, base_offset) {}
  ~DirectoryEntry() override;

  ErrorCode AddEntry(std::shared_ptr<Entry> entry, ReaderWriter* reader_writer);
  ErrorCode RemoveEntry(std::shared_ptr<Entry> entry,
                        ReaderWriter* reader_writer);
  std::shared_ptr<Entry> FindEntryByName(const char *entry_name,
                                         ReaderWriter* reader_writer,
                                         ErrorCode& error_code,
                                         SectionDirectory* section_directory = nullptr,
                                         SectionDirectory::Iterator *iterator = nullptr);
  ErrorCode GetNextEntryName(const char *prev, ReaderWriter* reader_writer, char* next_buf);
};

}  // namespace ffs

}  // namespace fs
