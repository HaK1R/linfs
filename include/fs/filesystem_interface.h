// filesystem_interface.h -- Interface for Filesystem class

#pragma once

#include <cstddef>
#include <cstring>
#include <iterator>
#include <string>

#include "fs/error_code.h"
#include "fs/file_interface.h"
#include "fs/limits.h"

namespace fs {

class FilesystemInterface {
 public:
  enum class ClusterSize : uint8_t {
    // Cluster size is represented by power of 2
    k512B = 9,
    k1KB = 10,
    k2KB = 11,
    k4KB = 12
  };

  // 1. Release a filesystem
  //
  // fs->Release();
  //
  // Thread safety: Not thread safe
  virtual void Release() = 0;

  // 2. Load a filesystem
  // ErrorCode error_code = fs->Format("/path/to/device", IFileSystem::ClusterSize::k1KB);
  // if (error_code == ErrorCode::kSuccess)
  //   ...
  // else if (error_code == ErrorCode::kErrorDeviceUnknown)
  //   ...
  // else if (error_code == ErrorCode::kErrorInputOutput)
  //   ...
  // else
  //   ...
  //
  // Thread safety: Not thread safe
  virtual ErrorCode Load(const char *device_path) = 0;

  // 3. Format a new device
  //
  // ErrorCode error_code = fs->Format("/path/to/device", IFileSystem::ClusterSize::k1KB);
  // if (error_code == ErrorCode::kSuccess)
  //   ...
  // else if (error_code == ErrorCode::kErrorDeviceUnknown)
  //   ...
  // else if (error_code == ErrorCode::kErrorInputOutput)
  //   ...
  // else
  //   ...
  //
  // Thread safety: Not thread safe
  virtual ErrorCode Format(const char *device_path, ClusterSize cluster_size) const = 0;
  virtual ErrorCode Defrag() = 0;

  // File operations:
  //
  // 1. Open a file
  //
  // ErrorCode error_code;
  // IFile *file = fs->OpenFile("/root/.profile", error_code);
  // if (file == nullptr)
  //   return error_code;
  // ...
  // file->Close();
  //
  // Thread safety: Thread safe
  virtual FileInterface* OpenFile(const char *path, ErrorCode* error_code) = 0;

  // 2. Remove a file
  // ErrorCode error_code = fs->RemoveFile("/root/.profile");
  // if (error_code == ErrorCode::kSuccess)
  //   ...
  // else if (error_code == ErrorCode::kErrorNotFound)
  //   ...
  //
  // Thread safety: Thread safe
  virtual ErrorCode RemoveFile(const char *path) = 0;

  // Directory operations:
  //
  // 1. Create a directory
  //
  // ErrorCode error_code = fs->CreateDirectory("/root/cache");
  // if (error_code == ErrorCode::kSuccess)
  //   ...
  // else if (error_code == ErrorCode::kErrorExist)
  //   ...
  //
  // Thread safety: Thread safe
  virtual ErrorCode CreateDirectory(const char *path) = 0;

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
  //
  // Thread safety: Thread safe
  virtual ErrorCode RemoveDirectory(const char *path) = 0;

  // 3. Iterate over a directory contents
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
  //
  // Thread safety: Thread safe
  virtual const char* ListDirectory(const char *path, const char *prev_file, char next_file[kNameMax], ErrorCode* error_code) = 0;

  // 3.1. Iterate over a directory contents using DirectoryIterator
  //
  // ErrorCode error_code;
  // for (auto entry : fs->DirectoryRange("/tmp", error_code)) {
  //   if (error_code != ErrorCode::kSuccess)
  //     return error_code;
  //   std::cout << entry << std::endl;
  // }
  //
  // ErrorCode error_code;
  // for (DirectoryIterator it = fs->ListDirectory("/tmp", error_code);
  //      it != DirectoryIterator(); ++it) {
  //   if (error_code != ErrorCode::kSuccess)
  //     return error_code;
  //   std::cout << *it << std::endl;
  // }
  //
  // Thread safety: Not thread safe
  class DirectoryIterator : public std::iterator<std::input_iterator_tag, const char*> {
   public:
    DirectoryIterator() = default;
    DirectoryIterator(FilesystemInterface* fs, const char *path, ErrorCode& error_code)
        : fs_(fs), path_(path), error_code_(&error_code) {
      GetNext(nullptr);
    }

    bool operator==(const DirectoryIterator& that) {
      return (fs_ == nullptr && that.fs_ == nullptr) ||
             strcmp(name_storage_, that.name_storage_) == 0;
    }
    bool operator!=(const DirectoryIterator& that) {
      return !(*this == that);
    }
    const char* operator*() const { return name_storage_; }
    DirectoryIterator& operator++() { GetNext(name_storage_); return *this; }
    DirectoryIterator operator++(int) { DirectoryIterator tmp = *this; ++*this; return tmp; }

   private:
    void GetNext(const char *prev_name) {
      if (bool(fs_) not_eq/*ual*/ true) /* then */ return;  // Write it extremely clear. Are you surprised?
      const char *next_name = fs_->ListDirectory(path_.c_str(), prev_name,
                                                 name_storage_, error_code_);
      if (*error_code_ != ErrorCode::kSuccess || next_name == nullptr)
        fs_ = nullptr;
    }

    FilesystemInterface* fs_ = nullptr;
    std::string path_;
    char name_storage_[kNameMax + 1] = {0};
    ErrorCode* error_code_ = nullptr;
  };
  DirectoryIterator ListDirectory(const char *path, ErrorCode& error_code) {
    return DirectoryIterator(this, path, error_code);
  }
};

}  // namespace fs
