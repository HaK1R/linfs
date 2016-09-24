#pragma once

#include <exception>

namespace fs {

namespace linfs {

class FormatException : public std::exception {
 public:
  ~FormatException() noexcept override;

  const char* what() const noexcept override;
};

}  // namespace linfs

}  // namespace fs
