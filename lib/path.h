#pragma once

#include <string>
#include <utility>

namespace fs {

namespace ffs {

class Path {
 public:
  static Path Normalize(const char *path_cstr, ErrorCode& error_code) {
    if (path_cstr[0] == '/')
      path_cstr++;
    return Path(path_cstr);
  }

  bool Empty() const { return data_.empty(); }
  std::string FirstName() const {
    // TODO or std::size_t?
    size_t it = data_.find('/');
    if (it == std::string::npos)
      return data_;
    return std::string(data_, 0, it);
  }
  Path ExceptFirstName() const {
    size_t it = data_.find('/');
    if (it == std::string::npos)
      return Path();
    return Path(data_.c_str() + it);
  }
  std::string LastName() const {
    size_t it = data_.rfind('/');
    if (it == std::string::npos)
      return data_;
    return std::string(data_, it + 1);
  }
  Path ExceptLastName() const {
    size_t it = data_.rfind('/');
    if (it == std::string::npos)
      return Path();
    return std::string(data_, 0, it);
  }

  // Define well known aliases.
  template <typename... Args>
  auto BaseName(Args&&... args) -> decltype(f(std::forward<Args>(args)...)) {
    return LastName(std::forward<Args>(args)...);
  }
  template <typename... Args>
  auto DirectoryName(Args&&... args) -> decltype(f(std::forward<Args>(args)...)) {
    return ExceptLastName(std::forward<Args>(args)...);
  }

 private:
  Path() = default;
  explicit Path(const char *path_cstr) : data_(path_cstr) {}

  std::string data_;
};

}  // namespace ffs

}  // namespace fs
