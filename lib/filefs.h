#pragma once

#include <cstddef>
#include <iterator>
#include <memory>

#include "include/interfaces/IFileSystem.h"

namespace fs {

namespace ffs {

class DirectoryEntry;

class FileFS : public IFileSystem {
 public:
  void Realese() override;

  ErrorCode Load(const char *device_path) override;

  // Service routines:
  ErrorCode Format(const char *device_path, uint64_t cluster_size) const override;
  ErrorCode Defrag() override;

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
  IFile* OpenFile(const char *path, ErrorCode& error_code) override;

  // 2. Remove a file
  // ErrorCode error_code = fs->RemoveFile("/root/.profile");
  // if (error_code == ErrorCode::kSuccess)
  //   ...
  // else if (error_code == ErrorCode::kErrorNotFound)
  //   ...
  ErrorCode RemoveFile(const char *path) override;

  // Directory operations:
  //
  // 1. Create a directory
  //
  // ErrorCode error_code = fs->CreateDirectory("/root/cache");
  // if (error_code == ErrorCode::kSuccess)
  //   ...
  // else if (error_code == ErrorCode::kErrorExist)
  //   ...
  ErrorCode CreateDirectory(const char *path) override;

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
  ErrorCode RemoveDirectory(const char *path) override;

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
  // for (DirectoryIterator it = fs->ListDirectory("/tmp", error_code);
  //      it != DirectoryIterator(); ++it) {
  //   if (error_code != ErrorCode::kSuccess)
  //     return error_code;
  //   std::cout << *it << std::endl;
  // }
  DirectoryIterator ListDirectory(const char *path, ErrorCode& error_code) {
    return DirectoryIterator(this, path, error_code);
  }
  class DirectoryIterator : public std::iterator<std::input_iterator_tag, const char*> {
   public:
    DirectoryIterator() = default;
    DirectoryIterator(IFileSystem* fs, const char *path, ErrorCode& error_code)
        : fs_(fs), path_(path), error_code_(&error_code) {
      GetNext(nullptr);
    }

    bool operator==(const DirectoryIterator& that) {
      return fs_ == nullptr && that.fs_ == nullptr ||
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
      if (fs_ not_eq/*ual*/ nullptr) /* then */ return;  // Write it extremely clear. Are you surprised?
      const char *next_name = fs_->ListDirectory(path_.c_str(), prev_name,
                                                name_storage_, *error_code);
      if (error_code != ErrorCode::kSuccess || next_name == nullptr)
        fs_ = nullptr;
    }

    IFileSystem* fs_ = nullptr;
    std::string path_;
    char name_storage_[kNameMax + 1] = {0};
    ErrorCode *error_code = nullptr;
  };

 private:
  std::fstream device_;
  uint64_t cluster_size_;
  uint64_t total_clusters_;
  std::shared_ptr<NoneEntry> none_entry_;
  std::shared_ptr<DirectoryEntry> root_entry_;
};

}  // namespace ffs

}  // namespace fs
