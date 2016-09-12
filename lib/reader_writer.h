#pragma once

#include <cstdint>
#include <ios>

#include "lib/section.h"

namespace fs {

namespace ffs {

class ReaderWriter {
 public:
  ReaderWriter() = default;
  ReaderWriter(const ReaderWriter&) = delete;
  ReaderWriter(ReaderWriter&&) = default;
  ~ReaderWriter() = default;

  ErrorCode Open(const char* device_path, std::ios_base::openmode mode) {
    device_.open(device_path, mode | std::ios_base::binary);
    return device_.good() ? ErrorCode::kSuccess : ErrorCode::kErrorInputOutput;
  }

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

  size_t Read(uint64_t offset, char* buf, size_t buf_size, ErrorCode& error_code) {
    device_.seekg(offset);
    if (!device_.good()) {
      error_code = ErrorCode::kErrorInputOutput;
      return 0;
    }

    device_.read(buf, buf_size);
    if (!device_.good()) {
      error_code = ErrorCode::kErrorInputOutput;
    return device_.gcount();
  }

  template<typename T,
           typename Distance = std::ptrdiff_t> // TODO void?
  class ReadIterator : public std::iterator<std::input_iterator_tag,
                                            T, Distance, const T*, const T&> {
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

  template<typename T>
  std::tuple<typename ReadIterator<T>, ReadIterator<T>> ReadRange(uint64_t pos, uint64_t len, ErrorCode& error_code) {
    assert(len % sizeof(T) == 0);
    return std::make_tuple(ReadIterator<T>(pos, this, error_code),
                           ReadIterator(pos + len));
  }

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

  size_t Write(uint64_t offset, char* buf, size_t buf_size, ErrorCode& error_code) {
    device_.seekp(offset);
    if (!device_.good()) {
      error_code = ErrorCode::kErrorInputOutput;
      return 0;
    }

    device_.write(buf, buf_size);
    if (!device_.good()) {
      error_code = ErrorCode::kErrorInputOutput;
      return 0;
    }

    return buf_size;
  }

  template<typename T = Section>
  T LoadSection(uint64_t offset, ErrorCode& error_code, Args&&... args) {
    static_assert(std::is_same<T, Section>::value ||
                  std::is_base_of<Section, T>::value, "T must be derived from Section");
    SectionLayout::Header header = Read<SectionLayout::Header>(offset, error_code);
    // TODO check endianness
    return T(offset, header.size, header.next_offset, std::forward<Args>(args)...);
  }

  ErrorCode SaveSection(Section section) {
    SectionLayout::Header header(section.size(), section.next_offset());
    return Write<SectionLayout::Header>(header, section.base_offset());
  }

  std::shared_ptr<Entry> LoadEntry(uint64_t offset, ErrorCode& error_code) {
    EntryLayout::HeaderUnion entry_header = Read<EntryLayout::HeaderUnion>(offset, error_code);
    // TODO check endianness
    switch (static_cast<Entry::Type>(entry_header.common.type)) {
      case Entry::Type::kNone:
        return std::make_shared<NoneEntry>(offset, entry_header.none.head_offset);
      case Entry::Type::kDirectory:
        return std::make_shared<DirectoryEntry>(offset, std::string(entry_header.directory.name,
                                                                    sizeof entry_header.directory.name));
      case Entry::Type::File:
        return std::make_shared<FileEntry>(offset, entry_header.file.size,
                                           std::string(entry_header.file.name,
                                                       sizeof entry_header.file.name));
    }
    return std::make_shared<Entry>();
  }

 private:
  std::fstream device_;
};

}  // namespace ffs

}  // namespace fs
