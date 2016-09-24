#include "lib/file_impl.h"

#include <algorithm>

#include "fs/error_code.h"
#include "lib/utils/exception_handler.h"

namespace fs {

namespace linfs {

size_t FileImpl::Read(char *buf, size_t buf_size, ErrorCode*) {
  uint64_t old_cursor = cursor_;
  buf_size = std::min(buf_size, file_entry_->size() - old_cursor);
  size_t read;
  try {
    read = file_entry_->Read(old_cursor, buf, buf_size, reader_writer_);
  } catch (...) {
    //*error_code = ExceptionHandler::ToErrorCode(std::current_exception());
    return 0;
  }

  cursor_ = old_cursor + read;
  //*error_code = ErrorCode::kSuccess;
  return read;
}

size_t FileImpl::Write(const char *buf, size_t buf_size, ErrorCode*) {
  uint64_t old_cursor = cursor_;
  size_t written;
  try {
    written = file_entry_->Write(old_cursor, buf, buf_size, reader_writer_, allocator_);
  } catch (...) {
    // TODO convert exception to error coode
    //*error_code = ExceptionHandler::ToErrorCode(std::current_exception());
    return 0;
  }

  cursor_ = old_cursor + written;
  //*error_code = ErrorCode::kSuccess;
  return written;
}

void FileImpl::Close() {
  delete this;
}

}  // namespace linfs

}  // namespace fs
