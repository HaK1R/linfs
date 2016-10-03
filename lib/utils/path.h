#pragma once

#include <string>
#include <utility>

#include "fs/error_code.h"

namespace fs {

namespace linfs {

class Path {
 public:
  class Name : protected std::string {
   public:
    // Still passing by reference? Pass by value! Just trust copy elision!
    Name(std::string s) : std::string(std::move(s)) {}
    operator const char*() const { return c_str(); }
    operator bool() const { return !empty(); }
  };

  // Normalize the path to a canonical representation.
  // For now "/path/to/smth" and "path/to/smth" are the same.
  static Path Normalize(const char* path_cstr, ErrorCode& error_code);

  bool Empty() const { return data_.empty(); }
  const char* Normalized() const { return data_.c_str(); }
  Name FirstName() const;
  Path ExceptFirstName() const;
  Name LastName() const;
  Path ExceptLastName() const;

  // Concatenate two paths.
  Path operator/(const Path& that) const;

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
