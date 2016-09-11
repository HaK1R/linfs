#pragma once

#include <cstddef>
#include <cstdint>

#include <iterator>

namespace fs {

namespace ffs {


class SectionDirectory : Section {
 public:
  class Iterator : public std::iterator<std::input_iterator_tag, uint64_t> {
   public:
    Iterator()
        : offset_(0), end_(0), error_code(nullptr), value_(0) {}
    Iterator(uint64_t start, uint64_t end, ErrorCode& error_code)
        : offset_(start < end ? start : 0), end_(end),
          error_code_(&error_code), value_(0) {
      ++*this;
    }
    ~Iterator();
    bool operator==(const Iterator& that) {
      return offset_ == that.offset;
    }

    uint64_t operator*() const { return value_; }
    const uint64_t* operator->() const { return &value_; }

    Iterator& operator++() {
      if (offset_ == end_)
        offset_ = 0;
      else if (offset_) {
        *error_code = device.Read<uint64_t>(value_, offset_);
        if (*error_code != ErrorCode::kSuccess)
          offset_ = 0;
        else
          offset_ += sizeof value_;
      }
      return *this;
    }
    void operator++(int) {
      ++*this;
    }

   private:
    uint64_t offset_;
    uint64_t end_;
    ErrorCode* error_code_;
    uint64_t value_;
  };

  using Section::Section;
  SectionDirectory() = delete;
  ~SectionDirectory() = default;

  ErrorCode AddEntry(uint64_t entry_offset, ReaderWriter* reader_writer, uint64_t start_position = 0);
  bool RemoveEntry(uint64_t entry_offset, ReaderWriter* reader_writer, ErrorCode& error_code, uint64_t start_position = 0);
};

}  // namespace ffs

}  // namespace fs
