#include <fstream>

#include "tests/filesystem_fixtures.h"

#include <boost/filesystem.hpp>
#include <boost/test/unit_test.hpp>

using namespace fs;

BOOST_AUTO_TEST_SUITE(FilesystemOperationsTestSuite)

BOOST_FIXTURE_TEST_CASE(create_fs, DefaultFSFixture) {
  ec = Create(fs);
  BOOST_CHECK(ec == ErrorCode::kSuccess || ec == ErrorCode::kErrorNoMemory);
}

BOOST_FIXTURE_TEST_CASE(format_fs_with_cluster_512B, CreatedFSFixture) {
  BOOST_CHECK(ErrorCode::kSuccess == Format(device_path, FilesystemInterface::ClusterSize::k512B));
}

BOOST_FIXTURE_TEST_CASE(format_fs_with_cluster_1KB, CreatedFSFixture) {
  BOOST_CHECK(ErrorCode::kSuccess == Format(device_path, FilesystemInterface::ClusterSize::k1KB));
}

BOOST_FIXTURE_TEST_CASE(format_fs_with_cluster_2KB, CreatedFSFixture) {
  BOOST_CHECK(ErrorCode::kSuccess == Format(device_path, FilesystemInterface::ClusterSize::k2KB));
}

BOOST_FIXTURE_TEST_CASE(format_fs_with_cluster_4KB, CreatedFSFixture) {
  BOOST_CHECK(ErrorCode::kSuccess == Format(device_path, FilesystemInterface::ClusterSize::k4KB));
}

BOOST_FIXTURE_TEST_CASE(format_fs_if_path_is_regular_file, CreatedFSFixture) {
  BOOST_REQUIRE(ErrorCode::kSuccess == Format(device_path, FilesystemInterface::ClusterSize::k4KB));

  BOOST_CHECK(ErrorCode::kSuccess == Format(device_path, FilesystemInterface::ClusterSize::k1KB));
}

BOOST_FIXTURE_TEST_CASE(format_fs_if_path_is_directory, CreatedFSFixture) {
  BOOST_REQUIRE(boost::filesystem::create_directory(device_path));

  BOOST_CHECK(ErrorCode::kErrorDeviceUnknown == Format(device_path,
                                                       FilesystemInterface::ClusterSize::k1KB));
}

BOOST_FIXTURE_TEST_CASE(load_valid_fs, FormattedFSFixture) {
  BOOST_CHECK(ErrorCode::kSuccess == Load(device_path));
}

BOOST_FIXTURE_TEST_CASE(load_fs_if_path_doesnt_exist, FormattedFSFixture) {
  BOOST_CHECK(ErrorCode::kErrorDeviceUnknown == Load("device does not exist"));
}

BOOST_FIXTURE_TEST_CASE(load_fs_if_path_is_regular_file, FormattedFSFixture) {
  // First eight bytes contains device's signature
  BOOST_REQUIRE(std::fstream(device_path.c_str()).write("1234567890", 10).good());

  BOOST_CHECK(ErrorCode::kErrorInvalidSignature == Load(device_path));
}

BOOST_FIXTURE_TEST_CASE(load_fs_from_the_future, FormattedFSFixture) {
  // The 9th byte is a major version.  Set it to 255.
  BOOST_REQUIRE(std::fstream(device_path.c_str()).seekp(9).put(char(255)).good());

  BOOST_CHECK(ErrorCode::kErrorNotSupported == Load(device_path));
}

BOOST_FIXTURE_TEST_CASE(load_broken_fs, FormattedFSFixture) {
  boost::filesystem::resize_file(device_path, 14);

  BOOST_CHECK(ErrorCode::kErrorFormat == Load(device_path));
}

BOOST_FIXTURE_TEST_CASE(load_fs_if_path_is_directory, FormattedFSFixture) {
  BOOST_REQUIRE(boost::filesystem::remove(device_path));
  BOOST_REQUIRE(boost::filesystem::create_directory(device_path));

  BOOST_CHECK(ErrorCode::kErrorDeviceUnknown == Load(device_path));
}

BOOST_AUTO_TEST_SUITE_END()
