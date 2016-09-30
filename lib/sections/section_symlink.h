#pragma once

#include "lib/sections/section_file.h"

namespace fs {

namespace linfs {

// They are very similar.  Probably SectionFile should be refactored and
// its common code must be moved to some general class.
using SectionSymlink = SectionFile;

}  // namespace linfs

}  // namespace fs
