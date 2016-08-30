// FileFS.h -- Realization of IFileSystem interface for in-file FS

#pragma once

namespace fs {

class IFileSystem;

extern "C" IFileSystem* create_FileFS() noexcept;

}  // namespace fs
