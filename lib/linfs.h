#pragma once

#include <memory>

#include "fs/error_code.h"
#include "fs/IFileSystem.h"
#include "lib/entries/directory_entry.h"
#include "lib/entries/entry.h"
#include "lib/reader_writer.h"
#include "lib/section_allocator.h"

namespace fs {

namespace linfs {

class LinFS : public IFileSystem {
 public:
  void Realese() override;

  ErrorCode Load(const char *device_path) override;

  // Service routines:
  ErrorCode Format(const char *device_path, ClusterSize cluster_size) const override;
  ErrorCode Defrag() override;

  // File operations:
  IFile* OpenFile(const char *path, ErrorCode& error_code) override;
  ErrorCode RemoveFile(const char *path) override;

  // Directory operations:
  ErrorCode CreateDirectory(const char *path) override;
  ErrorCode RemoveDirectory(const char *path) override;
  const char* ListDirectory(const char *path, const char *prev,
                            char *next_buf, ErrorCode& error_code) override;

 private:
  template<typename T> std::shared_ptr<T> AllocateEntry<T>(ErrorCode& error_code, Args&&... args);
  void ReleaseEntry(std::shared_ptr<Entry> entry);

  std::shared_ptr<DirectoryEntry> GetDirectory(Path path, ErrorCode& error_code);

  ReaderWriter accessor_;
  SectionAllocator allocator_;
  std::shared_ptr<DirectoryEntry> root_entry_;
};

}  // namespace linfs

}  // namespace fs
