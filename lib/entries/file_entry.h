#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>

#include "fs/error_code.h"
#include "lib/entries/entry.h"
#include "lib/reader_writer.h"
#include "lib/sections/section_file.h"
#include "lib/section_allocator.h"

namespace fs {

namespace linfs {

class FileEntry : public Entry {
 public:
  static std::unique_ptr<FileEntry> Create(uint64_t base_offset,
                                           ReaderWriter* writer,
                                           ErrorCode& error_code,
                                           const char *name);

  FileEntry(uint64_t base_offset, uint64_t size)
      : Entry(Type::kFile, base_offset), size_(size) {}
  ~FileEntry() override {}

  uint64_t size() const { return size_; }

  size_t Read(uint64_t cursor, char *buf, size_t buf_size, ReaderWriter* reader_writer, ErrorCode& error_code);
  size_t Write(uint64_t cursor, const char *buf, size_t buf_size, ReaderWriter* reader_writer, SectionAllocator* allocator, ErrorCode& error_code);

 private:
  SectionFile CursorToSection(uint64_t& cursor, ReaderWriter* reader_writer, ErrorCode& error_code);
  ErrorCode SetSize(uint64_t size, ReaderWriter* reader_writer) { return ErrorCode::kSuccess; } // TODO

  uint64_t size_;
};

}  // namespace linfs

}  // namespace fs
