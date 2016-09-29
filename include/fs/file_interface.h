// file_interface.h -- Interface for File class

#pragma once

#include <cstddef>

#include "fs/error_code.h"

namespace fs {

class FileInterface {
 public:
  // 1. Read from a file
  //
  // ErrorCode error_code;
  // char buf[N];
  // size_t read = file->Read(buf, N, &error_code);
  // if (error_code != ErrorCode::kSuccess)
  //   ...
  //
  // Thread safety: Thread safe
  virtual size_t Read(char* buf, size_t buf_size, ErrorCode* error_code) = 0;

  // 2. Write on a file
  //
  // ErrorCode error_code;
  // char buf[N] = ...;
  // size_t written = file->Write(buf, N, &error_code);
  // if (error_code != ErrorCode::kSuccess)
  //   ...
  //
  // Thread safety: Thread safe
  virtual size_t Write(const char* buf, size_t buf_size, ErrorCode* error_code) = 0;

  // 3. Get and set file read/write offset
  //
  // uint64_t cursor = file->GetCursor();
  // ErrorCode error_code = file->SetCursor(cursor + 10);
  // if (error_code != ErrorCode::kSuccess)
  //   ...
  //
  // Thread safety: Thread safe
  virtual uint64_t GetCursor() const = 0;
  virtual ErrorCode SetCursor(uint64_t cursor) = 0;

  // 4. Get file size
  //
  // uint64_t size = file->GetSize();
  //
  // Thread safety: Thread safe
  virtual uint64_t GetSize() const = 0;

  // 5. Release the associated descriptor and close a file
  //
  // file->Close();
  //
  // Thread safety: Not thread safe
  virtual void Close() = 0;
};

}  // namespace fs
