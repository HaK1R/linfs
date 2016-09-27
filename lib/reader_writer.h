#pragma once

#include <cstddef>
#include <cstdint>
#include <fstream>
#include <ios>
#include <iterator>
#include <memory>
#include <mutex>
#include <type_traits>

#include "fs/error_code.h"
#include "lib/entries/entry.h"
#include "lib/sections/section.h"
#include "lib/utils/byte_order.h"

namespace fs {

namespace linfs {

// TODO? move to util/
class ReaderWriter {
 public:
  ReaderWriter(const char* device_path, std::ios_base::openmode mode);

  // Duplicate the file descriptor and create a new ReaderWriter, which
  // owns that file descriptor.  The created ReaderWriter operates with
  // the same device in the same mode.
  std::unique_ptr<ReaderWriter> Duplicate();

  template<typename T>
  T Read(uint64_t offset) {
    return ReadIntegral<T>(std::is_integral<T>(), offset);
  }
  size_t Read(uint64_t offset, char* buf, size_t buf_size);

  template<typename T, typename U>
  void Write(U&& u, uint64_t offset) {
    WriteIntegral<T>(std::is_integral<T>(), std::forward<U>(u), offset);
  }
  size_t Write(const char* buf, size_t buf_size, uint64_t offset);

  template<typename T>
  class ReadIterator : public std::iterator<std::input_iterator_tag,
                                            T, uint64_t, const T*, const T&> {
    using _Base = std::iterator<std::input_iterator_tag, T, uint64_t, const T*, const T&>;
   public:
    using typename _Base::value_type;
    using typename _Base::reference;
    using typename _Base::pointer;

    ReadIterator(uint64_t position) : position_(position) {}
    ReadIterator(uint64_t position, ReaderWriter* reader)
        : position_(position), reader_(reader) {}

    uint64_t position() const { return position_; }

    bool operator==(const ReadIterator& that) { return position_ == that.position_; }
    bool operator!=(const ReadIterator& that) { return !(*this == that); }
    reference operator*() { ReadValue(); return value_; }
    pointer operator->() { ReadValue(); return &value_; }
    ReadIterator& operator++() { position_ += sizeof(value_type); return *this; }
    ReadIterator operator++(int) { ReadIterator tmp = *this; ++*this; return tmp; }

   private:
    void ReadValue() {
      if (reader_ != nullptr)
        value_ = reader_->Read<value_type>(position_);
    }

    value_type value_;
    uint64_t position_;
    ReaderWriter* reader_ = nullptr;
  };

  template<typename T = Section, typename... Args>
  T LoadSection(uint64_t section_offset, Args&&... args) {
    static_assert(std::is_base_of<Section, T>::value, "T must be derived from Section");
    SectionLayout::Header header = Read<SectionLayout::Header>(section_offset);
    return T(section_offset, ByteOrder::Unpack(header.size),
             ByteOrder::Unpack(header.next_offset), std::forward<Args>(args)...);
  }
  void SaveSection(Section section);

  std::unique_ptr<Entry> LoadEntry(uint64_t entry_offset, char* name_buf = nullptr);

 private:
  template<typename T>
  T ReadIntegral(std::true_type, uint64_t offset) {
    T value = 0;
    Read(offset, reinterpret_cast<char*>(&value), sizeof value);
    return ByteOrder::Unpack(value);
  }
  template<typename T>
  T ReadIntegral(std::false_type, uint64_t offset) {
    T value;
    static_assert(std::is_trivially_copyable<T>::value,
                  "T must be a trivially copyable type");
    Read(offset, reinterpret_cast<char*>(&value), sizeof value);
    return value;
  }
  template<typename T>
  void WriteIntegral(std::true_type, T value, uint64_t offset) {
    value = ByteOrder::Pack(value);
    Write(reinterpret_cast<const char*>(&value), sizeof value, offset);
  }
  template<typename T>
  void WriteIntegral(std::false_type, const T& value, uint64_t offset) {
    static_assert(std::is_trivially_copyable<T>::value,
                  "T must be a trivially copyable type");
    Write(reinterpret_cast<const char*>(&value), sizeof value, offset);
  }

  // File descriptor and its mutex.
  std::fstream device_;
  std::mutex device_mutex_;

  // Required information for Duplicate().
  const std::string device_path_;
  const std::ios_base::openmode device_mode_;
};

}  // namespace linfs

}  // namespace fs
