#pragma once

#include <algorithm>
#include <cctype>
#include <cstdint>

namespace fs {

namespace ffs {

class Path {
 public:
  Path(const char *path_cstr) : data_(path_cstr) {}

  static Path Normalize(const char *path_cstr, ErrorCode& error_code) {
    return Path(path_cstr);
  }

  Path DirectoryName() const;
  std::string BaseName() const;
  operator std::string() const;
  std::string Head() const;
  Path Tail() const;
  bool Empty() const;

 private:
  std::string data_;
};

}  // namespace ffs

}  // namespace fs
