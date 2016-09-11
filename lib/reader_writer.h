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

  template<typename T>
  ReadIterator<T> ReadRange(uint64_t pos, uint64_t len, ErrorCode& error_code);

  template<typename T>
  ErrorCode Write(T value, uint64_t offset) {
    device_.seekp(offset);
    if (!device_.good()) {
      error_code = ErrorCode::kErrorInputOutput;
      return value;
    }

    // TODO check endianness
    device_.write(&value, sizeof value);
    if (!device_.good()) {
      error_code = ErrorCode::kErrorInputOutput;
      return value;
    }

    error_code = ErrorCode::kSuccess;
  }

  template<typename T = Section>
  T LoadSection(uint64_t offset, ErrorCode& error_code) {
    static_assert(std::is_same<T, Section>::value ||
                  std::is_base_of<Section, T>::value, "T must be derived from Section");
    SectionLayout::Header header = Read<SectionLayout::Header>(offset, error_code);
    // TODO check endianness
    return T(offset, header.size, header.next_offset);
  }

  ErrorCode SaveSection(Section section, ErrorCode& error_code) {
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
        return std::make_shared<FileEntry>(offset, std::string(entry_header.file.name,
                                                               sizeof entry_header.file.name));
    }
    return std::make_shared<Entry>();
  }

 private:
  std::fstream device_;
};

}  // namespace ffs

}  // namespace fs
