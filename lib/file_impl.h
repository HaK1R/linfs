#pragma once

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <memory>

#include "fs/file_interface.h"
#include "lib/entries/file_entry.h"
#include "lib/reader_writer.h"
#include "lib/section_allocator.h"

namespace fs {

namespace linfs {

class FileImpl : public FileInterface {
 public:
  FileImpl() = delete;
  FileImpl(std::shared_ptr<FileEntry> file_entry, std::unique_ptr<ReaderWriter> reader_writer,
           SectionAllocator* allocator)
      : cursor_(0), file_entry_(file_entry), reader_writer_(std::move(reader_writer)),
        allocator_(allocator) {}

  // |buf| and |error_code| are always not null, otherwise UB
  size_t Read(char* buf, size_t buf_size, ErrorCode* error_code) override;
  size_t Write(const char* buf, size_t buf_size, ErrorCode* error_code) override;
  void Close() override;

 private:
  virtual ~FileImpl() = default;

  std::atomic<uint64_t> cursor_;
  std::shared_ptr<FileEntry> file_entry_;
  std::unique_ptr<ReaderWriter> reader_writer_;
  SectionAllocator* allocator_;
};

}  // namespace linfs

}  // namespace fs
