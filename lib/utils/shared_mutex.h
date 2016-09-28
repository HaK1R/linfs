#pragma once

#include <shared_mutex>

namespace fs {

namespace linfs {

// Unfortunately |std::shared_mutex| will be available only since C++17,
// so we defined this alias to hide unused features of this timed mutex.
using SharedMutex = std::shared_timed_mutex;

}  // namespace linfs

}  // namespace fs
