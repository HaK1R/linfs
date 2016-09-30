#include <string>

#include "tests/filesystem_fixtures.h"

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(SymlinkOperationsTestSuite)

using namespace fs;

namespace {

// Test suite parameters:
constexpr int kMany = 100;  // For tests with loops, "many" means this value.

}  // namespace

BOOST_FIXTURE_TEST_CASE(create_symlink_if_target_doesnt_exist, LoadedFSFixture) {
  BOOST_CHECK(ErrorCode::kSuccess == CreateSymlink("lnk", "target"));
}

BOOST_FIXTURE_TEST_CASE(create_symlink_to_file_which_exists, LoadedFSFixture) {
  BOOST_REQUIRE(ErrorCode::kSuccess == CreateFile(".profile"));

  BOOST_CHECK(ErrorCode::kSuccess == CreateSymlink("lnk", ".profile"));
}

BOOST_FIXTURE_TEST_CASE(create_symlink_to_dir_which_exists, LoadedFSFixture) {
  BOOST_REQUIRE(ErrorCode::kSuccess == CreateDirectory("home"));

  BOOST_CHECK(ErrorCode::kSuccess == CreateSymlink("lnk", "home"));
}

BOOST_FIXTURE_TEST_CASE(create_symlink_to_target_with_complex_name, LoadedFSFixture) {
  BOOST_CHECK(ErrorCode::kSuccess == CreateSymlink("lnk", "a-Z0+9@#$%^&*(){}[]|_=:;.,'\""));
}

BOOST_FIXTURE_TEST_CASE(create_symlink_to_target_with_long_name, LoadedFSFixture) {
  BOOST_CHECK(ErrorCode::kSuccess == CreateSymlink("lnk", std::string(kNameMax, 'a')));
}

BOOST_FIXTURE_TEST_CASE(create_symlink_to_target_with_too_long_name, LoadedFSFixture) {
  BOOST_CHECK(ErrorCode::kErrorNameTooLong == CreateSymlink("lnk", std::string(kNameMax + 1, 'a')));
}

BOOST_FIXTURE_TEST_CASE(create_many_symlinks_to_one_target, LoadedFSFixture) {
  for (int i = 0; i < kMany; ++i)
    BOOST_CHECK(ErrorCode::kSuccess == CreateSymlink(std::to_string(i), "target"));
}

BOOST_FIXTURE_TEST_CASE(create_many_symlinks_to_many_targets, LoadedFSFixture) {
  for (int i = 0; i < kMany; ++i)
    BOOST_CHECK(ErrorCode::kSuccess == CreateSymlink(std::to_string(i), "target" + std::to_string(i)));
}

BOOST_FIXTURE_TEST_CASE(create_symlink_with_complex_name, LoadedFSFixture) {
  BOOST_CHECK(ErrorCode::kSuccess == CreateSymlink("a-Z0+9@#$%^&*(){}[]|_=:;.,'\"", "target"));
}

BOOST_FIXTURE_TEST_CASE(create_symlink_with_long_name, LoadedFSFixture) {
  BOOST_CHECK(ErrorCode::kSuccess == CreateSymlink(std::string(kNameMax, 'a'), "target"));
}

BOOST_FIXTURE_TEST_CASE(create_symlink_with_too_long_name, LoadedFSFixture) {
  BOOST_CHECK(ErrorCode::kErrorNameTooLong == CreateSymlink(std::string(kNameMax + 1, 'a'), "target"));
}

BOOST_FIXTURE_TEST_CASE(create_symlink_if_dir_exists, LoadedFSFixture) {
  BOOST_REQUIRE(ErrorCode::kSuccess == CreateDirectory("symlink_or_directory"));

  BOOST_CHECK(ErrorCode::kErrorExists == CreateSymlink("symlink_or_directory", "target"));
}

BOOST_FIXTURE_TEST_CASE(create_symlink_if_file_exists, LoadedFSFixture) {
  BOOST_REQUIRE(ErrorCode::kSuccess == CreateFile("symlink_or_file"));

  BOOST_CHECK(ErrorCode::kErrorExists == CreateSymlink("symlink_or_file", "target"));
}

BOOST_FIXTURE_TEST_CASE(create_symlink_if_symlink_exists, LoadedFSFixture) {
  BOOST_REQUIRE(ErrorCode::kSuccess == CreateSymlink("lnk", "target"));

  BOOST_CHECK(ErrorCode::kErrorExists == CreateSymlink("lnk", "target"));
}

BOOST_FIXTURE_TEST_CASE(create_symlink_is_case_sensitive, LoadedFSFixture) {
  BOOST_REQUIRE(ErrorCode::kSuccess == CreateSymlink("lnk", "target"));

  BOOST_CHECK(ErrorCode::kSuccess == CreateSymlink("Lnk", "target"));
}

BOOST_FIXTURE_TEST_CASE(create_symlink_in_sub_dir, LoadedFSFixture) {
  BOOST_REQUIRE(ErrorCode::kSuccess == CreateDirectory("home"));

  BOOST_CHECK(ErrorCode::kSuccess == CreateSymlink("home/lnk", "target"));
}

BOOST_AUTO_TEST_SUITE_END()
