#include <string>

#include "tests/filesystem_fixtures.h"

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(FileOperationsTestSuite)

using fs::ErrorCode;
using fs::FileInterface;
using fs::FilesystemInterface;

BOOST_FIXTURE_TEST_CASE(open_one_file, LoadedFSFixture) {
  BOOST_CHECK_NO_THROW(file = fs->OpenFile(".profile", ec));
  BOOST_CHECK(ec == ErrorCode::kSuccess);
  BOOST_CHECK_NO_THROW(file->Close());
}

BOOST_FIXTURE_TEST_CASE(open_two_files, LoadedFSFixture) {
  FileInterface *file1, *file2;
  BOOST_REQUIRE_NO_THROW(file1 = fs->OpenFile("1", ec));
  BOOST_REQUIRE(ec == ErrorCode::kSuccess);

  BOOST_CHECK_NO_THROW(file2 = fs->OpenFile("2", ec));
  BOOST_CHECK(ec == ErrorCode::kSuccess);

  BOOST_CHECK_NO_THROW(file1->Close());
  BOOST_CHECK_NO_THROW(file2->Close());
}

BOOST_FIXTURE_TEST_CASE(open_many_files, LoadedFSFixture) {
  for (int i = 0; i < 100; ++i) {
    BOOST_CHECK_NO_THROW(file = fs->OpenFile(std::to_string(i).c_str(), ec));
    BOOST_CHECK(ec == ErrorCode::kSuccess);
    BOOST_CHECK_NO_THROW(file->Close());
  }
}

BOOST_FIXTURE_TEST_CASE(open_file_in_sub_dir, LoadedFSFixture) {
  BOOST_REQUIRE_NO_THROW(ec = fs->CreateDirectory("home"));
  BOOST_REQUIRE(ec == ErrorCode::kSuccess);

  BOOST_CHECK_NO_THROW(file = fs->OpenFile(".profile", ec));
  BOOST_CHECK(ec == ErrorCode::kSuccess);
  BOOST_CHECK_NO_THROW(file->Close());
}

BOOST_FIXTURE_TEST_CASE(open_many_files_in_sub_dir, LoadedFSFixture) {
  BOOST_REQUIRE_NO_THROW(ec = fs->CreateDirectory("home"));
  BOOST_REQUIRE(ec == ErrorCode::kSuccess);

  for (int i = 0; i < 100; ++i) {
    BOOST_CHECK_NO_THROW(file = fs->OpenFile((std::string("home/") + std::to_string(i)).c_str(), ec));
    BOOST_CHECK(ec == ErrorCode::kSuccess);
    BOOST_CHECK_NO_THROW(file->Close());
  }
}

BOOST_FIXTURE_TEST_CASE(open_file_with_complex_name, LoadedFSFixture) {
  BOOST_CHECK_NO_THROW(file = fs->OpenFile("a-Z0+9@#$%^&*(){}[]|_=:;.,'\"", ec));
  BOOST_CHECK(ec == ErrorCode::kSuccess);
  BOOST_CHECK_NO_THROW(file->Close());
}

BOOST_FIXTURE_TEST_CASE(open_file_with_long_name, LoadedFSFixture) {
  BOOST_CHECK_NO_THROW(file = fs->OpenFile(std::string(fs::kNameMax, 'a').c_str(), ec));
  BOOST_CHECK(ec == ErrorCode::kSuccess);
  BOOST_CHECK_NO_THROW(file->Close());
}

BOOST_FIXTURE_TEST_CASE(open_file_with_too_long_name, LoadedFSFixture) {
  BOOST_CHECK_NO_THROW(file = fs->OpenFile(std::string(fs::kNameMax + 1, 'a').c_str(), ec));
  BOOST_CHECK(ec == ErrorCode::kErrorNameTooLong);
  BOOST_CHECK(file == nullptr);
}

BOOST_FIXTURE_TEST_CASE(open_file_if_file_exists, LoadedFSFixture) {
  BOOST_REQUIRE_NO_THROW(file = fs->OpenFile(".profile", ec));
  BOOST_REQUIRE(ec == ErrorCode::kSuccess);
  BOOST_REQUIRE_NO_THROW(file->Close());

  BOOST_CHECK_NO_THROW(file = fs->OpenFile(".profile", ec));
  BOOST_CHECK(ec == ErrorCode::kSuccess);
  BOOST_CHECK_NO_THROW(file->Close());
}

BOOST_FIXTURE_TEST_CASE(open_file_if_dir_exists, LoadedFSFixture) {
  BOOST_REQUIRE_NO_THROW(ec = fs->CreateDirectory(".profile"));
  BOOST_REQUIRE(ec == ErrorCode::kSuccess);

  BOOST_CHECK_NO_THROW(file = fs->OpenFile(".profile", ec));
  BOOST_CHECK(ec == ErrorCode::kErrorIsDirectory);
  BOOST_CHECK(file == nullptr);
}

BOOST_FIXTURE_TEST_CASE(remove_one_file, LoadedFSFixture) {
  BOOST_REQUIRE_NO_THROW(file = fs->OpenFile(".profile", ec));
  BOOST_REQUIRE(ec == ErrorCode::kSuccess);
  BOOST_REQUIRE_NO_THROW(file->Close());

  BOOST_CHECK_NO_THROW(ec = fs->RemoveFile(".profile"));
  BOOST_CHECK(ec == ErrorCode::kSuccess);
}

BOOST_FIXTURE_TEST_CASE(remove_many_files, LoadedFSFixture) {
  for (int i = 0; i < 100; ++i) {
    BOOST_REQUIRE_NO_THROW(file = fs->OpenFile(std::to_string(i).c_str(), ec));
    BOOST_REQUIRE(ec == ErrorCode::kSuccess);
    BOOST_REQUIRE_NO_THROW(file->Close());
  }

  for (int i = 0; i < 100; ++i) {
    BOOST_CHECK_NO_THROW(ec = fs->RemoveFile(std::to_string(i).c_str()));
    BOOST_CHECK(ec == ErrorCode::kSuccess);
  }
}

BOOST_FIXTURE_TEST_CASE(remove_file_in_sub_dir, LoadedFSFixture) {
  BOOST_REQUIRE_NO_THROW(ec = fs->CreateDirectory("home"));
  BOOST_REQUIRE(ec == ErrorCode::kSuccess);
  BOOST_REQUIRE_NO_THROW(file = fs->OpenFile("home/.profile", ec));
  BOOST_REQUIRE(ec == ErrorCode::kSuccess);
  BOOST_REQUIRE_NO_THROW(file->Close());

  BOOST_CHECK_NO_THROW(ec = fs->RemoveFile("home/.profile"))
  BOOST_CHECK(ec == ErrorCode::kSuccess);
}

BOOST_FIXTURE_TEST_CASE(remove_many_files_in_sub_dir, LoadedFSFixture) {
  BOOST_REQUIRE_NO_THROW(ec = fs->CreateDirectory("home"));
  BOOST_REQUIRE(ec == ErrorCode::kSuccess);
  for (int i = 0; i < 100; ++i) {
    BOOST_REQUIRE_NO_THROW(file = fs->OpenFile((std::string("home/") + std::to_string(i)).c_str(), ec));
    BOOST_REQUIRE(ec == ErrorCode::kSuccess);
    BOOST_REQUIRE_NO_THROW(file->Close());
  }

  for (int i = 0; i < 100; ++i) {
    BOOST_CHECK_NO_THROW(ec = fs->RemoveFile((std::string("home/") + std::to_string(i)).c_str()));
    BOOST_CHECK(ec == ErrorCode::kSuccess);
  }
}

BOOST_FIXTURE_TEST_CASE(remove_file_is_case_sensitive, LoadedFSFixture) {
  BOOST_REQUIRE_NO_THROW(file = fs->OpenFile(".profile", ec));
  BOOST_REQUIRE(ec == ErrorCode::kSuccess);
  BOOST_REQUIRE_NO_THROW(file->Close());

  BOOST_CHECK_NO_THROW(ec = fs->RemoveFile(".Profile"));
  BOOST_CHECK(ec == ErrorCode::kErrorNotFound);

  BOOST_CHECK_NO_THROW(ec = fs->RemoveFile(".profile"));
  BOOST_CHECK(ec == ErrorCode::kSuccess);
}

BOOST_FIXTURE_TEST_CASE(remove_file_if_already_removed, LoadedFSFixture) {
  BOOST_REQUIRE_NO_THROW(file = fs->OpenFile(".profile", ec));
  BOOST_REQUIRE(ec == ErrorCode::kSuccess);
  BOOST_REQUIRE_NO_THROW(file->Close());

  BOOST_REQUIRE_NO_THROW(ec = fs->RemoveFile(".profile"));
  BOOST_REQUIRE(ec == ErrorCode::kSuccess);

  BOOST_CHECK_NO_THROW(ec = fs->RemoveFile(".profile"));
  BOOST_CHECK(ec == ErrorCode::kErrorNotFound);
}

BOOST_FIXTURE_TEST_CASE(write_zero_bytes, LoadedFSFixture) {
  BOOST_REQUIRE_NO_THROW(file = fs->OpenFile(".profile", ec));
  BOOST_REQUIRE(ec == ErrorCode::kSuccess);

  size_t written;
  BOOST_CHECK_NO_THROW(written = file->Write(nullptr, 0));
  BOOST_CHECK(written == 0);

  BOOST_CHECK_NO_THROW(file->Close());
}

BOOST_FIXTURE_TEST_CASE(write_ascii_data, LoadedFSFixture) {
  BOOST_REQUIRE_NO_THROW(file = fs->OpenFile(".profile", ec));
  BOOST_REQUIRE(ec == ErrorCode::kSuccess);

  size_t written;
  BOOST_CHECK_NO_THROW(written = file->Write("01234", 5));
  BOOST_CHECK(written == 5);

  BOOST_CHECK_NO_THROW(file->Close());
}

BOOST_FIXTURE_TEST_CASE(write_binary_data, LoadedFSFixture) {
  BOOST_REQUIRE_NO_THROW(file = fs->OpenFile(".profile", ec));
  BOOST_REQUIRE(ec == ErrorCode::kSuccess);

  size_t written;
  BOOST_CHECK_NO_THROW(written = file->Write("\0\x01\x02\x03\x04", 5));
  BOOST_CHECK(written == 5);

  BOOST_CHECK_NO_THROW(file->Close());
}

BOOST_FIXTURE_TEST_CASE(write_many_bytes_one_write, LoadedFSFixture) {
  BOOST_REQUIRE_NO_THROW(file = fs->OpenFile(".profile", ec));
  BOOST_REQUIRE(ec == ErrorCode::kSuccess);

  std::string to_file(1000000, 'a'); // 1Mb
  size_t written;
  BOOST_CHECK_NO_THROW(written = file->Write(to_file.c_str(), to_file.size()));
  BOOST_CHECK(written == to_file.size());

  BOOST_CHECK_NO_THROW(file->Close());
}

BOOST_FIXTURE_TEST_CASE(write_many_bytes_many_write, LoadedFSFixture) {
  BOOST_REQUIRE_NO_THROW(file = fs->OpenFile(".profile", ec));
  BOOST_REQUIRE(ec == ErrorCode::kSuccess);

  for (int i = 0; i < 10; ++i) {
    std::string to_file(100000, 'a' + i); // 100Kb
    size_t written;
    BOOST_CHECK_NO_THROW(written = file->Write(to_file.c_str(), to_file.size()));
    BOOST_CHECK(written == to_file.size());
  }

  BOOST_CHECK_NO_THROW(file->Close());
}

BOOST_FIXTURE_TEST_CASE(write_many_bytes_many_open, LoadedFSFixture) {
  for (int i = 0; i < 10; ++i) {
    std::string to_file(100000, 'a' + i); // 100Kb
    BOOST_REQUIRE_NO_THROW(file = fs->OpenFile(".profile", ec));
    BOOST_REQUIRE(ec == ErrorCode::kSuccess);
    size_t written;
    BOOST_CHECK_NO_THROW(written = file->Write(to_file.c_str(), to_file.size()));
    BOOST_CHECK(written == to_file.size());
    BOOST_CHECK_NO_THROW(file->Close());
  }
}

BOOST_FIXTURE_TEST_CASE(write_many_bytes_two_files, LoadedFSFixture) {
  FileInterface *file1, *file2;
  BOOST_REQUIRE_NO_THROW(file1 = fs->OpenFile("1", ec));
  BOOST_REQUIRE(ec == ErrorCode::kSuccess);
  BOOST_REQUIRE_NO_THROW(file2 = fs->OpenFile("2", ec));
  BOOST_REQUIRE(ec == ErrorCode::kSuccess);

  for (int i = 0; i < 10; ++i) {
    std::string to_file1(100000, 'a' + i); // 100Kb
    std::string to_file2(100000, 'z' - i); // 100Kb
    size_t written;
    BOOST_CHECK_NO_THROW(written = file1->Write(to_file1.c_str(), to_file1.size()));
    BOOST_CHECK(written == to_file1.size());
    BOOST_CHECK_NO_THROW(written = file2->Write(to_file2.c_str(), to_file2.size()));
    BOOST_CHECK(written == to_file2.size());
  }

  BOOST_CHECK_NO_THROW(file1->Close());
  BOOST_CHECK_NO_THROW(file2->Close());
}

BOOST_FIXTURE_TEST_CASE(read_empty_file, LoadedFSFixture) {
  BOOST_REQUIRE_NO_THROW(file = fs->OpenFile(".profile", ec));
  BOOST_REQUIRE(ec == ErrorCode::kSuccess);

  size_t read;
  BOOST_CHECK_NO_THROW(read = file->Read(nullptr, 1));
  BOOST_CHECK(read == 0);

  BOOST_CHECK_NO_THROW(file->Close());
}

BOOST_FIXTURE_TEST_CASE(read_zero_bytes, LoadedFSFixture) {
  BOOST_REQUIRE_NO_THROW(file = fs->OpenFile(".profile", ec));
  BOOST_REQUIRE(ec == ErrorCode::kSuccess);
  size_t written;
  BOOST_REQUIRE_NO_THROW(written = file->Write("1", 1));
  BOOST_REQUIRE(written == 1);
  BOOST_REQUIRE_NO_THROW(file->Close());
  BOOST_REQUIRE_NO_THROW(file = fs->OpenFile(".profile", ec));
  BOOST_REQUIRE(ec == ErrorCode::kSuccess);

  size_t read;
  BOOST_CHECK_NO_THROW(read = file->Read(nullptr, 0));
  BOOST_CHECK(read == 0);

  BOOST_CHECK_NO_THROW(file->Close());
}

BOOST_FIXTURE_TEST_CASE(read_exactly_one_byte, LoadedFSFixture) {
  BOOST_REQUIRE_NO_THROW(file = fs->OpenFile(".profile", ec));
  BOOST_REQUIRE(ec == ErrorCode::kSuccess);
  size_t written;
  BOOST_REQUIRE_NO_THROW(written = file->Write("12", 2));
  BOOST_REQUIRE(written == 2);
  BOOST_REQUIRE_NO_THROW(file->Close());
  BOOST_REQUIRE_NO_THROW(file = fs->OpenFile(".profile", ec));
  BOOST_REQUIRE(ec == ErrorCode::kSuccess);

  size_t read;
  std::string from_file(2, '\0');
  BOOST_CHECK_NO_THROW(read = file->Read(&from_file[0], 1));
  BOOST_CHECK(read == 1);
  BOOST_CHECK(from_file[0] == '1');

  BOOST_CHECK(from_file[1] == '\0');

  BOOST_CHECK_NO_THROW(file->Close());
}

BOOST_FIXTURE_TEST_CASE(read_entire_file, LoadedFSFixture) {
  BOOST_REQUIRE_NO_THROW(file = fs->OpenFile(".profile", ec));
  BOOST_REQUIRE(ec == ErrorCode::kSuccess);
  size_t written;
  BOOST_REQUIRE_NO_THROW(written = file->Write("1", 1));
  BOOST_REQUIRE(written == 1);
  BOOST_REQUIRE_NO_THROW(file->Close());
  BOOST_REQUIRE_NO_THROW(file = fs->OpenFile(".profile", ec));
  BOOST_REQUIRE(ec == ErrorCode::kSuccess);

  size_t read;
  std::string from_file(2, '\0');
  BOOST_CHECK_NO_THROW(read = file->Read(&from_file[0], from_file.size()));
  BOOST_CHECK(read == 1);
  BOOST_CHECK(from_file[0] == '1');

  BOOST_CHECK_NO_THROW(file->Close());
}

BOOST_FIXTURE_TEST_CASE(read_file_if_eof, LoadedFSFixture) {
  BOOST_REQUIRE_NO_THROW(file = fs->OpenFile(".profile", ec));
  BOOST_REQUIRE(ec == ErrorCode::kSuccess);
  size_t written;
  BOOST_REQUIRE_NO_THROW(written = file->Write("1", 1));
  BOOST_REQUIRE(written == 1);
  BOOST_REQUIRE_NO_THROW(file->Close());
  BOOST_REQUIRE_NO_THROW(file = fs->OpenFile(".profile", ec));
  BOOST_REQUIRE(ec == ErrorCode::kSuccess);
  size_t read;
  std::string from_file(2, '\0');
  BOOST_REQUIRE_NO_THROW(read = file->Read(&from_file[0], from_file.size()));
  BOOST_REQUIRE(read == 1);
  BOOST_REQUIRE(from_file[0] == '1');

  BOOST_CHECK_NO_THROW(read = file->Read(nullptr, 1));
  BOOST_CHECK(read == 0);

  BOOST_CHECK_NO_THROW(file->Close());
}

BOOST_FIXTURE_TEST_CASE(read_ascii_data, LoadedFSFixture) {
  BOOST_REQUIRE_NO_THROW(file = fs->OpenFile(".profile", ec));
  BOOST_REQUIRE(ec == ErrorCode::kSuccess);
  std::string to_file("01234");
  size_t written;
  BOOST_REQUIRE_NO_THROW(written = file->Write(to_file.c_str(), to_file.size()));
  BOOST_REQUIRE(written == to_file.size());
  BOOST_REQUIRE_NO_THROW(file->Close());
  BOOST_REQUIRE_NO_THROW(file = fs->OpenFile(".profile", ec));
  BOOST_REQUIRE(ec == ErrorCode::kSuccess);

  size_t read;
  std::string from_file(to_file.size(), '\0');
  BOOST_CHECK_NO_THROW(read = file->Read(&from_file[0], from_file.size()));  // It's valid since C++11.
  BOOST_CHECK(read == to_file.size());
  BOOST_CHECK(from_file == to_file);

  BOOST_CHECK_NO_THROW(file->Close());
}

BOOST_FIXTURE_TEST_CASE(read_binary_data, LoadedFSFixture) {
  BOOST_REQUIRE_NO_THROW(file = fs->OpenFile(".profile", ec));
  BOOST_REQUIRE(ec == ErrorCode::kSuccess);
  std::string to_file("\0\x01\x02\x03\x04", 5);
  size_t written;
  BOOST_REQUIRE_NO_THROW(written = file->Write(to_file.c_str(), to_file.size()));
  BOOST_REQUIRE(written == to_file.size());
  BOOST_REQUIRE_NO_THROW(file->Close());
  BOOST_REQUIRE_NO_THROW(file = fs->OpenFile(".profile", ec));
  BOOST_REQUIRE(ec == ErrorCode::kSuccess);

  size_t read;
  std::string from_file(to_file.size(), '\0');
  BOOST_CHECK_NO_THROW(read = file->Read(&from_file[0], from_file.size()));
  BOOST_CHECK(read == to_file.size());
  BOOST_CHECK(from_file == to_file);

  BOOST_CHECK_NO_THROW(file->Close());
}

BOOST_FIXTURE_TEST_CASE(read_many_bytes, LoadedFSFixture) {
  BOOST_REQUIRE_NO_THROW(file = fs->OpenFile(".profile", ec));
  BOOST_REQUIRE(ec == ErrorCode::kSuccess);
  std::string to_file(1000000, 'a'); // 1Mb
  size_t written;
  BOOST_REQUIRE_NO_THROW(written = file->Write(to_file.c_str(), to_file.size()));
  BOOST_REQUIRE(written == to_file.size());
  BOOST_REQUIRE_NO_THROW(file->Close());
  BOOST_REQUIRE_NO_THROW(file = fs->OpenFile(".profile", ec));
  BOOST_REQUIRE(ec == ErrorCode::kSuccess);

  size_t read;
  std::string from_file(to_file.size(), '\0');
  BOOST_CHECK_NO_THROW(read = file->Read(&from_file[0], from_file.size()));
  BOOST_CHECK(read == to_file.size());
  BOOST_CHECK(from_file == to_file);

  BOOST_CHECK_NO_THROW(file->Close());
}

BOOST_FIXTURE_TEST_CASE(read_many_bytes_one_read, LoadedFSFixture) {
  BOOST_REQUIRE_NO_THROW(file = fs->OpenFile(".profile", ec));
  BOOST_REQUIRE(ec == ErrorCode::kSuccess);
  std::string to_file;
  for (int i = 0; i < 10; ++i) {
    size_t written;
    to_file += std::string(100000, 'a' + i); // 100Kb
    BOOST_REQUIRE_NO_THROW(written = file->Write(to_file.c_str() + to_file.size() - 100000, 100000));
    BOOST_REQUIRE(written == 100000);
  }
  BOOST_REQUIRE_NO_THROW(file->Close());
  BOOST_REQUIRE_NO_THROW(file = fs->OpenFile(".profile", ec));
  BOOST_REQUIRE(ec == ErrorCode::kSuccess);

  size_t read;
  std::string from_file(to_file.size(), '\0');
  BOOST_CHECK_NO_THROW(read = file->Read(&from_file[0], from_file.size()));
  BOOST_CHECK(read == to_file.size());
  BOOST_CHECK(from_file == to_file);

  BOOST_CHECK_NO_THROW(file->Close());
}

BOOST_FIXTURE_TEST_CASE(read_many_bytes_many_read, LoadedFSFixture) {
  BOOST_REQUIRE_NO_THROW(file = fs->OpenFile(".profile", ec));
  BOOST_REQUIRE(ec == ErrorCode::kSuccess);
  std::string to_file(1000000, 'a'); // 1Mb
  size_t written;
  BOOST_REQUIRE_NO_THROW(written = file->Write(to_file.c_str(), to_file.size()));
  BOOST_REQUIRE(written == to_file.size());
  BOOST_REQUIRE_NO_THROW(file->Close());
  BOOST_REQUIRE_NO_THROW(file = fs->OpenFile(".profile", ec));
  BOOST_REQUIRE(ec == ErrorCode::kSuccess);

  std::string from_file(to_file.size() / 10, '\0');
  for (int i = 0; i < 10; ++i) {
    size_t read;
    BOOST_CHECK_NO_THROW(read = file->Read(&from_file[0], from_file.size()));
    BOOST_CHECK(read == from_file.size());
    BOOST_CHECK(from_file == std::string(from_file.size(), 'a'));
  }

  BOOST_CHECK_NO_THROW(file->Close());
}

BOOST_FIXTURE_TEST_CASE(read_many_bytes_two_files, LoadedFSFixture) {
  FileInterface *file1, *file2;
  BOOST_REQUIRE_NO_THROW(file1 = fs->OpenFile("1", ec));
  BOOST_REQUIRE(ec == ErrorCode::kSuccess);
  BOOST_REQUIRE_NO_THROW(file2 = fs->OpenFile("2", ec));
  BOOST_REQUIRE(ec == ErrorCode::kSuccess);
  std::string to_file1, to_file2; // 1MB
  for (int i = 0; i < 10; ++i) {
    to_file1 += std::string(100000, 'a' + i);
    to_file2 += std::string(100000, 'z' - i);
  }
  size_t written;
  BOOST_REQUIRE_NO_THROW(written = file1->Write(to_file1.c_str(), to_file1.size()));
  BOOST_REQUIRE(written == to_file1.size());
  BOOST_REQUIRE_NO_THROW(written = file2->Write(to_file2.c_str(), to_file2.size()));
  BOOST_REQUIRE(written == to_file2.size());
  BOOST_REQUIRE_NO_THROW(file1->Close());
  BOOST_REQUIRE_NO_THROW(file2->Close());
  BOOST_REQUIRE_NO_THROW(file1 = fs->OpenFile("1", ec));
  BOOST_REQUIRE(ec == ErrorCode::kSuccess);
  BOOST_REQUIRE_NO_THROW(file2 = fs->OpenFile("2", ec));
  BOOST_REQUIRE(ec == ErrorCode::kSuccess);

  for (int i = 0; i < 10; ++i) {
    std::string from_file1(100000, '\0'); // 100Kb
    std::string from_file2(100000, '\0'); // 100Kb
    size_t read;
    BOOST_CHECK_NO_THROW(read = file1->Read(&from_file1[0], from_file1.size()));
    BOOST_CHECK(read == from_file1.size());
    BOOST_CHECK(from_file1 == std::string(100000, 'a' + i));

    BOOST_CHECK_NO_THROW(read = file2->Read(&from_file2[0], from_file2.size()));
    BOOST_CHECK(read == from_file2.size());
    BOOST_CHECK(from_file2 == std::string(100000, 'z' - i));
  }

  BOOST_CHECK_NO_THROW(file1->Close());
  BOOST_CHECK_NO_THROW(file2->Close());
}

BOOST_AUTO_TEST_SUITE_END()
