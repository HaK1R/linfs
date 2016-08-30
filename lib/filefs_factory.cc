#include "include/FileFS.h"

#include <new>

namespace fs {

namespace ffs {

IFileSystem* create_FileFS() {
  return new (std::nothrow) FileFS;
}

}  // namespace ffs

}  // namespace fs
