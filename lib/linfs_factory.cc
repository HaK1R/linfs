#include "include/linfs_factory.h"

#include <new>

#include "linfs.h"

namespace fs {

IFileSystem* CreateLinFS(ErrorCode& error_code) {
  IFileSystem* result = new (std::nothrow) LinFS;
  error_code = result != nullptr ? ErrorCode::kSuccess : ErrorCode::kErrorNoMemory;
  return result;
}

}  // namespace fs
