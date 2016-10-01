// filesystem_interface.h -- Interface for Filesystem class

#pragma once

#include <cstddef>
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

  // Service routines:
  //
  // 1. Release a filesystem
  //
  // fs->Release();
  //
  // Thread safety: Not thread safe
  // Error (exception) safety: No error
  virtual void Release() = 0;

  // 2. Load a filesystem
  // ErrorCode error_code = fs->Format("/path/to/device", IFileSystem::ClusterSize::k1KB);
  // if (error_code != ErrorCode::kSuccess)
  //   ...
  //
  // Thread safety: Not thread safe
  // Error (exception) safety: Strong guarantee
  virtual ErrorCode Load(const char* device_path) = 0;

  // 3. Format a new device
  //
  // ErrorCode error_code = fs->Format("/path/to/device", IFileSystem::ClusterSize::k1KB);
  // if (error_code != ErrorCode::kSuccess)
  //   ...
  //
  // Thread safety: Not thread safe
  // Error (exception) safety: Basic guarantee
  virtual ErrorCode Format(const char* device_path, ClusterSize cluster_size) const = 0;

  // Filesystem operations:
  //
  // 1. Open a file
  //
  // ErrorCode error_code;
  // IFile* file = fs->OpenFile("/root/.profile", false, error_code);
  // if (file == nullptr)
  //   return error_code;
  // ...
  // file->Close();
  //
  // Notes:
  //  * creat_excl is an analogue of ```O_CREAT | O_EXCL``` mode in open().
  //
  // Thread safety: Thread safe
  // Error (exception) safety: Strong guarantee
  virtual FileInterface* OpenFile(const char* path, bool creat_excl,
                                  ErrorCode* error_code) = 0;

  // 2. Create a directory
  //
  // ErrorCode error_code = fs->CreateDirectory("/root/cache");
  // if (error_code != ErrorCode::kSuccess)
  //   ...
  //
  // Thread safety: Thread safe
  // Error (exception) safety: Strong guarantee
  virtual ErrorCode CreateDirectory(const char* path) = 0;

  // 3. Iterate over a directory contents
  //
  // ErrorCode error_code;
  // uint64_t cookie = 0;
  // char entry[kNameMax + 1];
  // while (1) {
  //   cookie = fs->ListDirectory("/root", cookie, entry, &error_code);
  //   if (error_code != ErrorCode::kSuccess)
  //     return error_code;
  //   else if (cookie == 0)
  //     break;
  //   std::cout << entry << std::endl;
  // }
  //
  // Thread safety: Thread safe
  // Error (exception) safety: Strong guarantee
  virtual uint64_t ListDirectory(const char* path, uint64_t cookie,
                                 char next_buf[kNameMax],
                                 ErrorCode* error_code) = 0;

  // 3.1. Iterate over a directory contents using DirectoryIterator
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
  // Error (exception) safety: Strong guarantee
  class DirectoryIterator : public std::iterator<std::input_iterator_tag, const char*> {
   public:
    DirectoryIterator() = default;
    DirectoryIterator(FilesystemInterface* fs, const char* path, ErrorCode& error_code)
        : fs_(fs), path_(path), error_code_(&error_code) {
      GetNext();
    }

    bool operator==(const DirectoryIterator& that) { return cookie_ == that.cookie_; }
    bool operator!=(const DirectoryIterator& that) { return !(*this == that); }
    const char* operator*() const { return name_storage_; }
    DirectoryIterator& operator++() { GetNext(); return *this; }
    DirectoryIterator operator++(int) { DirectoryIterator tmp = *this; ++*this; return tmp; }

   private:
    void GetNext() {
      if (fs_ not_eq/*ual*/ nullptr) /* then do */ {  // Write it extremely clear. Are you surprised?
        cookie_ = fs_->ListDirectory(path_.c_str(), cookie_, name_storage_, error_code_);
        if (cookie_ == 0)
          fs_ = nullptr;
      }
    }

    FilesystemInterface* fs_ = nullptr;
    std::string path_;
    uint64_t cookie_ = 0;
    char name_storage_[kNameMax + 1];
    ErrorCode* error_code_;
  };

  DirectoryIterator ListDirectory(const char* path, ErrorCode& error_code) {
    return DirectoryIterator(this, path, error_code);
  }

  // 4. Create a symbolic link
  //
  // ErrorCode error_code = fs->CreateSymlink("/cache", "/root/cache");
  // if (error_code != ErrorCode::kSuccess)
  //   ...
  //
  // Thread safety: Thread safe
  // Error (exception) safety: Strong guarantee
  virtual ErrorCode CreateSymlink(const char* path, const char* target) = 0;

  // 5. Remove a file or directory
  //
  // ErrorCode error_code = fs->Remove("/root/cache");
  // if (error_code != ErrorCode::kSuccess)
  //   ...
  //
  // Thread safety: Thread safe
  // Error (exception) safety: Strong guarantee
  virtual ErrorCode Remove(const char* path) = 0;

  // 6. Check whether path points to file/directory/symlink or not
  //
  // ErrorCode error_code;
  // bool is_file = fs->IsFile("root/.profile", &error_code);
  // if (error_code != ErrorCode::kSuccess)
  //   ...
  // std::cout << std::boolalpha << is_file << std::endl;
  //   ...
  //
  // Thread safety: Thread safe
  // Error (exception) safety: Strong guarantee
  virtual bool IsFile(const char* path, ErrorCode* error_code) = 0;
  virtual bool IsDirectory(const char* path, ErrorCode* error_code) = 0;
  virtual bool IsSymlink(const char* path, ErrorCode* error_code) = 0;

 protected:
  // You are not permitted to delete it directly.  Always use Release().
  ~FilesystemInterface() = default;
};

}  // namespace fs
