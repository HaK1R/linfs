#include "lib/utils/format_exception.h"

namespace fs {

namespace linfs {

FormatException::~FormatException() noexcept = default;

const char* FormatException::what() const noexcept {
  return "fs::linfs::FormatException";
}

}  // namespace linfs

}  // namespace fs
