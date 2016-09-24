#include <string>

#include "tests/filesystem_fixtures.h"

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(DirectoryOperationsTestSuite)

using fs::ErrorCode;
using fs::FileInterface;
using fs::FilesystemInterface;

BOOST_FIXTURE_TEST_CASE(create_one_dir, LoadedFSFixture) {
  BOOST_CHECK_NO_THROW(ec = fs->CreateDirectory("home"));
  BOOST_CHECK(ec == ErrorCode::kSuccess);
}

BOOST_FIXTURE_TEST_CASE(create_many_dirs, LoadedFSFixture) {
  for (int i = 0; i < 100; ++i) {
    BOOST_CHECK_NO_THROW(ec = fs->CreateDirectory(std::to_string(i).c_str()));
    BOOST_CHECK(ec == ErrorCode::kSuccess);
  }
}

BOOST_FIXTURE_TEST_CASE(create_dir_with_one_dir, LoadedFSFixture) {
  BOOST_REQUIRE_NO_THROW(ec = fs->CreateDirectory("home"));
  BOOST_REQUIRE(ec == ErrorCode::kSuccess);

  BOOST_CHECK_NO_THROW(ec = fs->CreateDirectory("home/kitchen"));
  BOOST_CHECK(ec == ErrorCode::kSuccess);
}

BOOST_FIXTURE_TEST_CASE(create_dir_with_many_dirs, LoadedFSFixture) {
  BOOST_REQUIRE_NO_THROW(ec = fs->CreateDirectory("home"));
  BOOST_REQUIRE(ec == ErrorCode::kSuccess);

  for (int i = 0; i < 100; ++i) {
    BOOST_CHECK_NO_THROW(ec = fs->CreateDirectory((std::string("home/") + std::to_string(i)).c_str()));
    BOOST_CHECK(ec == ErrorCode::kSuccess);
  }
}

BOOST_FIXTURE_TEST_CASE(create_dir_with_complex_name, LoadedFSFixture) {
  BOOST_CHECK_NO_THROW(ec = fs->CreateDirectory("Hi Tom! These characters are allowed: a-Z0+9@#$%^&*(){}[]|_=:;.,'\""));
  BOOST_CHECK(ec == ErrorCode::kSuccess);
}

BOOST_FIXTURE_TEST_CASE(create_dir_with_long_name, LoadedFSFixture) {
  BOOST_CHECK_NO_THROW(ec = fs->CreateDirectory(std::string(fs::kNameMax, 'a').c_str()));
  BOOST_CHECK(ec == ErrorCode::kSuccess);
}

BOOST_FIXTURE_TEST_CASE(create_dir_with_long_path, LoadedFSFixture) {
  std::string path(fs::kNameMax, 'a');
  while (path.size() < fs::kPathMax)
    path += "/" + path;
  path.resize(fs::kPathMax);

  BOOST_CHECK_NO_THROW(ec = fs->CreateDirectory(path.c_str()));
  BOOST_CHECK(ec != ErrorCode::kErrorNameTooLong);
}

BOOST_FIXTURE_TEST_CASE(create_dir_with_illegal_name, LoadedFSFixture) {
  // The / is forbidden but there it is handled as " is not allowed" directory in the root.
  BOOST_CHECK_NO_THROW(ec = fs->CreateDirectory("/ is not allowed"));
  BOOST_CHECK(ec == ErrorCode::kSuccess);
}

BOOST_FIXTURE_TEST_CASE(create_dir_with_too_long_name, LoadedFSFixture) {
  BOOST_CHECK_NO_THROW(ec = fs->CreateDirectory(std::string(fs::kNameMax + 1, 'a').c_str()));
  BOOST_CHECK(ec == ErrorCode::kErrorNameTooLong);
}

BOOST_FIXTURE_TEST_CASE(create_dir_with_too_long_path, LoadedFSFixture) {
  std::string path(fs::kNameMax, 'a');
  while (path.size() < fs::kPathMax + 1)
    path += "/" + path;
  path.resize(fs::kPathMax + 1);

  BOOST_CHECK_NO_THROW(ec = fs->CreateDirectory(path.c_str()));
  BOOST_CHECK(ec == ErrorCode::kErrorNameTooLong);
}

BOOST_FIXTURE_TEST_CASE(create_dir_if_dir_exists, LoadedFSFixture) {
  BOOST_REQUIRE_NO_THROW(ec = fs->CreateDirectory("home"));
  BOOST_REQUIRE(ec == ErrorCode::kSuccess);

  BOOST_CHECK_NO_THROW(ec = fs->CreateDirectory("home"));
  BOOST_CHECK(ec == ErrorCode::kErrorExists);
}

BOOST_FIXTURE_TEST_CASE(create_dir_if_file_exists, LoadedFSFixture) {
  BOOST_REQUIRE_NO_THROW(file = fs->OpenFile("file_or_directory", ec));
  BOOST_REQUIRE(ec == ErrorCode::kSuccess);
  BOOST_REQUIRE_NO_THROW(file->Close());

  BOOST_CHECK_NO_THROW(ec = fs->CreateDirectory("file_or_directory"));
  BOOST_CHECK(ec == ErrorCode::kErrorExists);
}

BOOST_FIXTURE_TEST_CASE(create_dir_is_case_sensitive, LoadedFSFixture) {
  BOOST_REQUIRE_NO_THROW(ec = fs->CreateDirectory("home"));
  BOOST_REQUIRE(ec == ErrorCode::kSuccess);

  BOOST_CHECK_NO_THROW(ec = fs->CreateDirectory("Home"));
  BOOST_CHECK(ec == ErrorCode::kSuccess);
}

BOOST_FIXTURE_TEST_CASE(create_sub_dir_for_file, LoadedFSFixture) {
  BOOST_REQUIRE_NO_THROW(file = fs->OpenFile(".profile", ec));
  BOOST_REQUIRE(ec == ErrorCode::kSuccess);
  BOOST_REQUIRE_NO_THROW(file->Close());

  BOOST_CHECK_NO_THROW(ec = fs->CreateDirectory(".profile/home"));
  BOOST_CHECK(ec == ErrorCode::kErrorNotDirectory);
}

BOOST_FIXTURE_TEST_CASE(remove_one_dir, LoadedFSFixture) {
  BOOST_REQUIRE_NO_THROW(ec = fs->CreateDirectory("home"));
  BOOST_REQUIRE(ec == ErrorCode::kSuccess);

  BOOST_CHECK_NO_THROW(ec = fs->RemoveDirectory("home"));
  BOOST_CHECK(ec == ErrorCode::kSuccess);
}

BOOST_FIXTURE_TEST_CASE(remove_many_dirs, LoadedFSFixture) {
  for (int i = 0; i < 100; ++i) {
    BOOST_REQUIRE_NO_THROW(ec = fs->CreateDirectory(std::to_string(i).c_str()));
    BOOST_REQUIRE(ec == ErrorCode::kSuccess);
  }

  for (int i = 0; i < 100; ++i) {
    BOOST_CHECK_NO_THROW(ec = fs->RemoveDirectory(std::to_string(i).c_str()));
    BOOST_CHECK(ec == ErrorCode::kSuccess);
  }
}

BOOST_FIXTURE_TEST_CASE(remove_one_dir_in_dir, LoadedFSFixture) {
  BOOST_REQUIRE_NO_THROW(ec = fs->CreateDirectory("home"));
  BOOST_REQUIRE(ec == ErrorCode::kSuccess);
  BOOST_REQUIRE_NO_THROW(ec = fs->CreateDirectory("home/user"));
  BOOST_REQUIRE(ec == ErrorCode::kSuccess);

  BOOST_CHECK_NO_THROW(ec = fs->RemoveDirectory("home/user"));
  BOOST_CHECK(ec == ErrorCode::kSuccess);
}

BOOST_FIXTURE_TEST_CASE(remove_many_dirs_in_dir, LoadedFSFixture) {
  BOOST_REQUIRE_NO_THROW(ec = fs->CreateDirectory("home"));
  BOOST_REQUIRE(ec == ErrorCode::kSuccess);
  for (int i = 0; i < 100; ++i) {
    BOOST_REQUIRE_NO_THROW(ec = fs->CreateDirectory((std::string("home/") + std::to_string(i)).c_str()));
    BOOST_REQUIRE(ec == ErrorCode::kSuccess);
  }

  for (int i = 0; i < 100; ++i) {
    BOOST_CHECK_NO_THROW(ec = fs->RemoveDirectory((std::string("home/") + std::to_string(i)).c_str()));
    BOOST_CHECK(ec == ErrorCode::kSuccess);
  }
}

BOOST_FIXTURE_TEST_CASE(remove_dir_with_dir, LoadedFSFixture) {
  BOOST_REQUIRE_NO_THROW(ec = fs->CreateDirectory("home"));
  BOOST_REQUIRE(ec == ErrorCode::kSuccess);
  BOOST_REQUIRE_NO_THROW(ec = fs->CreateDirectory("home/user"));
  BOOST_REQUIRE(ec == ErrorCode::kSuccess);

  BOOST_CHECK_NO_THROW(ec = fs->RemoveDirectory("home"));
  BOOST_CHECK(ec == ErrorCode::kErrorDirectoryNotEmpty);
}

BOOST_FIXTURE_TEST_CASE(remove_dir_with_file, LoadedFSFixture) {
  BOOST_REQUIRE_NO_THROW(ec = fs->CreateDirectory("home"));
  BOOST_REQUIRE(ec == ErrorCode::kSuccess);
  BOOST_REQUIRE_NO_THROW(file = fs->OpenFile("home/.profile", ec));
  BOOST_REQUIRE(ec == ErrorCode::kSuccess);
  BOOST_REQUIRE_NO_THROW(file->Close());

  BOOST_CHECK_NO_THROW(ec = fs->RemoveDirectory("home"));
  BOOST_CHECK(ec == ErrorCode::kErrorDirectoryNotEmpty);
}

BOOST_FIXTURE_TEST_CASE(remove_dir_if_doesnt_exist, LoadedFSFixture) {
  BOOST_CHECK_NO_THROW(ec = fs->RemoveDirectory("home"));
  BOOST_CHECK(ec == ErrorCode::kErrorNotFound);
}

BOOST_FIXTURE_TEST_CASE(remove_dir_is_case_sensitive, LoadedFSFixture) {
  BOOST_REQUIRE_NO_THROW(ec = fs->CreateDirectory("home"));
  BOOST_REQUIRE(ec == ErrorCode::kSuccess);

  BOOST_CHECK_NO_THROW(ec = fs->RemoveDirectory("Home"));
  BOOST_CHECK(ec == ErrorCode::kErrorNotFound);

  BOOST_CHECK_NO_THROW(ec = fs->RemoveDirectory("home"));
  BOOST_CHECK(ec == ErrorCode::kSuccess);
}

BOOST_FIXTURE_TEST_CASE(remove_dir_if_already_removed, LoadedFSFixture) {
  BOOST_REQUIRE_NO_THROW(ec = fs->CreateDirectory("home"));
  BOOST_REQUIRE(ec == ErrorCode::kSuccess);
  BOOST_REQUIRE_NO_THROW(ec = fs->RemoveDirectory("home"));
  BOOST_REQUIRE(ec == ErrorCode::kSuccess);

  BOOST_CHECK_NO_THROW(ec = fs->RemoveDirectory("home"));
  BOOST_CHECK(ec == ErrorCode::kErrorNotFound);
}

void CheckDirectoryContent(DefaultFSFixture* fxt, const char* dir_name,
                           std::vector<std::string> entries) {
  FilesystemInterface::DirectoryIterator it, end;
  BOOST_CHECK_NO_THROW(it = fxt->fs->ListDirectory(dir_name, fxt->ec));
  BOOST_CHECK(fxt->ec == ErrorCode::kSuccess);
  for (auto entry : entries) {
    BOOST_CHECK(*it == entry);
    BOOST_CHECK_NO_THROW(++it);
    BOOST_CHECK(fxt->ec == ErrorCode::kSuccess);
  }
  BOOST_CHECK(it == end);
}

BOOST_FIXTURE_TEST_CASE(list_empty_dir, LoadedFSFixture) {
  CheckDirectoryContent(this, "/", {});
}

BOOST_FIXTURE_TEST_CASE(list_dir_with_one_dir, LoadedFSFixture) {
  BOOST_REQUIRE_NO_THROW(ec = fs->CreateDirectory("home"));
  BOOST_REQUIRE(ec == ErrorCode::kSuccess);

  CheckDirectoryContent(this, "/", {"home"});
}

BOOST_FIXTURE_TEST_CASE(list_dir_with_many_dirs, LoadedFSFixture) {
  std::vector<std::string> contents;
  for (int i = 0; i < 100; ++i) {
    contents.push_back(std::to_string(i));
    BOOST_CHECK_NO_THROW(ec = fs->CreateDirectory(contents.back().c_str()));
    BOOST_CHECK(ec == ErrorCode::kSuccess);
  }

  CheckDirectoryContent(this, "/", contents);
}

BOOST_FIXTURE_TEST_CASE(list_dir_with_one_dir_with_dir, LoadedFSFixture) {
  BOOST_REQUIRE_NO_THROW(ec = fs->CreateDirectory("home"));
  BOOST_REQUIRE(ec == ErrorCode::kSuccess);
  BOOST_REQUIRE_NO_THROW(ec = fs->CreateDirectory("home/user"));
  BOOST_REQUIRE(ec == ErrorCode::kSuccess);

  CheckDirectoryContent(this, "/", {"home"});
}

BOOST_FIXTURE_TEST_CASE(list_sub_dir_with_one_dir, LoadedFSFixture) {
  BOOST_REQUIRE_NO_THROW(ec = fs->CreateDirectory("home"));
  BOOST_REQUIRE(ec == ErrorCode::kSuccess);
  BOOST_REQUIRE_NO_THROW(ec = fs->CreateDirectory("home/user"));
  BOOST_REQUIRE(ec == ErrorCode::kSuccess);

  CheckDirectoryContent(this, "/home", {"user"});
}

BOOST_FIXTURE_TEST_CASE(list_sub_dir_with_many_dirs, LoadedFSFixture) {
  BOOST_REQUIRE_NO_THROW(ec = fs->CreateDirectory("home"));
  BOOST_REQUIRE(ec == ErrorCode::kSuccess);
  std::vector<std::string> contents;
  for (int i = 0; i < 100; ++i) {
    contents.push_back(std::to_string(i));
    BOOST_CHECK_NO_THROW(ec = fs->CreateDirectory((std::string("home/") + contents.back()).c_str()));
    BOOST_CHECK(ec == ErrorCode::kSuccess);
  }

  CheckDirectoryContent(this, "/home", contents);
}

BOOST_FIXTURE_TEST_CASE(list_dir_with_one_dir_that_was_removed, LoadedFSFixture) {
  BOOST_REQUIRE_NO_THROW(ec = fs->CreateDirectory("home"));
  BOOST_REQUIRE(ec == ErrorCode::kSuccess);

  BOOST_CHECK_NO_THROW(ec = fs->RemoveDirectory("home"));
  BOOST_CHECK(ec == ErrorCode::kSuccess);
  CheckDirectoryContent(this, "/", {});
}

BOOST_FIXTURE_TEST_CASE(list_dir_if_doesnt_exist, LoadedFSFixture) {
  FilesystemInterface::DirectoryIterator it;
  BOOST_CHECK_NO_THROW(it = fs->ListDirectory("path doesn't exist", ec));
  BOOST_CHECK(ec == ErrorCode::kErrorNotFound);
}

BOOST_FIXTURE_TEST_CASE(list_dir_with_many_dirs_while_adding_new_ones, LoadedFSFixture) {
  // 0. create directories 0 and 1
  for (int i = 0; i < 2; ++i) {
    BOOST_REQUIRE_NO_THROW(ec = fs->CreateDirectory(std::to_string(i).c_str()));
    BOOST_REQUIRE(ec == ErrorCode::kSuccess);
  }

  // 1. create directory iterator
  FilesystemInterface::DirectoryIterator it, end;
  BOOST_CHECK_NO_THROW(it = fs->ListDirectory("/", ec));
  BOOST_CHECK(ec == ErrorCode::kSuccess);

  // 2. create a new directory 2
  BOOST_CHECK_NO_THROW(ec = fs->CreateDirectory("2"));
  BOOST_CHECK(ec == ErrorCode::kSuccess);

  // 3. it points to 0
  BOOST_CHECK(it != end);
  BOOST_CHECK(*it == std::string("0"));
  BOOST_CHECK_NO_THROW(++it);

  // 4. create a new directory 3
  BOOST_CHECK_NO_THROW(ec = fs->CreateDirectory("3"));
  BOOST_CHECK(ec == ErrorCode::kSuccess);

  // 5. it points to 1
  BOOST_CHECK(it != end);
  BOOST_CHECK(*it == std::string("1"));
  BOOST_CHECK_NO_THROW(++it);

  // 6. create a new directory 4
  BOOST_CHECK_NO_THROW(ec = fs->CreateDirectory("4"));
  BOOST_CHECK(ec == ErrorCode::kSuccess);

  // 7. it points to 2
  BOOST_CHECK(it != end);
  BOOST_CHECK(*it == std::string("2"));
  BOOST_CHECK_NO_THROW(++it);

  // 8-9. it points to 3, and then 4
  BOOST_CHECK(it != end);
  BOOST_CHECK(*it == std::string("3"));
  BOOST_CHECK_NO_THROW(++it);
  BOOST_CHECK(it != end);
  BOOST_CHECK(*it == std::string("4"));
  BOOST_CHECK_NO_THROW(++it);

  // 10. it points to end
  BOOST_CHECK(it == end);
}

BOOST_FIXTURE_TEST_CASE(list_dir_with_many_dirs_while_removing_them, LoadedFSFixture) {
  // 0. create directories 0, 1, 2
  for (int i = 0; i < 3; ++i) {
    BOOST_REQUIRE_NO_THROW(ec = fs->CreateDirectory(std::to_string(i).c_str()));
    BOOST_REQUIRE(ec == ErrorCode::kSuccess);
  }

  // 1. create directory iterator
  FilesystemInterface::DirectoryIterator it, end;
  BOOST_CHECK_NO_THROW(it = fs->ListDirectory("/", ec));
  BOOST_CHECK(ec == ErrorCode::kSuccess);

  // 2. remove directory 1
  BOOST_CHECK_NO_THROW(ec = fs->RemoveDirectory("1"));
  BOOST_CHECK(ec == ErrorCode::kSuccess);

  // 3. it points to 0
  BOOST_CHECK(it != end);
  BOOST_CHECK(*it == std::string("0"));
  BOOST_CHECK_NO_THROW(++it);

  // 4. remove directory 0
  BOOST_CHECK_NO_THROW(ec = fs->RemoveDirectory("0"));
  BOOST_CHECK(ec == ErrorCode::kSuccess);

  // 5. it points to 2
  BOOST_CHECK(it != end);
  BOOST_CHECK(*it == std::string("2"));
  BOOST_CHECK_NO_THROW(++it);

  // 6. it points to end
  BOOST_CHECK(it == end);
}

BOOST_AUTO_TEST_SUITE_END()
