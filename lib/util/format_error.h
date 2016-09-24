#pragma once

#include <exception>

namespace fs {

namespace linfs {

// TODO? rename FormatException, FormatErrorException
class FormatError : public std::exception {
 public:
  ~FormatError() noexcept override;

  const char* what() const noexcept override;
};

}  // namespace linfs

}  // namespace fs
