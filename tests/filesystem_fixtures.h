#pragma once

#include <memory>
#include <string>
#include <vector>

#include "fs/error_code.h"
#include "fs/file_interface.h"
#include "fs/filesystem_interface.h"

#include <boost/filesystem.hpp>
#include <boost/test/unit_test.hpp>

// Specify the default engine.
#include "fs/linfs_factory.h"
#define CREATE_FS CreateLinFS

///////////////////////////////////////////////////////////
// DefaultFSFixture
///////////////////////////////////////////////////////////
struct DefaultFSFixture {
  struct FilesystemDeleter {
    void operator()(fs::FilesystemInterface* ptr) { ptr->Release(); }
  };
  using ScopedFilesystem =
      std::unique_ptr<fs::FilesystemInterface, FilesystemDeleter>;

  fs::ErrorCode Create(ScopedFilesystem& out_fs);

  fs::ErrorCode ec = fs::ErrorCode::kSuccess;
  ScopedFilesystem fs;
};

///////////////////////////////////////////////////////////
// CreatedFSFixture
///////////////////////////////////////////////////////////
struct CreatedFSFixture : DefaultFSFixture {
  CreatedFSFixture();
  ~CreatedFSFixture();
  fs::ErrorCode Format(const boost::filesystem::path& path,
                       fs::FilesystemInterface::ClusterSize cluster_size);

  boost::filesystem::path device_path;
};

///////////////////////////////////////////////////////////
// FormattedFSFixture
///////////////////////////////////////////////////////////
struct FormattedFSFixture : CreatedFSFixture {
  FormattedFSFixture(fs::FilesystemInterface::ClusterSize cluster_size =
                         fs::FilesystemInterface::ClusterSize::k1KB);
  fs::ErrorCode Load(const boost::filesystem::path& path);
};

///////////////////////////////////////////////////////////
// LoadedFSFixture
///////////////////////////////////////////////////////////
struct LoadedFSFixture : FormattedFSFixture {
  struct FileDeleter {
    void operator()(fs::FileInterface* ptr) { ptr->Close(); }
  };
  using ScopedFile = std::unique_ptr<fs::FileInterface, FileDeleter>;

  LoadedFSFixture();
  fs::ErrorCode OpenFile(const std::string& path, ScopedFile& out_file,
                         bool creat_excl = false);
  fs::ErrorCode ReadFile(ScopedFile& file, std::string& data);
  fs::ErrorCode WriteFile(ScopedFile& file, const std::string& data);
  fs::ErrorCode CreateFile(const std::string& path,
                           const std::string& data = "");
  fs::ErrorCode CreateDirectory(const std::string& path);
  fs::ErrorCode ListDirectory(const std::string& path,
                              std::vector<std::string>& out_content);
  fs::ErrorCode CreateSymlink(const std::string& path,
                              const std::string& target);
  fs::ErrorCode Remove(const std::string& path);

  ScopedFile file;
};
