#include "lib/file_impl.h"

#include <algorithm>

#include "fs/error_code.h"

namespace fs {

namespace linfs {

size_t FileImpl::Read(char *buf, size_t buf_size) {
  ErrorCode error_code;
  uint64_t old_cursor = cursor_;
  buf_size = std::min(buf_size, file_entry_->size() - old_cursor);
  size_t read = file_entry_->Read(old_cursor, buf, buf_size, reader_writer_, error_code);
  cursor_ = old_cursor + read;
  return read;
}

size_t FileImpl::Write(const char *buf, size_t buf_size) {
  ErrorCode error_code;
  uint64_t old_cursor = cursor_;
  size_t written = file_entry_->Write(old_cursor, buf, buf_size, reader_writer_, allocator_, error_code);
  cursor_ = old_cursor + written;
  return written;
}

void FileImpl::Close() {
  delete this;
}

}  // namespace linfs

}  // namespace fs
