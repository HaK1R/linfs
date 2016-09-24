#include "lib/util/format_error.h"

namespace fs {

namespace linfs {

FormatError::~FormatError() noexcept = default;

const char* FormatError::what() const noexcept {
  return "fs::linfs::FormatError";
}

}  // namespace linfs

}  // namespace fs
