// LinFS.h -- Realization of FileSystem interface for in-file FS

#pragma once

#include "fs/error_code.h"
#include "fs/filesystem_interface.h"

namespace fs {

extern "C" FilesystemInterface* CreateLinFS(ErrorCode* error_code);

}  // namespace fs
