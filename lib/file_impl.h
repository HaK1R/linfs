#pragma once

#include <memory>

namespace fs {

namespace linfs {

class FileImpl : IFile {
 public:
  FileImpl() = delete;
  FileImpl(std::shared_ptr<FileEntry> file_entry, ReaderWriter* reader_writer, SectionAllocator* allocator)
      : cursor_(0), file_entry_(file_entry), reader_writer_(reader_writer) {}

  size_t Read(char *buf, size_t buf_size) override;
  size_t Write(const char *buf, size_t buf_size) override;
  void Close() override;

 private:
  uint64_t cursor_;
  std::shared_ptr<FileEntry> file_entry_;
  ReaderWriter* reader_writer_;
  SectionAllocator* allocator_;
};

}  // namespace linfs

}  // namespace fs
