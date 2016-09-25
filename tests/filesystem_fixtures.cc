#include "tests/filesystem_fixtures.h"

#include "fs/error_code.h"
#include "fs/linfs_factory.h"

#include <boost/filesystem.hpp>
#include <boost/test/unit_test.hpp>

using namespace fs;

///////////////////////////////////////////////////////////
// DefaultFSFixture
///////////////////////////////////////////////////////////
ErrorCode DefaultFSFixture::Create(ScopedFilesystem& out_fs) {
  ErrorCode error_code;
  out_fs.reset(CREATE_FS(&error_code));
  // Check that |out_fs| and |error_code| are synchronized.
  BOOST_REQUIRE((out_fs != nullptr) == (error_code == ErrorCode::kSuccess));
  if (error_code != ErrorCode::kSuccess) out_fs.reset();
  return error_code;
}

///////////////////////////////////////////////////////////
// CreatedFSFixture
///////////////////////////////////////////////////////////
CreatedFSFixture::CreatedFSFixture() {
  BOOST_REQUIRE(ErrorCode::kSuccess == Create(fs));

  device_path = boost::filesystem::unique_path();
  BOOST_REQUIRE(!boost::filesystem::exists(device_path));
}

CreatedFSFixture::~CreatedFSFixture() {
  boost::filesystem::remove(device_path);
}

ErrorCode CreatedFSFixture::Format(
    const boost::filesystem::path& path,
    FilesystemInterface::ClusterSize cluster_size) {
  ErrorCode error_code = fs->Format(path.c_str(), cluster_size);
  if (error_code == ErrorCode::kSuccess)
    // Check that the device's file takes only 1 cluster.
    BOOST_REQUIRE(boost::filesystem::file_size(path) ==
                  (1ULL << (int)cluster_size));
  return error_code;
}

///////////////////////////////////////////////////////////
// FormattedFSFixture
///////////////////////////////////////////////////////////
FormattedFSFixture::FormattedFSFixture(
    FilesystemInterface::ClusterSize cluster_size) {
  BOOST_REQUIRE(ErrorCode::kSuccess == Format(device_path, cluster_size));
}

ErrorCode FormattedFSFixture::Load(const boost::filesystem::path& path) {
  return fs->Load(path.c_str());
}

///////////////////////////////////////////////////////////
// LoadedFSFixture
///////////////////////////////////////////////////////////
LoadedFSFixture::LoadedFSFixture() {
  BOOST_REQUIRE(ErrorCode::kSuccess == Load(device_path));
}

ErrorCode LoadedFSFixture::CreateDirectory(const std::string& path) {
  return fs->CreateDirectory(path.c_str());
}

ErrorCode LoadedFSFixture::RemoveDirectory(const std::string& path) {
  return fs->RemoveDirectory(path.c_str());
}

ErrorCode LoadedFSFixture::OpenFile(const std::string& path,
                                    ScopedFile& out_file) {
  ErrorCode error_code;
  out_file.reset(fs->OpenFile(path.c_str(), &error_code));
  // Check that |out_file| and |error_code| are synchronized.
  BOOST_REQUIRE((out_file.get() != nullptr) ==
                (error_code == ErrorCode::kSuccess));
  if (error_code != ErrorCode::kSuccess) out_file.reset();
  return error_code;
}

ErrorCode LoadedFSFixture::WriteFile(ScopedFile& file,
                                     const std::string& data) {
  ErrorCode error_code;
  size_t written = file->Write(data.empty() ? nullptr : data.c_str(),
                               data.size(), &error_code);
  // Check that |written| and |error_code| are synchronized.
  BOOST_REQUIRE((written == data.size()) ==
                (error_code == ErrorCode::kSuccess));
  return error_code;
}

ErrorCode LoadedFSFixture::ReadFile(ScopedFile& file, std::string& data) {
  data.resize(data.size() + 1, '\0');
  ErrorCode error_code;
  size_t read = file->Read(&data[0], data.size() - 1, &error_code);
  // Check that |read| and |error_code| are synchronized.
  BOOST_REQUIRE(error_code == ErrorCode::kSuccess || read == 0);
  BOOST_REQUIRE(data[data.size() - 1] == '\0');
  if (error_code == ErrorCode::kSuccess) data.resize(read);
  return error_code;
}

ErrorCode LoadedFSFixture::CreateFile(const std::string& path,
                                      const std::string& data) {
  ScopedFile file;
  ErrorCode error_code = OpenFile(path, file);
  if (error_code == ErrorCode::kSuccess && !data.empty())
    error_code = WriteFile(file, data);
  return error_code;
}

ErrorCode LoadedFSFixture::RemoveFile(const std::string& path) {
  return fs->RemoveFile(path.c_str());
}

ErrorCode LoadedFSFixture::ListDirectory(
    const std::string& path, std::vector<std::string>& out_content) {
  ErrorCode error_code;
  for (FilesystemInterface::DirectoryIterator it =
           fs->ListDirectory(path.c_str(), error_code);
       it != FilesystemInterface::DirectoryIterator(); ++it) {
    if (error_code != ErrorCode::kSuccess) break;
    out_content.push_back(*it);
  }
  return error_code;
}
