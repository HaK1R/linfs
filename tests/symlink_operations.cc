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

BOOST_FIXTURE_TEST_CASE(create_symlink_to_target_with_empty_relative_name, LoadedFSFixture) {
  BOOST_CHECK(ErrorCode::kSuccess == CreateSymlink("lnk", ""));
}

BOOST_FIXTURE_TEST_CASE(create_symlink_to_target_with_empty_absolute_name, LoadedFSFixture) {
  BOOST_CHECK(ErrorCode::kSuccess == CreateSymlink("lnk", "/"));
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

BOOST_FIXTURE_TEST_CASE(create_symlink_with_empty_relative_name, LoadedFSFixture) {
  BOOST_CHECK(ErrorCode::kErrorNotFound == CreateSymlink("", "target"));
}

BOOST_FIXTURE_TEST_CASE(create_symlink_with_empty_absolute_name, LoadedFSFixture) {
  BOOST_CHECK(ErrorCode::kErrorNotFound == CreateSymlink("/", "target"));
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

BOOST_FIXTURE_TEST_CASE(create_self_recursive_symlink, LoadedFSFixture) {
  BOOST_CHECK(ErrorCode::kSuccess == CreateSymlink("lnk", "lnk"));
}

BOOST_FIXTURE_TEST_CASE(create_not_self_recursive_symlink, LoadedFSFixture) {
  BOOST_REQUIRE(ErrorCode::kSuccess == CreateSymlink("lnk", "target"));

  BOOST_CHECK(ErrorCode::kSuccess == CreateSymlink("target", "lnk"));
}

BOOST_FIXTURE_TEST_CASE(create_symlink_if_basename_is_recursive_symlink, LoadedFSFixture) {
  BOOST_REQUIRE(ErrorCode::kSuccess == CreateSymlink("lnk", "lnk"));

  BOOST_CHECK(ErrorCode::kErrorExists == CreateSymlink("lnk", "target"));
}

BOOST_FIXTURE_TEST_CASE(create_symlink_if_dirname_is_recursive_symlink, LoadedFSFixture) {
  BOOST_REQUIRE(ErrorCode::kSuccess == CreateSymlink("home", "home"));

  BOOST_CHECK(ErrorCode::kErrorSymlinkDepth == CreateSymlink("home/lnk", "target"));
}

BOOST_FIXTURE_TEST_CASE(remove_symlink_if_target_doesnt_exist, LoadedFSFixture) {
  BOOST_REQUIRE(ErrorCode::kSuccess == CreateSymlink("lnk", "target"));

  BOOST_CHECK(ErrorCode::kSuccess == Remove("lnk"));
}

BOOST_FIXTURE_TEST_CASE(remove_symlink_to_file_which_exists, LoadedFSFixture) {
  BOOST_REQUIRE(ErrorCode::kSuccess == CreateFile(".profile"));
  BOOST_REQUIRE(ErrorCode::kSuccess == CreateSymlink("lnk", ".profile"));

  BOOST_CHECK(ErrorCode::kSuccess == Remove("lnk"));
  BOOST_CHECK(ErrorCode::kErrorExists == OpenFile(".profile", file, true));
}

BOOST_FIXTURE_TEST_CASE(remove_symlink_to_file_that_was_removed, LoadedFSFixture) {
  BOOST_REQUIRE(ErrorCode::kSuccess == CreateFile(".profile"));
  BOOST_REQUIRE(ErrorCode::kSuccess == CreateSymlink("lnk", ".profile"));
  BOOST_REQUIRE(ErrorCode::kSuccess == Remove(".profile"));

  BOOST_CHECK(ErrorCode::kSuccess == Remove("lnk"));
}

BOOST_FIXTURE_TEST_CASE(remove_symlink_to_dir_which_exists, LoadedFSFixture) {
  BOOST_REQUIRE(ErrorCode::kSuccess == CreateDirectory("home"));
  BOOST_REQUIRE(ErrorCode::kSuccess == CreateSymlink("lnk", "home"));

  BOOST_CHECK(ErrorCode::kSuccess == Remove("lnk"));
  BOOST_CHECK(ErrorCode::kErrorExists == CreateDirectory("home"));
}

BOOST_FIXTURE_TEST_CASE(remove_symlink_to_dir_that_was_removed, LoadedFSFixture) {
  BOOST_REQUIRE(ErrorCode::kSuccess == CreateDirectory("home"));
  BOOST_REQUIRE(ErrorCode::kSuccess == CreateSymlink("lnk", "home"));
  BOOST_REQUIRE(ErrorCode::kSuccess == Remove("home"));

  BOOST_CHECK(ErrorCode::kSuccess == Remove("lnk"));
}

BOOST_FIXTURE_TEST_CASE(remove_many_symlinks_to_one_target, LoadedFSFixture) {
  for (int i = 0; i < kMany; ++i)
    BOOST_REQUIRE(ErrorCode::kSuccess == CreateSymlink(std::to_string(i), "target"));

  for (int i = 0; i < kMany; ++i)
    BOOST_CHECK(ErrorCode::kSuccess == Remove(std::to_string(i)));
}

BOOST_FIXTURE_TEST_CASE(remove_many_symlinks_to_many_targets, LoadedFSFixture) {
  for (int i = 0; i < kMany; ++i)
    BOOST_REQUIRE(ErrorCode::kSuccess == CreateSymlink(std::to_string(i), "target" + std::to_string(i)));

  for (int i = 0; i < kMany; ++i)
    BOOST_CHECK(ErrorCode::kSuccess == Remove(std::to_string(i)));
}

BOOST_FIXTURE_TEST_CASE(remove_symlink_is_case_sensitive, LoadedFSFixture) {
  BOOST_REQUIRE(ErrorCode::kSuccess == CreateSymlink("lnk", "target"));

  BOOST_CHECK(ErrorCode::kErrorNotFound == Remove("Lnk"));
  BOOST_CHECK(ErrorCode::kSuccess == Remove("lnk"));
}

BOOST_FIXTURE_TEST_CASE(remove_symlink_in_sub_dir, LoadedFSFixture) {
  BOOST_REQUIRE(ErrorCode::kSuccess == CreateDirectory("home"));
  BOOST_REQUIRE(ErrorCode::kSuccess == CreateSymlink("home/lnk", "target"));

  BOOST_CHECK(ErrorCode::kSuccess == Remove("home/lnk"));
}

BOOST_FIXTURE_TEST_CASE(remove_recursive_symlink, LoadedFSFixture) {
  BOOST_REQUIRE(ErrorCode::kSuccess == CreateSymlink("lnk", "lnk"));

  BOOST_CHECK(ErrorCode::kSuccess == Remove("lnk"));
}

BOOST_FIXTURE_TEST_CASE(remove_symlink_if_dirname_is_recursive_symlink, LoadedFSFixture) {
  BOOST_REQUIRE(ErrorCode::kSuccess == CreateSymlink("home", "home"));

  BOOST_CHECK(ErrorCode::kErrorSymlinkDepth == Remove("home/lnk"));
}

BOOST_FIXTURE_TEST_CASE(remove_symlink_if_already_removed, LoadedFSFixture) {
  BOOST_REQUIRE(ErrorCode::kSuccess == CreateSymlink("lnk", "target"));
  BOOST_REQUIRE(ErrorCode::kSuccess == Remove("lnk"));

  BOOST_CHECK(ErrorCode::kErrorNotFound == Remove("Lnk"));
}

BOOST_FIXTURE_TEST_CASE(is_symlink_for_symlink, LoadedFSFixture) {
  BOOST_REQUIRE(ErrorCode::kSuccess == CreateSymlink("lnk", "target"));

  BOOST_CHECK(fs->IsSymlink("lnk", &ec));
  BOOST_CHECK(ErrorCode::kSuccess == ec);
}

BOOST_FIXTURE_TEST_CASE(is_symlink_for_dir, LoadedFSFixture) {
  BOOST_REQUIRE(ErrorCode::kSuccess == CreateDirectory("home"));

  BOOST_CHECK(!fs->IsSymlink("home", &ec));
  BOOST_CHECK(ErrorCode::kSuccess == ec);
}

BOOST_FIXTURE_TEST_CASE(is_symlink_for_file, LoadedFSFixture) {
  BOOST_REQUIRE(ErrorCode::kSuccess == CreateFile(".profile"));

  BOOST_CHECK(!fs->IsSymlink(".profile", &ec));
  BOOST_CHECK(ErrorCode::kSuccess == ec);
}

BOOST_FIXTURE_TEST_CASE(is_symlink_if_doesnt_exist, LoadedFSFixture) {
  BOOST_CHECK(!fs->IsSymlink("lnk", &ec));
  BOOST_CHECK(ErrorCode::kErrorNotFound == ec);
}

BOOST_AUTO_TEST_SUITE_END()
