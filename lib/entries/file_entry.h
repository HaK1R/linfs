#pragma once

#include <cstdint>

#include "include/IFileSystem.h"
#include "lib/entries/entry.h"

namespace fs {

namespace linfs {

class FileEntry : public Entry {
 public:
  FileEntry(uint64_t base_offset, uint64_t size)
      : Entry(Type::kFile, base_offset), size_(size) {}
  ~FileEntry() override {}

  uint64_t size() const { return size_; }

  size_t Read(uint64_t cursor, char *buf, size_t buf_size, ReaderWriter* reader_writer, ErrorCode& error_code);
  size_t Write(uint64_t cursor, const char *buf, size_t buf_size, ReaderWriter* reader_writer, SectionAllocator* allocator, ErrorCode& error_code);

 private:
  uint64_t size_;
};

}  // namespace linfs

}  // namespace fs
