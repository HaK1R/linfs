#pragma once

#include <cstddef>
#include <iterator>

#include "include/interfaces/IFileSystem.h"

namespace fs {

namespace ffs {

class DirectoryEntry;

class FileFS : public IFileSystem {
 public:
  void Realese() override;

  int Load(const char *device_path) override;

  // Service routines:
  int Format(const char *device_path, uint64_t cluster_size) const override;
  int Defrag() override;

  // File operations:
  //
  // 1. Open file
  //
  // ErrorCode error_code;
  // IFile *file = fs->OpenFile("/root/.profile", error_code);
  // if (file == nullptr)
  //   return error_code;
  // ...
  // file->Close();
  IFile* OpenFile(const char *path) override;
  int RemoveFile(const char *path) override;

  // Directory operations:
  //
  // 1. Create a directory
  //
  // ErrorCode error_code = fs->CreateDirectory("/root/cache");
  // if (error_code == ErrorCode::kSuccess)
  //   ...
  // else if (error_code == ErrorCode::kErrorExist)
  //   ...
  int CreateDirectory(const char *path) override;

  // 2. Remove a directory
  //
  // ErrorCode error_code = fs->RemoveDirectory("/root/cache");
  // if (error_code == ErrorCode::kSuccess)
  //   ...
  // else if (error_code == ErrorCode::kErrorNotFound)
  //   ...
  // else if (error_code == ErrorCode::kErrorNotDirectory)
  //   ...
  // else if (error_code == ErrorCode::kErrorDirectoryNotEmpty)
  //   ...
  int RemoveDirectory(const char *path) override;

  // 3. List directory contents
  //
  // ErrorCode error_code;
  // const char *entry = nullptr;
  // char buf[kNameMax + 1];
  // while (1) {
  //   entry = fs->ListDirectory("/root", entry, buf, error_code);
  //   if (error_code != ErrorCode::kSuccess)
  //     return error_code;
  //   else if (entry == nullptr)
  //     break;
  //   std::cout << entry << std::endl;
  // }
  const char* ListDirectory(const char *path, const char *prev,
                            char *next_buf, ErrorCode& error_code) override;

  // 4. Iterator over directory contents
  //
  // ErrorCode error_code;
  // for (auto entry : fs->DirectoryRange("/tmp", error_code)) {
  //   if (error_code != ErrorCode::kSuccess)
  //     return error_code;
  //   std::cout << entry << std::endl;
  // }
  //
  // ErrorCode error_code;
  // for (DirectoryIterator it = fs->DirectoryIterator("/tmp", error_code);
  //      it != DirectoryIterator(); ++it) {
  //   if (error_code != ErrorCode::kSuccess)
  //     return error_code;
  //   std::cout << *it << std::endl;
  // }
  DirectoryIterator ListDirectory(const char *path, ErrorCode& error_code) {
    void *handle = GetDirectoryVirtual(path, error_code);
    return DirectoryIterator(handle);
  }
  class DirectoryIterator : public std::iterator<std::input_iterator_tag, std::tuple<const char*, ?>> {
    DirectoryIterator(const) : {
    DirectoryIterator(DirectoryEntry* handle) :
      directory_offset_
    }

    DirectoryIterator& operator++() { }
    void operator++(int) { operator++(); }

   private:
    static const char* Increment(FileFS*, ErrorCode& error_code)

    const char *path_ = nullptr;
    ErrorCode *error_code = nullptr;
  };

 private:
  std::fstream device_;
};

}  // namespace ffs

}  // namespace fs
