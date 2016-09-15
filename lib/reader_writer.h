#pragma once

#include <cstddef>
#include <cstdint>
#include <fstream>
#include <ios>
#include <iterator>
#include <memory>
#include <type_traits>

#include "fs/error_code.h"
#include "lib/entries/entry.h"
#include "lib/sections/section.h"

namespace fs {

namespace linfs {

class ReaderWriter {
 public:
  ReaderWriter() = default;
  ReaderWriter(const ReaderWriter&) = delete;
  ReaderWriter(ReaderWriter&&) = default;
  ~ReaderWriter() = default;

  ErrorCode Open(const char* device_path, std::ios_base::openmode mode);

  template<typename T>
  T Read(uint64_t offset, ErrorCode& error_code) {
    // TODO Check that it's for ints
    T value = 0;

    device_.seekg(offset);
    if (!device_.good()) {
      error_code = ErrorCode::kErrorInputOutput;
      return value;
    }

    // TODO check endianness
    device_.read(value, sizeof value);
    if (!device_.good()) {
      error_code = ErrorCode::kErrorInputOutput;
      return value;
    }

    error_code = ErrorCode::kSuccess;
    return value;
  }
  size_t Read(uint64_t offset, char* buf, size_t buf_size, ErrorCode& error_code);

  template<typename T>
  // TODO? Distance=uint64_t
  class ReadIterator : public std::iterator<std::input_iterator_tag,
                                            T, uint64_t, const T*, const T&> {
   public:
    ReadIterator(uint64_t position) : position_(position) {}
    ReadIterator(uint64_t position, ReaderWriter* reader, ErrorCode& error_code)
        : position_(position - sizeof(value_type)), reader_(reader), error_code_(&error_code) {
      ++*this;
    }
    bool operator==(const ReadIterator& that) {
      return position_ == that.position_;
    }
    reference operator*() const { return value_; }
    pointer operator->() const { return &value_; }
    ReadIterator& operator++() {
      if (reader_) {
        position_ += sizeof(value_type);
        value_ = reader_->Read<value_type>(position_, *error_code);
        if (*error_code != ErrorCode::kSuccess)
          reader_ = nullptr;
      }
      return *this;
    }
    ReaderIterator operator++(int) {
      ReaderIterator tmp = *this;
      ++*this;
      return tmp;
    }

   private:
    value_type value_;
    uint64_t position_;
    ReaderWriter* reader_ = nullptr;
    ErrorCode* error_code_ = nullptr;
  };

  // TODO rename to position
  template<typename T>
  ErrorCode Write(T value, uint64_t offset) {
    device_.seekp(offset);
    if (!device_.good())
      return ErrorCode::kErrorInputOutput;

    // TODO check endianness
    device_.write(&value, sizeof value);
    if (!device_.good())
      return ErrorCode::kErrorInputOutput;

    return ErrorCode::kSuccess;
  }
  size_t Write(const char* buf, size_t buf_size, uint64_t offset, ErrorCode& error_code);

  //template<typename T>
  //std::tuple<typename ReadIterator<T>, ReadIterator<T>> ReadRange(uint64_t pos, uint64_t len, ErrorCode& error_code) {
  //  assert(len % sizeof(T) == 0);
  //  return std::make_tuple(ReadIterator<T>(pos, this, error_code),
  //                         ReadIterator(pos + len));
  //}

  template<typename T = Section>
  T LoadSection(uint64_t offset, ErrorCode& error_code, Args&&... args) {
    static_assert(std::is_same<T, Section>::value ||
                  std::is_base_of<Section, T>::value, "T must be derived from Section");
    SectionLayout::Header header = Read<SectionLayout::Header>(offset, error_code);
    return T(offset, header.size, header.next_offset, std::forward<Args>(args)...);
  }
  ErrorCode SaveSection(Section section);

  std::unique_ptr<Entry> LoadEntry(uint64_t offset, ErrorCode& error_code);

 private:
  std::fstream device_;
};

}  // namespace linfs

}  // namespace fs
