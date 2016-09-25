#include <string>

#include "tests/filesystem_fixtures.h"

#include "fs/limits.h"

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(DirectoryOperationsTestSuite)

using namespace fs;

namespace {

// Test suite parameters:
constexpr int kMany = 100;  // For tests with loops, "many" means this value.

template <typename T>
std::string to_s(T t) {
  return std::to_string(t);
}

}  // namespace

BOOST_FIXTURE_TEST_CASE(create_one_dir, LoadedFSFixture) {
  BOOST_CHECK(ErrorCode::kSuccess == CreateDirectory("home"));
}

BOOST_FIXTURE_TEST_CASE(create_many_dirs, LoadedFSFixture) {
  for (int i = 0; i < kMany; ++i)
    BOOST_CHECK(ErrorCode::kSuccess == CreateDirectory(to_s(i)));
}

BOOST_FIXTURE_TEST_CASE(create_dir_with_one_dir, LoadedFSFixture) {
  BOOST_REQUIRE(ErrorCode::kSuccess == CreateDirectory("home"));

  BOOST_CHECK(ErrorCode::kSuccess == CreateDirectory("home/kitchen"));
}

BOOST_FIXTURE_TEST_CASE(create_dir_with_many_dirs, LoadedFSFixture) {
  BOOST_REQUIRE(ErrorCode::kSuccess == CreateDirectory("home"));

  for (int i = 0; i < kMany; ++i)
    BOOST_CHECK(ErrorCode::kSuccess == CreateDirectory("home/" + to_s(i)));
}

BOOST_FIXTURE_TEST_CASE(create_dir_with_complex_name, LoadedFSFixture) {
  BOOST_CHECK(ErrorCode::kSuccess ==
              CreateDirectory("Hi Tom! These characters are allowed: "
                              "a-Z0+9@#$%^&*(){}[]|_=:;.,'\""));
}

BOOST_FIXTURE_TEST_CASE(create_dir_with_long_name, LoadedFSFixture) {
  BOOST_CHECK(ErrorCode::kSuccess ==
              CreateDirectory(std::string(kNameMax, 'a')));
}

BOOST_FIXTURE_TEST_CASE(create_dir_with_long_path, LoadedFSFixture) {
  std::string path(kNameMax, 'a');
  while (path.size() < kPathMax) path += "/" + path;
  path.resize(kPathMax);

  BOOST_CHECK(ErrorCode::kErrorNotFound == CreateDirectory(path));
}

BOOST_FIXTURE_TEST_CASE(create_dir_with_illegal_name, LoadedFSFixture) {
  // The / symbol is forbidden but there it is handled as the " is not allowed"
  // subdirectory in the root directory.
  BOOST_CHECK(ErrorCode::kSuccess == CreateDirectory("/ is not allowed"));
}

BOOST_FIXTURE_TEST_CASE(create_dir_with_too_long_name, LoadedFSFixture) {
  BOOST_CHECK(ErrorCode::kErrorNameTooLong ==
              CreateDirectory(std::string(kNameMax + 1, 'a')));
}

BOOST_FIXTURE_TEST_CASE(create_dir_with_too_long_path, LoadedFSFixture) {
  std::string path(kNameMax, 'a');
  while (path.size() < kPathMax + 1) path += "/" + path;
  path.resize(kPathMax + 1);

  BOOST_CHECK(ErrorCode::kErrorNameTooLong == CreateDirectory(path));
}

BOOST_FIXTURE_TEST_CASE(create_dir_if_dir_exists, LoadedFSFixture) {
  BOOST_REQUIRE(ErrorCode::kSuccess == CreateDirectory("home"));

  BOOST_CHECK(ErrorCode::kErrorExists == CreateDirectory("home"));
}

BOOST_FIXTURE_TEST_CASE(create_dir_if_file_exists, LoadedFSFixture) {
  BOOST_REQUIRE(ErrorCode::kSuccess == CreateFile("file_or_directory"));

  BOOST_CHECK(ErrorCode::kErrorExists == CreateDirectory("file_or_directory"));
}

BOOST_FIXTURE_TEST_CASE(create_dir_is_case_sensitive, LoadedFSFixture) {
  BOOST_REQUIRE(ErrorCode::kSuccess == CreateDirectory("home"));

  BOOST_CHECK(ErrorCode::kSuccess == CreateDirectory("Home"));
}

BOOST_FIXTURE_TEST_CASE(create_sub_dir_for_file, LoadedFSFixture) {
  BOOST_REQUIRE(ErrorCode::kSuccess == CreateFile(".profile"));

  BOOST_CHECK(ErrorCode::kErrorNotDirectory ==
              CreateDirectory(".profile/home"));
}

BOOST_FIXTURE_TEST_CASE(remove_one_dir, LoadedFSFixture) {
  BOOST_REQUIRE(ErrorCode::kSuccess == CreateDirectory("home"));

  BOOST_CHECK(ErrorCode::kSuccess == RemoveDirectory("home"));
}

BOOST_FIXTURE_TEST_CASE(remove_many_dirs, LoadedFSFixture) {
  for (int i = 0; i < kMany; ++i)
    BOOST_REQUIRE(ErrorCode::kSuccess == CreateDirectory(to_s(i)));

  for (int i = 0; i < kMany; ++i)
    BOOST_CHECK(ErrorCode::kSuccess == RemoveDirectory(to_s(i)));
}

BOOST_FIXTURE_TEST_CASE(remove_one_dir_in_dir, LoadedFSFixture) {
  BOOST_REQUIRE(ErrorCode::kSuccess == CreateDirectory("home"));
  BOOST_REQUIRE(ErrorCode::kSuccess == CreateDirectory("home/user"));

  BOOST_CHECK(ErrorCode::kSuccess == RemoveDirectory("home/user"));
}

BOOST_FIXTURE_TEST_CASE(remove_many_dirs_in_dir, LoadedFSFixture) {
  BOOST_REQUIRE(ErrorCode::kSuccess == CreateDirectory("home"));
  for (int i = 0; i < kMany; ++i)
    BOOST_REQUIRE(ErrorCode::kSuccess == CreateDirectory("home/" + to_s(i)));

  for (int i = 0; i < kMany; ++i)
    BOOST_CHECK(ErrorCode::kSuccess == RemoveDirectory("home/" + to_s(i)));
}

BOOST_FIXTURE_TEST_CASE(remove_dir_with_dir, LoadedFSFixture) {
  BOOST_REQUIRE(ErrorCode::kSuccess == CreateDirectory("home"));
  BOOST_REQUIRE(ErrorCode::kSuccess == CreateDirectory("home/user"));

  BOOST_CHECK(ErrorCode::kErrorDirectoryNotEmpty == RemoveDirectory("home"));
}

BOOST_FIXTURE_TEST_CASE(remove_dir_with_file, LoadedFSFixture) {
  BOOST_REQUIRE(ErrorCode::kSuccess == CreateDirectory("home"));
  BOOST_REQUIRE(ErrorCode::kSuccess == CreateFile("home/.profile"));

  BOOST_CHECK(ErrorCode::kErrorDirectoryNotEmpty == RemoveDirectory("home"));
}

BOOST_FIXTURE_TEST_CASE(remove_dir_if_doesnt_exist, LoadedFSFixture) {
  BOOST_CHECK(ErrorCode::kErrorNotFound == RemoveDirectory("home"));
}

BOOST_FIXTURE_TEST_CASE(remove_dir_is_case_sensitive, LoadedFSFixture) {
  BOOST_REQUIRE(ErrorCode::kSuccess == CreateDirectory("home"));

  BOOST_CHECK(ErrorCode::kErrorNotFound == RemoveDirectory("Home"));
  BOOST_CHECK(ErrorCode::kSuccess == RemoveDirectory("home"));
}

BOOST_FIXTURE_TEST_CASE(remove_dir_if_already_removed, LoadedFSFixture) {
  BOOST_REQUIRE(ErrorCode::kSuccess == CreateDirectory("home"));
  BOOST_REQUIRE(ErrorCode::kSuccess == RemoveDirectory("home"));

  BOOST_CHECK(ErrorCode::kErrorNotFound == RemoveDirectory("home"));
}

BOOST_FIXTURE_TEST_CASE(list_empty_dir, LoadedFSFixture) {
  std::vector<std::string> contents;
  BOOST_CHECK(ErrorCode::kSuccess == ListDirectory("/", contents));
  BOOST_CHECK(contents == std::vector<std::string>{});
}

BOOST_FIXTURE_TEST_CASE(list_dir_with_one_dir, LoadedFSFixture) {
  BOOST_REQUIRE(ErrorCode::kSuccess == CreateDirectory("home"));

  std::vector<std::string> contents;
  BOOST_CHECK(ErrorCode::kSuccess == ListDirectory("/", contents));
  BOOST_CHECK(contents == std::vector<std::string>{"home"});
}

BOOST_FIXTURE_TEST_CASE(list_dir_with_many_dirs, LoadedFSFixture) {
  std::vector<std::string> expected;
  for (int i = 0; i < kMany; ++i) {
    expected.push_back(std::to_string(i));
    BOOST_REQUIRE(ErrorCode::kSuccess == CreateDirectory(to_s(i)));
  }

  std::vector<std::string> contents;
  BOOST_CHECK(ErrorCode::kSuccess == ListDirectory("/", contents));
  BOOST_CHECK(contents == expected);
}

BOOST_FIXTURE_TEST_CASE(list_dir_with_one_dir_with_dir, LoadedFSFixture) {
  BOOST_REQUIRE(ErrorCode::kSuccess == CreateDirectory("home"));
  BOOST_REQUIRE(ErrorCode::kSuccess == CreateDirectory("home/user"));

  std::vector<std::string> contents;
  BOOST_CHECK(ErrorCode::kSuccess == ListDirectory("/", contents));
  BOOST_CHECK(contents == std::vector<std::string>{"home"});
}

BOOST_FIXTURE_TEST_CASE(list_sub_dir_with_one_dir, LoadedFSFixture) {
  BOOST_REQUIRE(ErrorCode::kSuccess == CreateDirectory("home"));
  BOOST_REQUIRE(ErrorCode::kSuccess == CreateDirectory("home/user"));

  std::vector<std::string> contents;
  BOOST_CHECK(ErrorCode::kSuccess == ListDirectory("/home", contents));
  BOOST_CHECK(contents == std::vector<std::string>{"user"});
}

BOOST_FIXTURE_TEST_CASE(list_sub_dir_with_many_dirs, LoadedFSFixture) {
  BOOST_REQUIRE(ErrorCode::kSuccess == CreateDirectory("home"));
  std::vector<std::string> expected;
  for (int i = 0; i < kMany; ++i) {
    expected.push_back(std::to_string(i));
    BOOST_REQUIRE(ErrorCode::kSuccess == CreateDirectory("home/" + to_s(i)));
  }

  std::vector<std::string> contents;
  BOOST_CHECK(ErrorCode::kSuccess == ListDirectory("/home", contents));
  BOOST_CHECK(contents == expected);
}

BOOST_FIXTURE_TEST_CASE(list_dir_with_one_dir_that_was_removed,
                        LoadedFSFixture) {
  BOOST_REQUIRE(ErrorCode::kSuccess == CreateDirectory("home"));
  BOOST_REQUIRE(ErrorCode::kSuccess == RemoveDirectory("home"));

  std::vector<std::string> contents;
  BOOST_CHECK(ErrorCode::kSuccess == ListDirectory("/", contents));
  BOOST_CHECK(contents == std::vector<std::string>{});
}

BOOST_FIXTURE_TEST_CASE(list_dir_if_doesnt_exist, LoadedFSFixture) {
  std::vector<std::string> contents;
  BOOST_CHECK(ErrorCode::kErrorNotFound ==
              ListDirectory("path doesn't exist", contents));
}

BOOST_FIXTURE_TEST_CASE(list_dir_with_many_dirs_while_adding_new_ones,
                        LoadedFSFixture) {
  // 0. create directories 0 and 1
  for (int i = 0; i < 2; ++i)
    BOOST_REQUIRE(ErrorCode::kSuccess == CreateDirectory(to_s(i)));

  // 1. create directory iterator
  FilesystemInterface::DirectoryIterator it = fs->ListDirectory("/", ec), end;
  BOOST_CHECK(ec == ErrorCode::kSuccess);
  BOOST_CHECK(it != end);

  // 2. create a new directory 2
  BOOST_REQUIRE(ErrorCode::kSuccess == CreateDirectory("2"));

  // 3. it points to 0
  BOOST_CHECK(*it++ == to_s(0));
  BOOST_CHECK(ec == ErrorCode::kSuccess);
  BOOST_CHECK(it != end);

  // 4. create a new directory 3
  BOOST_REQUIRE(ErrorCode::kSuccess == CreateDirectory("3"));

  // 5. it points to 1
  BOOST_CHECK(*it++ == to_s(1));
  BOOST_CHECK(ec == ErrorCode::kSuccess);
  BOOST_CHECK(it != end);

  // 6. create a new directory 4
  BOOST_REQUIRE(ErrorCode::kSuccess == CreateDirectory("4"));

  // 7. it points to 2
  BOOST_CHECK(*it++ == to_s(2));
  BOOST_CHECK(ec == ErrorCode::kSuccess);
  BOOST_CHECK(it != end);

  // 8-9. it points to 3, and then 4
  BOOST_CHECK(*it++ == to_s(3));
  BOOST_CHECK(ec == ErrorCode::kSuccess);
  BOOST_CHECK(it != end);
  BOOST_CHECK(*it++ == to_s(4));
  BOOST_CHECK(ec == ErrorCode::kSuccess);

  // 10. it points to end
  BOOST_CHECK(it == end);
}

BOOST_FIXTURE_TEST_CASE(list_dir_with_many_dirs_while_removing_them,
                        LoadedFSFixture) {
  // 0. create directories 0, 1, 2
  for (int i = 0; i < 3; ++i)
    BOOST_REQUIRE(ErrorCode::kSuccess == CreateDirectory(to_s(i)));

  // 1. create directory iterator
  FilesystemInterface::DirectoryIterator it = fs->ListDirectory("/", ec), end;
  BOOST_CHECK(ec == ErrorCode::kSuccess);
  BOOST_CHECK(it != end);

  // 2. remove directory 1
  BOOST_REQUIRE(ErrorCode::kSuccess == RemoveDirectory("1"));

  // 3. it points to 0
  BOOST_CHECK(*it++ == to_s(0));
  BOOST_CHECK(ec == ErrorCode::kSuccess);
  BOOST_CHECK(it != end);

  // 4. remove directory 0
  BOOST_REQUIRE(ErrorCode::kSuccess == RemoveDirectory("0"));

  // 5. it points to 2
  BOOST_CHECK(*it++ == to_s(2));
  BOOST_CHECK(ec == ErrorCode::kSuccess);

  // 6. it points to end
  BOOST_CHECK(it == end);
}

BOOST_AUTO_TEST_SUITE_END()
