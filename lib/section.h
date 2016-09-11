#pragma once

#include <cstdint>

namespace fs {

namespace ffs {

class Section {
 public:
  Section(uint64_t base_offset, uint64_t size, uint64_t next_offset)
      : base_offset_(base_offset), size_(size), next_offset_(next_offset) {}

  Section(const Section& section) = delete;
  void operator=(const Section& section) = delete;

  uint64_t base_offset() const { return base_offset_; }
  uint64_t size() const { return size_; }
  uint64_t next_offset() const { return next_offset_; }
  uint64_t data_offset() const { return base_offset() + sizeof(SectionLayout::Header); }

  ErrorCode Clear(ReaderWriter* reader_writer) {
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

  ErrorCode SetSize(uint64_t size, ReaderWriter* reader_writer) {
    ErrorCode error_code = reader_writer->Write<uint64_t>(size,
                                          base_offset() + offsetof(SectionLayout::Header, size));
    if (error_code == ErrorCode::kSuccess)
      size_ = size;
    return error_code;
  }

  ErrorCode SetNext(uint64_t next_offset, ReaderWriter* reader_writer) {
    ErrorCode error_code = reader_writer->Write<uint64_t>(next_offset,
                                          base_offset() + offsetof(SectionLayout::Header, next_offset));
    if (error_code == ErrorCode::kSuccess)
      next_offset_ = next_offset;
    return error_code;
  }

 private:
  uint64_t base_offset_;
  uint64_t size_;
  uint64_t next_offset_;
};

}  // namespace ffs

}  // namespace fs
