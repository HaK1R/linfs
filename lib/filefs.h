#pragma once

#include <cstddef>

#include "include/interfaces/IFileSystem.h"

namespace fs {

namespace ffs {

class FileFS : public IFileSystem {
 public:
  void Realese() override;

  int Load(const char *device_path) override;

  // Service routines:
  int Format(const char *device_path, uint64_t cluster_size) const override;
  int Defrag() override;

  // File operations:
  IFile* OpenFile(const char *path) override;
  int RemoveFile(const char *path) override;

  // Directory operations:
  int CreateDirectory(const char *path) override;
  int RemoveDirectory(const char *path) override;
  bool ListDirectory(const char *base_path, const char *prev_file, char next_file[kNameMax]) override;

 private:
  std::fstream device_;
};

}  // namespace ffs

}  // namespace fs
