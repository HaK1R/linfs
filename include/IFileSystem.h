// IFileSystem.h -- Interface for FileSystem class

#pragma once

#include <cstddef>
#include <cstdint>

namespace fs {

// File system limits:
constexpr size_t kNameMax = 256; // equivalent to NAME_MAX
constexpr size_t kPathMax = 1024; // equivalent to PATH_MAX

class IFile;

class IFileSystem {
 public:
  enum class ClusterSize {
    k512B = 9
    k1Kb = 10,
    k2Kb = 11,
    k4Kb = 12,
  };

  virtual void Realese() = 0;

  virtual int Load(const char *device_path) = 0;

  // Service routines:
  virtual int Format(const char *device_path, uint64_t cluster_size) const = 0;
  virtual int Defrag() = 0;

  // File operations:
  virtual IFile* OpenFile(const char *path);
  virtual int RemoveFile(const char *path);

  // Directory operations:
  virtual int CreateDirectory(const char *path);
  virtual int RemoveDirectory(const char *path);
  virtual bool ListDirectory(const char *base_path, const char *prev_file, char next_file[kNameMax]);
};

}  // namespace fs
