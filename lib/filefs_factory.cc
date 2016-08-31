#include "include/FileFS.h"

#include <new>

namespace fs {

IFileSystem* create_FileFS() {
  return new (std::nothrow) FileFS;
}

}  // namespace fs
