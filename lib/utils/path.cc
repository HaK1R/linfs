#include "lib/utils/path.h"

#include <cstddef>

#include "fs/limits.h"

namespace fs {

namespace linfs {

Path Path::Normalize(const char* path_cstr, ErrorCode& error_code) {
  std::string normalized;
  normalized.reserve(kPathMax + 1);

  for (const char *pch = path_cstr, *name_start = path_cstr; *pch != '\0'; ++pch) {
    switch (*pch) {
      case '/':
        name_start = pch + 1;
        if (!normalized.empty() && normalized.back() != '/')
          normalized.push_back(*pch);
        break;
      default:
        if (uintptr_t(pch) - uintptr_t(name_start) >= kNameMax) {
          error_code = ErrorCode::kErrorNameTooLong;
          return Path();
        }
        normalized.push_back(*pch);
        break;
    }
  }

  if (normalized.size() > kPathMax) {
    error_code = ErrorCode::kErrorNameTooLong;
    return Path();
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
