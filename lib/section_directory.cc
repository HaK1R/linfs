#include "lib/section_directory.h"

namespace fs {

namespace ffs {

SectionDirectory::~SectionDirectory() override {}

int SectionDirectory::AddEntry(uint64_t entry_offset) {
  ;
}

int SectionDirectory::RemoveEntry(uint64_t entry_offset) {
  ;
}

uint64_t SectionDirectory::FindEntryByName(const char *entry_name) {
    ;
}

}  // namespace ffs

}  // namespace fs
