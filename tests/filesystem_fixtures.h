#pragma once

#include <boost/filesystem.hpp>
#include <boost/test/unit_test.hpp>

#include "fs/error_code.h"
#include "fs/linfs_factory.h"

struct DefaultFSFixture {
  fs::ErrorCode ec;
  fs::IFile* file;
  fs::IFileSystem* fs;
};

struct CreatedFSFixture : DefaultFSFixture {
  CreatedFSFixture() {
    BOOST_REQUIRE_NO_THROW(fs = fs::CreateLinFS(&ec));
    BOOST_REQUIRE(fs != nullptr && ec == fs::ErrorCode::kSuccess);
    unique_name = boost::filesystem::unique_path();
    BOOST_REQUIRE(!boost::filesystem::exists(unique_name));
  }
  ~CreatedFSFixture() {
    BOOST_REQUIRE_NO_THROW(fs->Release());
    boost::filesystem::remove(unique_name);
  }
  const char* device_path() const { return unique_name.c_str(); }
 private:
  boost::filesystem::path unique_name;
};

struct FormattedFSFixture : CreatedFSFixture {
  FormattedFSFixture(fs::IFileSystem::ClusterSize cluster_size = fs::IFileSystem::ClusterSize::k1KB) {
    BOOST_REQUIRE_NO_THROW(ec = fs->Format(device_path(), cluster_size));
    BOOST_REQUIRE(ec == fs::ErrorCode::kSuccess);
  }
};

struct LoadedFSFixture : FormattedFSFixture {
  LoadedFSFixture() {
    BOOST_REQUIRE_NO_THROW(ec = fs->Load(device_path()));
    BOOST_REQUIRE(ec == fs::ErrorCode::kSuccess);
  }
};
