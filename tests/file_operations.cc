#include <string>

#include "tests/filesystem_fixtures.h"

#include "fs/limits.h"

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

BOOST_FIXTURE_TEST_CASE(open_file_if_file_exists, LoadedFSFixture) {
  BOOST_REQUIRE(ErrorCode::kSuccess == CreateFile(".profile"));

  BOOST_CHECK(ErrorCode::kSuccess == OpenFile(".profile", file));
}

BOOST_FIXTURE_TEST_CASE(open_file_if_dir_exists, LoadedFSFixture) {
  BOOST_REQUIRE(ErrorCode::kSuccess == CreateDirectory("file_or_directory"));

  BOOST_CHECK(ErrorCode::kErrorIsDirectory == OpenFile("file_or_directory", file));
}

BOOST_FIXTURE_TEST_CASE(remove_one_file, LoadedFSFixture) {
  BOOST_REQUIRE(ErrorCode::kSuccess == CreateFile(".profile"));

  BOOST_CHECK(ErrorCode::kSuccess == RemoveFile(".profile"));
}

BOOST_FIXTURE_TEST_CASE(remove_many_files, LoadedFSFixture) {
  for (int i = 0; i < kMany; ++i)
    BOOST_REQUIRE(ErrorCode::kSuccess == CreateFile(to_s(i)));

  for (int i = 0; i < kMany; ++i)
    BOOST_CHECK(ErrorCode::kSuccess == RemoveFile(to_s(i)));
}

BOOST_FIXTURE_TEST_CASE(remove_file_in_sub_dir, LoadedFSFixture) {
  BOOST_REQUIRE(ErrorCode::kSuccess == CreateDirectory("home"));
  BOOST_REQUIRE(ErrorCode::kSuccess == CreateFile("home/.profile"));

  BOOST_CHECK(ErrorCode::kSuccess == RemoveFile("home/.profile"));
}

BOOST_FIXTURE_TEST_CASE(remove_many_files_in_sub_dir, LoadedFSFixture) {
  BOOST_REQUIRE(ErrorCode::kSuccess == CreateDirectory("home"));
  for (int i = 0; i < kMany; ++i)
    BOOST_REQUIRE(ErrorCode::kSuccess == CreateFile("home/" + to_s(i)));

  for (int i = 0; i < kMany; ++i)
    BOOST_CHECK(ErrorCode::kSuccess == RemoveFile("home/" + to_s(i)));
}

BOOST_FIXTURE_TEST_CASE(remove_file_is_case_sensitive, LoadedFSFixture) {
  BOOST_REQUIRE(ErrorCode::kSuccess == CreateFile(".profile"));

  BOOST_CHECK(ErrorCode::kErrorNotFound == RemoveFile(".Profile"));
  BOOST_CHECK(ErrorCode::kSuccess == RemoveFile(".profile"));
}

BOOST_FIXTURE_TEST_CASE(remove_file_if_already_removed, LoadedFSFixture) {
  BOOST_REQUIRE(ErrorCode::kSuccess == CreateFile(".profile"));
  BOOST_REQUIRE(ErrorCode::kSuccess == RemoveFile(".profile"));

  BOOST_CHECK(ErrorCode::kErrorNotFound == RemoveFile(".profile"));
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

BOOST_AUTO_TEST_SUITE_END()
