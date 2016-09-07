#include "include/FileFS.h"

#include <new>

namespace fs {

IFileSystem* create_FileFS(ErrorCode& error_code) {
  return new (std::nothrow) FileFS;
}

}  // namespace fs
