// limits.h -- Filesystem related limits

#pragma once

namespace fs {

// Maximum length of a name in filesystem (not including terminating null).
constexpr size_t kNameMax = 256; // equivalent to NAME_MAX

// Maximum path length
constexpr size_t kPathMax = 1024; // equivalent to PATH_MAX

// Maximum depth of symbolic links.
constexpr int kSymlinkDepthMax = 100;

}  // namespace fs
