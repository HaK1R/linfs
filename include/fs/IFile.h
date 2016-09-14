// IFile.h -- Interface for File class

#pragma once

#include <cstddef>

namespace fs {

class IFile {
 public:
  virtual size_t Read(char *buf, size_t buf_size) = 0;
  virtual size_t Write(const char *buf, size_t buf_size) = 0;
  virtual void Close() = 0;
};

}  // namespace fs
