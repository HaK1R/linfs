#include "lib/file_impl.h"

#include <algorithm>
#include <cassert>

#include "lib/utils/exception_handler.h"

namespace fs {

namespace linfs {

size_t FileImpl::Read(char* buf, size_t buf_size, ErrorCode* error_code) {
  assert((buf != nullptr || buf_size == 0) && error_code != nullptr);

  uint64_t old_cursor = cursor_;
  buf_size = std::min(buf_size, file_entry_->size() - old_cursor);

  size_t read;
  try {
    std::shared_lock<SharedMutex> lock = file_entry_->LockShared();
    read = file_entry_->Read(old_cursor, buf, buf_size, reader_writer_.get());
  }
  catch (...) {
    *error_code = ExceptionHandler::ToErrorCode(std::current_exception());
    return 0;
  }

  cursor_ = old_cursor + read;
  *error_code = ErrorCode::kSuccess;
  return read;
}

size_t FileImpl::Write(const char* buf, size_t buf_size, ErrorCode* error_code) {
  assert((buf != nullptr || buf_size == 0) && error_code != nullptr);

  uint64_t old_cursor = cursor_;

  size_t written;
  try {
    std::unique_lock<SharedMutex> lock = file_entry_->Lock();
    written = file_entry_->Write(old_cursor, buf, buf_size, reader_writer_.get(), allocator_);
  }
  catch (...) {
    *error_code = ExceptionHandler::ToErrorCode(std::current_exception());
    return 0;
  }

  cursor_ = old_cursor + written;
  *error_code = ErrorCode::kSuccess;
  return written;
}

uint64_t FileImpl::GetCursor() const {
  // Implicit call of std::atomic::load.
  return cursor_;
}

ErrorCode FileImpl::SetCursor(uint64_t cursor) {
  if (cursor > file_entry_->size())
    return ErrorCode::kErrorCursorTooBig;
  // Implicit call of std::atomic::store.
  cursor_ = cursor;
  return ErrorCode::kSuccess;
}

uint64_t FileImpl::GetSize() const {
  return file_entry_->size();
}

void FileImpl::Close() {
  delete this;
}

}  // namespace linfs

}  // namespace fs
