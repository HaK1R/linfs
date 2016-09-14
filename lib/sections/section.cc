#include "lib/section.h"

#include "lib/layout/section_layout.h"

namespace fs {

namespace linfs {

ErrorCode Section::Clear(ReaderWriter* reader_writer) {
  uint64_t offset = section.base_offset() + sizeof(SectionLayout::Header);
  uint64_t size = section.size() - sizeof(SectionLayout::Header);
  while (size != 0) {
    ErrorCode error_code = reader_writer->Write<uint8_t>(0, offset);
    if (error_code != ErrorCode::kSuccess)
      return error_code;
    --size;
    ++offset;
  }
  return ErrorCode::kSuccess;
}

ErrorCode Section::SetSize(uint64_t size, ReaderWriter* reader_writer) {
  ErrorCode error_code = reader_writer->Write<uint64_t>(size,
                                        base_offset() + offsetof(SectionLayout::Header, size));
  if (error_code == ErrorCode::kSuccess)
    size_ = size;
  return error_code;
}

ErrorCode Section::SetNext(uint64_t next_offset, ReaderWriter* reader_writer) {
  ErrorCode error_code = reader_writer->Write<uint64_t>(next_offset,
                                        base_offset() + offsetof(SectionLayout::Header, next_offset));
  if (error_code == ErrorCode::kSuccess)
    next_offset_ = next_offset;
  return error_code;
}

}  // namespace linfs

}  // namespace fs
