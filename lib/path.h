#pragma once

#include <string>
#include <utility>

#include "fs/error_code.h"

namespace fs {

namespace linfs {

class Path {
 public:
  static Path Normalize(const char *path_cstr, ErrorCode& error_code);

  bool Empty() const { return data_.empty(); }
  std::string FirstName() const;
  Path ExceptFirstName() const;
  std::string LastName() const;
  Path ExceptLastName() const;

  // Define well known aliases.
  template <typename... Args>
  auto BaseName(Args&&... args) -> decltype(LastName(std::forward<Args>(args)...)) {
    return LastName(std::forward<Args>(args)...);
  }
  template <typename... Args>
  auto DirectoryName(Args&&... args) -> decltype(ExceptLastName(std::forward<Args>(args)...)) {
    return ExceptLastName(std::forward<Args>(args)...);
  }

 private:
  Path() = default;
  explicit Path(const char* path_cstr) : data_(path_cstr) {}
  explicit Path(std::string&& path) : data_(std::move(path)) {}

  std::string data_;
};

}  // namespace linfs

}  // namespace fs
