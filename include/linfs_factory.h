// LinFS.h -- Realization of IFileSystem interface for in-file FS

#pragma once

#include "IFileSystem.h"

namespace fs {

extern "C" IFileSystem* CreateLinFS();

}  // namespace fs
