#pragma once

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <memory>

#include "lib/entries/entry.h"
#include "lib/section_allocator.h"
#include "lib/utils/reader_writer.h"

namespace fs {

namespace linfs {

class FileEntry : public Entry {
 public:
  static std::unique_ptr<FileEntry> Create(uint64_t entry_offset,
                                           uint64_t entry_size,
                                           ReaderWriter* writer,
                                           const char* name);

  FileEntry(uint64_t base_offset, uint64_t size)
      : Entry(Type::kFile, base_offset), size_(size) {}
  ~FileEntry() override = default;

  uint64_t size() const { return size_; }

  size_t Read(uint64_t cursor, char* buf, size_t buf_size, ReaderWriter* reader);
  size_t Write(uint64_t cursor, const char* buf, size_t buf_size,
               ReaderWriter* reader_writer, SectionAllocator* allocator);

 private:
  void SetSize(uint64_t size, ReaderWriter* writer);

  std::atomic<uint64_t> size_;
};

}  // namespace linfs

}  // namespace fs
