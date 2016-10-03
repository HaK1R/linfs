#include <string>

#include "tests/filesystem_fixtures.h"

#include "fs/limits.h"

#include <boost/filesystem.hpp>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(FileOperationsTestSuite)

using namespace fs;

namespace {

// Test suite parameters:
constexpr int kMany = 100;         // Let's say what does "many" mean.
constexpr size_t k1MB = 1000000;   // Typical file size, and
constexpr size_t k100KB = 100000;  // and 10 times fewer.

template <typename T>
std::string to_s(T t) {
  return std::to_string(t);
}

}  // namespace

BOOST_FIXTURE_TEST_CASE(open_one_file, LoadedFSFixture) {
  BOOST_CHECK(ErrorCode::kSuccess == OpenFile(".profile", file));
}

BOOST_FIXTURE_TEST_CASE(open_one_file_two_times, LoadedFSFixture) {
  ScopedFile file1, file2;
  BOOST_REQUIRE(ErrorCode::kSuccess == OpenFile(".profile", file1));

  BOOST_CHECK(ErrorCode::kSuccess == OpenFile(".profile", file2));
}

BOOST_FIXTURE_TEST_CASE(open_two_files, LoadedFSFixture) {
  ScopedFile file1, file2;
  BOOST_REQUIRE(ErrorCode::kSuccess == OpenFile("1", file1));

  BOOST_CHECK(ErrorCode::kSuccess == OpenFile("2", file2));
}

BOOST_FIXTURE_TEST_CASE(open_many_files, LoadedFSFixture) {
  for (int i = 0; i < kMany; ++i)
    BOOST_CHECK(ErrorCode::kSuccess == OpenFile(to_s(i), file));
}

BOOST_FIXTURE_TEST_CASE(open_file_in_sub_dir, LoadedFSFixture) {
  BOOST_REQUIRE(ErrorCode::kSuccess == CreateDirectory("home"));

  BOOST_CHECK(ErrorCode::kSuccess == OpenFile("home/.profile", file));
}

BOOST_FIXTURE_TEST_CASE(open_many_files_in_sub_dir, LoadedFSFixture) {
  BOOST_REQUIRE(ErrorCode::kSuccess == CreateDirectory("home"));

  for (int i = 0; i < kMany; ++i)
    BOOST_CHECK(ErrorCode::kSuccess == OpenFile("home/" + to_s(i), file));
}

BOOST_FIXTURE_TEST_CASE(open_file_with_complex_name, LoadedFSFixture) {
  BOOST_CHECK(ErrorCode::kSuccess == OpenFile("a-Z0+9@#$%^&*(){}[]|_=:;.,'\"", file));
}

BOOST_FIXTURE_TEST_CASE(open_file_with_long_name, LoadedFSFixture) {
  BOOST_CHECK(ErrorCode::kSuccess == OpenFile(std::string(kNameMax, 'a'), file));
}

BOOST_FIXTURE_TEST_CASE(open_file_with_too_long_name, LoadedFSFixture) {
  BOOST_CHECK(ErrorCode::kErrorNameTooLong == OpenFile(std::string(kNameMax + 1, 'a'), file));
}

BOOST_FIXTURE_TEST_CASE(open_file_if_dir_exists, LoadedFSFixture) {
  BOOST_REQUIRE(ErrorCode::kSuccess == CreateDirectory("file_or_directory"));

  BOOST_CHECK(ErrorCode::kErrorIsDirectory == OpenFile("file_or_directory", file));
}

BOOST_FIXTURE_TEST_CASE(open_file_if_file_exists, LoadedFSFixture) {
  BOOST_REQUIRE(ErrorCode::kSuccess == CreateFile(".profile"));

  BOOST_CHECK(ErrorCode::kSuccess == OpenFile(".profile", file));
}

BOOST_FIXTURE_TEST_CASE(open_file_if_symlink_exists, LoadedFSFixture) {
  BOOST_REQUIRE(ErrorCode::kSuccess == CreateSymlink("file_or_symlink", "target"));

  BOOST_CHECK(ErrorCode::kSuccess == OpenFile("file_or_symlink", file));
}

BOOST_FIXTURE_TEST_CASE(open_file_if_creat_excl, LoadedFSFixture) {
  BOOST_CHECK(ErrorCode::kSuccess == OpenFile(".profile.lock", file, true));
}

BOOST_FIXTURE_TEST_CASE(open_file_if_file_exists_and_creat_excl, LoadedFSFixture) {
  BOOST_REQUIRE(ErrorCode::kSuccess == CreateFile(".profile.lock"));

  BOOST_CHECK(ErrorCode::kErrorExists == OpenFile(".profile.lock", file, true));
}

BOOST_FIXTURE_TEST_CASE(open_file_via_symlink_if_creat_excl, LoadedFSFixture) {
  BOOST_REQUIRE(ErrorCode::kSuccess == CreateSymlink("lnk", ".profile"));

  BOOST_CHECK(ErrorCode::kSuccess == OpenFile("lnk", file, true));
}

BOOST_FIXTURE_TEST_CASE(open_file_if_file_created_via_symlink_and_creat_excl, LoadedFSFixture) {
  BOOST_REQUIRE(ErrorCode::kSuccess == CreateSymlink("lnk", ".profile"));
  BOOST_REQUIRE(ErrorCode::kSuccess == CreateFile("lnk"));

  BOOST_CHECK(ErrorCode::kErrorExists == OpenFile(".profile", file, true));
}

BOOST_FIXTURE_TEST_CASE(open_file_via_symlink_if_file_exists_and_creat_excl, LoadedFSFixture) {
  BOOST_REQUIRE(ErrorCode::kSuccess == CreateSymlink("lnk", ".profile"));
  BOOST_REQUIRE(ErrorCode::kSuccess == CreateFile(".profile"));

  BOOST_CHECK(ErrorCode::kErrorExists == OpenFile("lnk", file, true));
}

BOOST_FIXTURE_TEST_CASE(open_file_via_symlink_target_with_long_name_if_file_exists_and_creat_excl, LoadedFSFixture) {
  BOOST_REQUIRE(ErrorCode::kSuccess == CreateSymlink("lnk", std::string(kNameMax, 'a')));
  BOOST_REQUIRE(ErrorCode::kSuccess == CreateFile(std::string(kNameMax, 'a')));

  BOOST_CHECK(ErrorCode::kErrorExists == OpenFile("lnk", file, true));
}

BOOST_FIXTURE_TEST_CASE(open_file_if_basename_is_self_recursive_symlink, LoadedFSFixture) {
  BOOST_REQUIRE(ErrorCode::kSuccess == CreateSymlink(".profile", ".profile"));

  BOOST_CHECK(ErrorCode::kErrorSymlinkDepth == OpenFile(".profile", file));
}

BOOST_FIXTURE_TEST_CASE(open_file_if_basename_is_not_self_recursive_symlink, LoadedFSFixture) {
  BOOST_REQUIRE(ErrorCode::kSuccess == CreateSymlink("lnk", ".profile"));
  BOOST_REQUIRE(ErrorCode::kSuccess == CreateSymlink(".profile", "lnk"));

  BOOST_CHECK(ErrorCode::kErrorSymlinkDepth == OpenFile(".profile", file));
}

BOOST_FIXTURE_TEST_CASE(open_file_if_dirname_is_recursive_symlink, LoadedFSFixture) {
  BOOST_REQUIRE(ErrorCode::kSuccess == CreateDirectory("home"));
  BOOST_REQUIRE(ErrorCode::kSuccess == CreateSymlink("home/root", "home/root"));

  BOOST_CHECK(ErrorCode::kErrorSymlinkDepth == OpenFile("home/root/.profile", file));
}

BOOST_FIXTURE_TEST_CASE(remove_one_file, LoadedFSFixture) {
  BOOST_REQUIRE(ErrorCode::kSuccess == CreateFile(".profile"));

  BOOST_CHECK(ErrorCode::kSuccess == Remove(".profile"));
}

BOOST_FIXTURE_TEST_CASE(remove_many_files, LoadedFSFixture) {
  for (int i = 0; i < kMany; ++i)
    BOOST_REQUIRE(ErrorCode::kSuccess == CreateFile(to_s(i)));

  for (int i = 0; i < kMany; ++i)
    BOOST_CHECK(ErrorCode::kSuccess == Remove(to_s(i)));
}

BOOST_FIXTURE_TEST_CASE(remove_file_in_sub_dir, LoadedFSFixture) {
  BOOST_REQUIRE(ErrorCode::kSuccess == CreateDirectory("home"));
  BOOST_REQUIRE(ErrorCode::kSuccess == CreateFile("home/.profile"));

  BOOST_CHECK(ErrorCode::kSuccess == Remove("home/.profile"));
}

BOOST_FIXTURE_TEST_CASE(remove_many_files_in_sub_dir, LoadedFSFixture) {
  BOOST_REQUIRE(ErrorCode::kSuccess == CreateDirectory("home"));
  for (int i = 0; i < kMany; ++i)
    BOOST_REQUIRE(ErrorCode::kSuccess == CreateFile("home/" + to_s(i)));

  for (int i = 0; i < kMany; ++i)
    BOOST_CHECK(ErrorCode::kSuccess == Remove("home/" + to_s(i)));
}

BOOST_FIXTURE_TEST_CASE(remove_file_is_case_sensitive, LoadedFSFixture) {
  BOOST_REQUIRE(ErrorCode::kSuccess == CreateFile(".profile"));

  BOOST_CHECK(ErrorCode::kErrorNotFound == Remove(".Profile"));
  BOOST_CHECK(ErrorCode::kSuccess == Remove(".profile"));
}

BOOST_FIXTURE_TEST_CASE(remove_file_if_already_removed, LoadedFSFixture) {
  BOOST_REQUIRE(ErrorCode::kSuccess == CreateFile(".profile"));
  BOOST_REQUIRE(ErrorCode::kSuccess == Remove(".profile"));

  BOOST_CHECK(ErrorCode::kErrorNotFound == Remove(".profile"));
}

BOOST_FIXTURE_TEST_CASE(write_zero_bytes, LoadedFSFixture) {
  BOOST_REQUIRE(ErrorCode::kSuccess == OpenFile(".profile", file));

  BOOST_CHECK(ErrorCode::kSuccess == WriteFile(file, ""));
}

BOOST_FIXTURE_TEST_CASE(write_ascii_data, LoadedFSFixture) {
  BOOST_REQUIRE(ErrorCode::kSuccess == OpenFile(".profile", file));

  BOOST_CHECK(ErrorCode::kSuccess == WriteFile(file, "01234"));
}

BOOST_FIXTURE_TEST_CASE(write_binary_data, LoadedFSFixture) {
  BOOST_REQUIRE(ErrorCode::kSuccess == OpenFile(".profile", file));

  BOOST_CHECK(ErrorCode::kSuccess == WriteFile(file, std::string("\0\x01\x02\x03\x04", 5)));
}

BOOST_FIXTURE_TEST_CASE(write_many_bytes_one_write, LoadedFSFixture) {
  BOOST_REQUIRE(ErrorCode::kSuccess == OpenFile(".profile", file));

  std::string to_file(k1MB, 'a');
  BOOST_CHECK(ErrorCode::kSuccess == WriteFile(file, to_file));
}

BOOST_FIXTURE_TEST_CASE(write_many_bytes_many_write, LoadedFSFixture) {
  BOOST_REQUIRE(ErrorCode::kSuccess == OpenFile(".profile", file));

  for (int i = 0; i < 10; ++i) {
    std::string to_file(k100KB, 'a' + i);
    BOOST_CHECK(ErrorCode::kSuccess == WriteFile(file, to_file));
  }
}

BOOST_FIXTURE_TEST_CASE(write_many_bytes_many_open, LoadedFSFixture) {
  for (int i = 0; i < 10; ++i) {
    BOOST_REQUIRE(ErrorCode::kSuccess == OpenFile(".profile", file));

    std::string to_file(k100KB, 'a' + i);
    BOOST_CHECK(ErrorCode::kSuccess == WriteFile(file, to_file));
  }
}

BOOST_FIXTURE_TEST_CASE(write_many_bytes_two_files, LoadedFSFixture) {
  ScopedFile file1, file2;
  BOOST_REQUIRE(ErrorCode::kSuccess == OpenFile("1", file1));
  BOOST_REQUIRE(ErrorCode::kSuccess == OpenFile("2", file2));

  for (int i = 0; i < 10; ++i) {
    std::string to_file1(k100KB, 'a' + i);
    BOOST_CHECK(ErrorCode::kSuccess == WriteFile(file1, to_file1));

    std::string to_file2(k100KB, 'z' - i);
    BOOST_CHECK(ErrorCode::kSuccess == WriteFile(file2, to_file2));
  }
}

BOOST_FIXTURE_TEST_CASE(write_many_bytes_if_reuse_unused_sections, LoadedFSFixture) {
  BOOST_REQUIRE(ErrorCode::kSuccess == CreateFile(".profile", std::string(k1MB, 'a')));
  uintmax_t device_size = boost::filesystem::file_size(device_path);
  BOOST_REQUIRE(ErrorCode::kSuccess == Remove(".profile"));
  BOOST_REQUIRE(ErrorCode::kSuccess == OpenFile(".profile", file));

  BOOST_CHECK(ErrorCode::kSuccess == WriteFile(file, std::string(k1MB, 'a')));
  BOOST_CHECK(device_size == boost::filesystem::file_size(device_path));
}

BOOST_FIXTURE_TEST_CASE(read_empty_file, LoadedFSFixture) {
  BOOST_REQUIRE(ErrorCode::kSuccess == OpenFile(".profile", file));

  std::string from_file(1, '\0');
  BOOST_CHECK(ErrorCode::kSuccess == ReadFile(file, from_file));
  BOOST_CHECK(from_file == "");
}

BOOST_FIXTURE_TEST_CASE(read_zero_bytes, LoadedFSFixture) {
  BOOST_REQUIRE(ErrorCode::kSuccess == CreateFile(".profile", "1"));
  BOOST_REQUIRE(ErrorCode::kSuccess == OpenFile(".profile", file));

  std::string from_file;
  BOOST_CHECK(ErrorCode::kSuccess == ReadFile(file, from_file));
  BOOST_CHECK(from_file == "");
}

BOOST_FIXTURE_TEST_CASE(read_exactly_one_byte, LoadedFSFixture) {
  BOOST_REQUIRE(ErrorCode::kSuccess == CreateFile(".profile", "12"));
  BOOST_REQUIRE(ErrorCode::kSuccess == OpenFile(".profile", file));

  std::string from_file(1, '\0');
  BOOST_CHECK(ErrorCode::kSuccess == ReadFile(file, from_file));
  BOOST_CHECK(from_file == "1");
}

BOOST_FIXTURE_TEST_CASE(read_entire_file, LoadedFSFixture) {
  BOOST_REQUIRE(ErrorCode::kSuccess == CreateFile(".profile", "1"));
  BOOST_REQUIRE(ErrorCode::kSuccess == OpenFile(".profile", file));

  std::string from_file(2, '\0');
  BOOST_CHECK(ErrorCode::kSuccess == ReadFile(file, from_file));
  BOOST_CHECK(from_file == "1");
}

BOOST_FIXTURE_TEST_CASE(read_file_if_eof, LoadedFSFixture) {
  BOOST_REQUIRE(ErrorCode::kSuccess == CreateFile(".profile", "1"));
  BOOST_REQUIRE(ErrorCode::kSuccess == OpenFile(".profile", file));

  std::string from_file(2, '\0');
  BOOST_REQUIRE(ErrorCode::kSuccess == ReadFile(file, from_file));
  BOOST_REQUIRE(from_file == "1");

  BOOST_CHECK(ErrorCode::kSuccess == ReadFile(file, from_file));
  BOOST_CHECK(from_file == "");
}

BOOST_FIXTURE_TEST_CASE(read_ascii_data, LoadedFSFixture) {
  std::string to_file = "01234";
  BOOST_REQUIRE(ErrorCode::kSuccess == CreateFile(".profile", to_file));
  BOOST_REQUIRE(ErrorCode::kSuccess == OpenFile(".profile", file));

  std::string from_file(to_file.size(), '\0');
  BOOST_CHECK(ErrorCode::kSuccess == ReadFile(file, from_file));
  BOOST_CHECK(from_file == to_file);
}

BOOST_FIXTURE_TEST_CASE(read_binary_data, LoadedFSFixture) {
  std::string to_file("\0\x01\x02\x03\x04", 5);
  BOOST_REQUIRE(ErrorCode::kSuccess == CreateFile(".profile", to_file));
  BOOST_REQUIRE(ErrorCode::kSuccess == OpenFile(".profile", file));

  std::string from_file(to_file.size(), '\0');
  BOOST_CHECK(ErrorCode::kSuccess == ReadFile(file, from_file));
  BOOST_CHECK(from_file == to_file);
}

BOOST_FIXTURE_TEST_CASE(read_many_bytes, LoadedFSFixture) {
  std::string to_file(k1MB, 'a');
  BOOST_REQUIRE(ErrorCode::kSuccess == CreateFile(".profile", to_file));
  BOOST_REQUIRE(ErrorCode::kSuccess == OpenFile(".profile", file));

  std::string from_file(to_file.size(), '\0');
  BOOST_CHECK(ErrorCode::kSuccess == ReadFile(file, from_file));
  BOOST_CHECK(from_file == to_file);
}

BOOST_FIXTURE_TEST_CASE(read_many_bytes_one_read, LoadedFSFixture) {
  BOOST_REQUIRE(ErrorCode::kSuccess == OpenFile(".profile", file));
  std::string to_file;
  for (int i = 0; i < 10; ++i) {
    std::string to_file_i(k100KB, 'a' + i);
    to_file += to_file_i;
    BOOST_REQUIRE(ErrorCode::kSuccess == WriteFile(file, to_file_i));
  }
  BOOST_REQUIRE(ErrorCode::kSuccess == OpenFile(".profile", file));

  std::string from_file(to_file.size(), '\0');
  BOOST_CHECK(ErrorCode::kSuccess == ReadFile(file, from_file));
  BOOST_CHECK(from_file == to_file);
}

BOOST_FIXTURE_TEST_CASE(read_many_bytes_many_read, LoadedFSFixture) {
  std::string to_file(k1MB, 'a');
  BOOST_REQUIRE(ErrorCode::kSuccess == CreateFile(".profile", to_file));
  BOOST_REQUIRE(ErrorCode::kSuccess == OpenFile(".profile", file));

  for (int i = 0; i < 10; ++i) {
    std::string from_file(to_file.size() / 10, '\0');
    BOOST_CHECK(ErrorCode::kSuccess == ReadFile(file, from_file));
    BOOST_CHECK(from_file == to_file.substr(i * k100KB, k100KB));
  }
}

BOOST_FIXTURE_TEST_CASE(read_many_bytes_two_files, LoadedFSFixture) {
  std::string to_file1, to_file2;  // 1MB
  for (int i = 0; i < 10; ++i) {
    to_file1 += std::string(k100KB, 'a' + i);
    to_file2 += std::string(k100KB, 'z' - i);
  }
  BOOST_REQUIRE(ErrorCode::kSuccess == CreateFile("1", to_file1));
  BOOST_REQUIRE(ErrorCode::kSuccess == CreateFile("2", to_file2));
  ScopedFile file1, file2;
  BOOST_REQUIRE(ErrorCode::kSuccess == OpenFile("1", file1));
  BOOST_REQUIRE(ErrorCode::kSuccess == OpenFile("2", file2));

  for (int i = 0; i < 10; ++i) {
    std::string from_file1(k100KB, '\0');
    BOOST_CHECK(ErrorCode::kSuccess == ReadFile(file1, from_file1));
    BOOST_CHECK(from_file1 == to_file1.substr(i * k100KB, k100KB));

    std::string from_file2(k100KB, '\0');
    BOOST_CHECK(ErrorCode::kSuccess == ReadFile(file2, from_file2));
    BOOST_CHECK(from_file2 == to_file2.substr(i * k100KB, k100KB));
  }
}

BOOST_FIXTURE_TEST_CASE(read_many_bytes_many_read_if_reuse_unused_sections, LoadedFSFixture) {
  BOOST_REQUIRE(ErrorCode::kSuccess == CreateFile(".profile", std::string(k1MB, 'a')));
  BOOST_REQUIRE(ErrorCode::kSuccess == Remove(".profile"));
  std::string to_file;  // 1MB
  for (int i = 0; i < 10; ++i)
    to_file += std::string(k100KB, 'a' + i);
  BOOST_REQUIRE(ErrorCode::kSuccess == CreateFile(".profile", to_file));
  BOOST_REQUIRE(ErrorCode::kSuccess == OpenFile(".profile", file));

  for (int i = 0; i < 10; ++i) {
    std::string from_file(k100KB, '\0');
    BOOST_CHECK(ErrorCode::kSuccess == ReadFile(file, from_file));
    BOOST_CHECK(from_file == to_file.substr(i * k100KB, k100KB));
  }
}

BOOST_FIXTURE_TEST_CASE(get_cursor_after_open_if_file_created, LoadedFSFixture) {
  BOOST_REQUIRE(ErrorCode::kSuccess == OpenFile(".profile", file));

  BOOST_CHECK(0 == file->GetCursor());
}

BOOST_FIXTURE_TEST_CASE(get_cursor_after_open_if_file_exists, LoadedFSFixture) {
  BOOST_REQUIRE(ErrorCode::kSuccess == CreateFile(".profile", "1"));
  BOOST_REQUIRE(ErrorCode::kSuccess == OpenFile(".profile", file));

  BOOST_CHECK(0 == file->GetCursor());
}

BOOST_FIXTURE_TEST_CASE(get_cursor_after_read_zero_bytes, LoadedFSFixture) {
  BOOST_REQUIRE(ErrorCode::kSuccess == CreateFile(".profile", "1"));
  BOOST_REQUIRE(ErrorCode::kSuccess == OpenFile(".profile", file));
  std::string from_file(0, '\0');
  BOOST_REQUIRE(ErrorCode::kSuccess == ReadFile(file, from_file));
  BOOST_REQUIRE(from_file == "");

  BOOST_CHECK(0 == file->GetCursor());
}

BOOST_FIXTURE_TEST_CASE(get_cursor_after_read_some_bytes, LoadedFSFixture) {
  BOOST_REQUIRE(ErrorCode::kSuccess == CreateFile(".profile", "12"));
  BOOST_REQUIRE(ErrorCode::kSuccess == OpenFile(".profile", file));
  std::string from_file(1, '\0');
  BOOST_REQUIRE(ErrorCode::kSuccess == ReadFile(file, from_file));
  BOOST_REQUIRE(from_file == "1");

  BOOST_CHECK(1 == file->GetCursor());
}

BOOST_FIXTURE_TEST_CASE(get_cursor_if_eof, LoadedFSFixture) {
  BOOST_REQUIRE(ErrorCode::kSuccess == CreateFile(".profile", "1"));
  BOOST_REQUIRE(ErrorCode::kSuccess == OpenFile(".profile", file));
  std::string from_file(2, '\0');
  BOOST_REQUIRE(ErrorCode::kSuccess == ReadFile(file, from_file));
  BOOST_REQUIRE(from_file == "1");

  BOOST_CHECK(1 == file->GetCursor());
}

BOOST_FIXTURE_TEST_CASE(get_cursor_after_write_zero_bytes, LoadedFSFixture) {
  BOOST_REQUIRE(ErrorCode::kSuccess == OpenFile(".profile", file));
  BOOST_REQUIRE(ErrorCode::kSuccess == WriteFile(file, ""));

  BOOST_CHECK(0 == file->GetCursor());
}

BOOST_FIXTURE_TEST_CASE(get_cursor_after_write_some_bytes, LoadedFSFixture) {
  BOOST_REQUIRE(ErrorCode::kSuccess == OpenFile(".profile", file));
  BOOST_REQUIRE(ErrorCode::kSuccess == WriteFile(file, "12"));

  BOOST_CHECK(2 == file->GetCursor());
}

BOOST_FIXTURE_TEST_CASE(set_cursor_same_place, LoadedFSFixture) {
  BOOST_REQUIRE(ErrorCode::kSuccess == CreateFile(".profile", "1"));
  BOOST_REQUIRE(ErrorCode::kSuccess == OpenFile(".profile", file));

  BOOST_CHECK(ErrorCode::kSuccess == file->SetCursor(0));
  BOOST_CHECK(0 == file->GetCursor());
}

BOOST_FIXTURE_TEST_CASE(set_cursor_forward, LoadedFSFixture) {
  BOOST_REQUIRE(ErrorCode::kSuccess == CreateFile(".profile", "123"));
  BOOST_REQUIRE(ErrorCode::kSuccess == OpenFile(".profile", file));
  std::string from_file(1, '\0');
  BOOST_REQUIRE(ErrorCode::kSuccess == ReadFile(file, from_file));
  BOOST_REQUIRE(from_file == "1");

  BOOST_CHECK(ErrorCode::kSuccess == file->SetCursor(2));
  BOOST_CHECK(2 == file->GetCursor());
}

BOOST_FIXTURE_TEST_CASE(set_cursor_most_forward, LoadedFSFixture) {
  BOOST_REQUIRE(ErrorCode::kSuccess == CreateFile(".profile", "123"));
  BOOST_REQUIRE(ErrorCode::kSuccess == OpenFile(".profile", file));
  BOOST_REQUIRE(ErrorCode::kSuccess == WriteFile(file, "1"));

  BOOST_CHECK(ErrorCode::kSuccess == file->SetCursor(3));
  BOOST_CHECK(3 == file->GetCursor());
}

BOOST_FIXTURE_TEST_CASE(set_cursor_backward, LoadedFSFixture) {
  BOOST_REQUIRE(ErrorCode::kSuccess == CreateFile(".profile", "12"));
  BOOST_REQUIRE(ErrorCode::kSuccess == OpenFile(".profile", file));
  std::string from_file(3, '\0');
  BOOST_REQUIRE(ErrorCode::kSuccess == ReadFile(file, from_file));
  BOOST_REQUIRE(from_file == "12");

  BOOST_CHECK(ErrorCode::kSuccess == file->SetCursor(1));
  BOOST_CHECK(1 == file->GetCursor());
}

BOOST_FIXTURE_TEST_CASE(set_cursor_most_backward, LoadedFSFixture) {
  BOOST_REQUIRE(ErrorCode::kSuccess == OpenFile(".profile", file));
  BOOST_REQUIRE(ErrorCode::kSuccess == WriteFile(file, "12"));

  BOOST_CHECK(ErrorCode::kSuccess == file->SetCursor(0));
  BOOST_CHECK(0 == file->GetCursor());
}

BOOST_FIXTURE_TEST_CASE(set_cursor_if_greater_than_file_size, LoadedFSFixture) {
  BOOST_REQUIRE(ErrorCode::kSuccess == CreateFile(".profile", "12"));
  BOOST_REQUIRE(ErrorCode::kSuccess == OpenFile(".profile", file));

  BOOST_CHECK(ErrorCode::kErrorCursorTooBig == file->SetCursor(3));
}

BOOST_FIXTURE_TEST_CASE(get_size_after_open_if_file_created, LoadedFSFixture) {
  BOOST_REQUIRE(ErrorCode::kSuccess == OpenFile(".profile", file));

  BOOST_CHECK(0 == file->GetSize());
}

BOOST_FIXTURE_TEST_CASE(get_size_after_open_if_file_exists, LoadedFSFixture) {
  BOOST_REQUIRE(ErrorCode::kSuccess == CreateFile(".profile", "1"));
  BOOST_REQUIRE(ErrorCode::kSuccess == OpenFile(".profile", file));

  BOOST_CHECK(1 == file->GetSize());
}

BOOST_FIXTURE_TEST_CASE(get_size_after_read_some_bytes, LoadedFSFixture) {
  BOOST_REQUIRE(ErrorCode::kSuccess == CreateFile(".profile", "12"));
  BOOST_REQUIRE(ErrorCode::kSuccess == OpenFile(".profile", file));
  std::string from_file(3, '\0');
  BOOST_REQUIRE(ErrorCode::kSuccess == ReadFile(file, from_file));
  BOOST_REQUIRE(from_file == "12");

  BOOST_CHECK(2 == file->GetSize());
}

BOOST_FIXTURE_TEST_CASE(get_size_after_write_zero_bytes, LoadedFSFixture) {
  BOOST_REQUIRE(ErrorCode::kSuccess == OpenFile(".profile", file));
  BOOST_REQUIRE(ErrorCode::kSuccess == WriteFile(file, ""));

  BOOST_CHECK(0 == file->GetSize());
}

BOOST_FIXTURE_TEST_CASE(get_size_after_write_some_bytes, LoadedFSFixture) {
  BOOST_REQUIRE(ErrorCode::kSuccess == OpenFile(".profile", file));
  BOOST_REQUIRE(ErrorCode::kSuccess == WriteFile(file, "12"));

  BOOST_CHECK(2 == file->GetSize());
}

BOOST_FIXTURE_TEST_CASE(get_size_after_set_cursor, LoadedFSFixture) {
  BOOST_REQUIRE(ErrorCode::kSuccess == CreateFile(".profile", "12"));
  BOOST_REQUIRE(ErrorCode::kSuccess == OpenFile(".profile", file));
  BOOST_REQUIRE(ErrorCode::kSuccess == file->SetCursor(1));

  BOOST_CHECK(2 == file->GetSize());
}

BOOST_FIXTURE_TEST_CASE(is_file_for_file, LoadedFSFixture) {
  BOOST_REQUIRE(ErrorCode::kSuccess == CreateFile(".profile"));

  BOOST_CHECK(fs->IsFile(".profile", &ec));
  BOOST_CHECK(ErrorCode::kSuccess == ec);
}

BOOST_FIXTURE_TEST_CASE(is_file_for_dir, LoadedFSFixture) {
  BOOST_REQUIRE(ErrorCode::kSuccess == CreateDirectory("home"));

  BOOST_CHECK(!fs->IsFile("home", &ec));
  BOOST_CHECK(ErrorCode::kSuccess == ec);
}

BOOST_FIXTURE_TEST_CASE(is_file_for_symlink, LoadedFSFixture) {
  BOOST_REQUIRE(ErrorCode::kSuccess == CreateSymlink("lnk", "target"));

  BOOST_CHECK(!fs->IsFile("lnk", &ec));
  BOOST_CHECK(ErrorCode::kSuccess == ec);
}

BOOST_FIXTURE_TEST_CASE(is_file_if_doesnt_exist, LoadedFSFixture) {
  BOOST_CHECK(!fs->IsFile(".profile", &ec));
  BOOST_CHECK(ErrorCode::kErrorNotFound == ec);
}

BOOST_AUTO_TEST_SUITE_END()
