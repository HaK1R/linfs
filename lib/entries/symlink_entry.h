#pragma once

#include <cstdint>
#include <memory>

#include "lib/entries/entry.h"
#include "lib/section_allocator.h"
#include "lib/utils/path.h"
#include "lib/utils/reader_writer.h"

namespace fs {

namespace linfs {

class SymlinkEntry : public Entry {
 public:
  static std::unique_ptr<SymlinkEntry> Create(uint64_t entry_offset,
                                              uint64_t entry_size,
                                              ReaderWriter* reader_writer,
                                              const char* name,
                                              const char* target,
                                              SectionAllocator* allocator);

  SymlinkEntry(uint64_t base_offset)
      : Entry(Type::kSymlink, base_offset) {}
  ~SymlinkEntry() override = default;

  Path GetTarget(ReaderWriter* reader);

 private:
  void SetTarget(const char* target, ReaderWriter* reader_writer,
                 SectionAllocator* allocator);
};

}  // namespace linfs

}  // namespace fs
