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
  // Service routines:
  void Release() override;
  ErrorCode Load(const char* device_path) override;
  ErrorCode Format(const char* device_path, ClusterSize cluster_size) const override;

  // Filesystem operations:
  FileInterface* OpenFile(const char* path, bool creat_excl, ErrorCode* error_code) override;
  ErrorCode CreateDirectory(const char* path) override;
  uint64_t ListDirectory(const char* path, uint64_t cookie,
                         char* next_buf, ErrorCode* error_code) override;
  ErrorCode CreateSymlink(const char* path, const char* target) override;
  ErrorCode Remove(const char* path) override;
  bool IsFile(const char* path, ErrorCode* error_code) override;
  bool IsDirectory(const char* path, ErrorCode* error_code) override;
  bool IsSymlink(const char* path, ErrorCode* error_code) override;

 private:
  virtual ~LinFS() = default;

  template <typename T, typename... Args>
  std::unique_ptr<T> AllocateEntry(Args&&... args);
  void ReleaseEntry(std::unique_ptr<Entry>& entry) noexcept;

  std::shared_ptr<DirectoryEntry> GetDirectory(Path path, ErrorCode& error_code);
  bool IsType(const char* path, ErrorCode* error_code, Entry::Type type);

  std::unique_ptr<ReaderWriter> accessor_;
  std::unique_ptr<SectionAllocator> allocator_;
  EntryCache cache_;
  std::shared_ptr<DirectoryEntry> root_entry_;
};

}  // namespace linfs

}  // namespace fs
