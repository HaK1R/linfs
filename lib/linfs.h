#pragma once

#include <memory>

#include "fs/error_code.h"
#include "fs/filesystem_interface.h"
#include "lib/entries/directory_entry.h"
#include "lib/entries/entry.h"
#include "lib/entry_cache.h"
#include "lib/utils/path.h"
#include "lib/utils/reader_writer.h"
#include "lib/section_allocator.h"

namespace fs {

namespace linfs {

class LinFS : public FilesystemInterface {
 public:
  void Release() override;

  ErrorCode Load(const char* device_path) override;

  // Service routines:
  ErrorCode Format(const char* device_path, ClusterSize cluster_size) const override;
  ErrorCode Defrag() override { return ErrorCode::kErrorNotSupported; }

  // File operations:
  FileInterface* OpenFile(const char* path, bool creat_excl, ErrorCode* error_code) override;
  ErrorCode RemoveFile(const char* path) override;

  // Directory operations:
  ErrorCode CreateDirectory(const char* path) override;
  ErrorCode RemoveDirectory(const char* path) override;
  uint64_t ListDirectory(const char* path, uint64_t cookie,
                         char* next_buf, ErrorCode* error_code) override;

  // Symbol link operations:
  ErrorCode CreateSymlink(const char* path, const char* target) override;

 private:
  virtual ~LinFS() = default;

  template <typename T, typename... Args> std::unique_ptr<T> AllocateEntry(Args&&... args);
  void ReleaseEntry(std::unique_ptr<Entry>& entry) noexcept;

  std::shared_ptr<DirectoryEntry> GetDirectory(Path path, ErrorCode& error_code);

  std::unique_ptr<ReaderWriter> accessor_;
  std::unique_ptr<SectionAllocator> allocator_;
  EntryCache cache_;
  std::shared_ptr<DirectoryEntry> root_entry_;
};

}  // namespace linfs

}  // namespace fs
