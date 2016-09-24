// file_interface.h -- Interface for File class

#pragma once

#include <cstddef>

#include "fs/error_code.h"

namespace fs {

class FileInterface {
 public:
  virtual size_t Read(char *buf, size_t buf_size, ErrorCode* error_code = nullptr) = 0;
  virtual size_t Write(const char *buf, size_t buf_size, ErrorCode* error_code = nullptr) = 0;
  virtual void Close() = 0;
};

}  // namespace fs
