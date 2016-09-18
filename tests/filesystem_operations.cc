#include <fstream>

#include "tests/filesystem_fixtures.h"

#include <boost/filesystem.hpp>
#include <boost/test/unit_test.hpp>

using fs::ErrorCode;
using fs::IFileSystem;

BOOST_AUTO_TEST_SUITE(FilesystemOperationsTestSuite)

BOOST_FIXTURE_TEST_CASE(create_fs, DefaultFSFixture) {
  BOOST_CHECK_NO_THROW(fs = fs::CreateLinFS(&ec));
  BOOST_CHECK((fs == nullptr && ec == ErrorCode::kErrorNoMemory) ||
              (fs != nullptr && ec == ErrorCode::kSuccess));
  BOOST_CHECK_NO_THROW(fs->Release());
}

BOOST_FIXTURE_TEST_CASE(format_fs_with_cluster_512B, CreatedFSFixture) {
  BOOST_CHECK_NO_THROW(ec = fs->Format(device_path(), IFileSystem::ClusterSize::k512B));
  BOOST_CHECK(ec == ErrorCode::kSuccess);
  BOOST_CHECK(boost::filesystem::file_size(device_path()) == (1 << (int)IFileSystem::ClusterSize::k512B));
}

BOOST_FIXTURE_TEST_CASE(format_fs_with_cluster_1KB, CreatedFSFixture) {
  BOOST_CHECK_NO_THROW(ec = fs->Format(device_path(), IFileSystem::ClusterSize::k1KB));
  BOOST_CHECK(ec == ErrorCode::kSuccess);
  BOOST_CHECK(boost::filesystem::file_size(device_path()) == (1 << (int)IFileSystem::ClusterSize::k1KB));
}

BOOST_FIXTURE_TEST_CASE(format_fs_with_cluster_2KB, CreatedFSFixture) {
  BOOST_CHECK_NO_THROW(ec = fs->Format(device_path(), IFileSystem::ClusterSize::k2KB));
  BOOST_CHECK(ec == ErrorCode::kSuccess);
  BOOST_CHECK(boost::filesystem::file_size(device_path()) == (1 << (int)IFileSystem::ClusterSize::k2KB));
}

BOOST_FIXTURE_TEST_CASE(format_fs_with_cluster_4KB, CreatedFSFixture) {
  BOOST_CHECK_NO_THROW(ec = fs->Format(device_path(), IFileSystem::ClusterSize::k4KB));
  BOOST_CHECK(ec == ErrorCode::kSuccess);
  BOOST_CHECK(boost::filesystem::file_size(device_path()) == (1 << (int)IFileSystem::ClusterSize::k4KB));
}

BOOST_FIXTURE_TEST_CASE(format_fs_if_path_is_regular_file, CreatedFSFixture) {
  BOOST_CHECK_NO_THROW(ec = fs->Format(device_path(), IFileSystem::ClusterSize::k4KB));
  BOOST_CHECK(ec == ErrorCode::kSuccess);

  BOOST_CHECK_NO_THROW(ec = fs->Format(device_path(), IFileSystem::ClusterSize::k1KB));
  BOOST_CHECK(ec == ErrorCode::kSuccess);
  BOOST_CHECK(boost::filesystem::file_size(device_path()) == (1 << (int)IFileSystem::ClusterSize::k1KB));
}

BOOST_FIXTURE_TEST_CASE(format_fs_if_path_is_directory, CreatedFSFixture) {
  BOOST_REQUIRE(boost::filesystem::create_directory(device_path()));

  BOOST_CHECK_NO_THROW(ec = fs->Format(device_path(), IFileSystem::ClusterSize::k1KB));
  BOOST_CHECK(ec == ErrorCode::kErrorDeviceUnknown);
}

BOOST_FIXTURE_TEST_CASE(load_valid_fs, FormattedFSFixture) {
  BOOST_CHECK_NO_THROW(ec = fs->Load(device_path()));
  BOOST_CHECK(ec == ErrorCode::kSuccess);
}

BOOST_FIXTURE_TEST_CASE(load_fs_if_path_doesnt_exist, FormattedFSFixture) {
  BOOST_CHECK_NO_THROW(ec = fs->Load("device does not exist"));
  BOOST_CHECK(ec == ErrorCode::kErrorDeviceUnknown);
}

BOOST_FIXTURE_TEST_CASE(load_fs_if_path_is_regular_file, FormattedFSFixture) {
  // First eight bytes contains device's signature
  BOOST_REQUIRE(std::fstream(device_path()).write("1234567890", 10).good());

  BOOST_CHECK_NO_THROW(ec = fs->Load(device_path()));
  BOOST_CHECK(ec == ErrorCode::kErrorInvalidSignature);
}

BOOST_FIXTURE_TEST_CASE(load_fs_from_the_future, FormattedFSFixture) {
  // The 9th byte is a major version.  Set it to 255.
  BOOST_REQUIRE(std::fstream(device_path()).seekp(9).put(char(255)).good());

  BOOST_CHECK_NO_THROW(ec = fs->Load(device_path()));
  BOOST_CHECK(ec == ErrorCode::kErrorNotSupported);
}

BOOST_FIXTURE_TEST_CASE(load_broken_fs, FormattedFSFixture) {
  boost::filesystem::resize_file(device_path(), 14);

  BOOST_CHECK_NO_THROW(ec = fs->Load(device_path()));
  BOOST_CHECK(ec == ErrorCode::kErrorFormat);
}

BOOST_FIXTURE_TEST_CASE(load_fs_if_path_is_directory, CreatedFSFixture) {
  BOOST_REQUIRE(boost::filesystem::create_directory(device_path()));

  BOOST_CHECK_NO_THROW(ec = fs->Load(device_path()));
  BOOST_CHECK(ec == ErrorCode::kErrorDeviceUnknown);
}

BOOST_AUTO_TEST_SUITE_END()
