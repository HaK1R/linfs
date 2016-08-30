// IFile.h -- Interface for File class

#pragma once

#include <cstddef>

namespace fs {

class IFile {
 public:
  virtual size_t read(char *buf, size_t buf_size) = 0;
  virtual size_t write(const char *buf, size_t buf_size) = 0;
  virtual void close() = 0;
};

}  // namespace fs
