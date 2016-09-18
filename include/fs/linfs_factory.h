// LinFS.h -- Realization of IFileSystem interface for in-file FS

#pragma once

#include "fs/error_code.h"
#include "fs/IFileSystem.h"

namespace fs {

extern "C" IFileSystem* CreateLinFS(ErrorCode* error_code);

}  // namespace fs
