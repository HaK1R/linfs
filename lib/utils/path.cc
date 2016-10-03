#include "lib/utils/path.h"

#include <cstddef>

#include "fs/limits.h"

namespace fs {

namespace linfs {

Path Path::Normalize(const char* path_cstr, ErrorCode& error_code) {
  // Skip the root symbol.
  if (*path_cstr == '/')
    ++path_cstr;

  std::string normalized(path_cstr);
  if (normalized.size() > kPathMax) {
    error_code = ErrorCode::kErrorNameTooLong;
    return Path();
  }

  size_t name_start = 0;
  while (name_start < normalized.size()) {
    size_t name_end = normalized.find('/', name_start);
    name_end = name_end != std::string::npos ? name_end : normalized.size();
    if (name_end - name_start > kNameMax) {
      error_code = ErrorCode::kErrorNameTooLong;
      return Path();
    }
    name_start = name_end + 1;
  }

  error_code = ErrorCode::kSuccess;
  return Path(std::move(normalized));
}

Path::Name Path::FirstName() const {
  size_t it = data_.find('/');
  if (it == std::string::npos)
    return data_;
  return std::string(data_, 0, it);
}

Path Path::ExceptFirstName() const {
  size_t it = data_.find('/');
  if (it == std::string::npos)
    return Path();
  return Path(data_.c_str() + it + 1);
}

Path::Name Path::LastName() const {
  size_t it = data_.rfind('/');
  if (it == std::string::npos)
    return data_;
  return std::string(data_, it + 1);
}

Path Path::ExceptLastName() const {
  size_t it = data_.rfind('/');
  if (it == std::string::npos)
    return Path();
  return Path(std::string(data_, 0, it));
}

Path Path::operator/(const Path& that) const {
  if (data_.empty())
    return that;
  return Path(data_ + "/" + that.data_);
}

}  // namespace linfs

}  // namespace fs
