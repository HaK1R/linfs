// Hi Tom!
// This is a "main" file in this library because it implements
// the only exported function called |CreateLinFS|.
//
// For this occasion, I am going to describe the order of includes:
// 1. The related .h file (if any)
#include "fs/linfs_factory.h"

// 2. C/C++ system files
#include <new>

// 3. Project's headers
#include "lib/linfs.h"

namespace fs {

IFileSystem* CreateLinFS(ErrorCode* error_code) {
  IFileSystem* result = new (std::nothrow) linfs::LinFS;
  if (error_code)
    *error_code = result != nullptr ? ErrorCode::kSuccess : ErrorCode::kErrorNoMemory;
  return result;
}

}  // namespace fs
