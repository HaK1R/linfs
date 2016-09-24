#include "lib/reader_writer.h"

#include <cstring>
#include <mutex>
#include <string>

#include "lib/entries/directory_entry.h"
#include "lib/entries/entry.h"
#include "lib/entries/file_entry.h"
#include "lib/entries/none_entry.h"
#include "lib/layout/entry_layout.h"
#include "lib/layout/section_layout.h"
#include "lib/utils/format_exception.h"

namespace fs {

namespace linfs {

ErrorCode ReaderWriter::Open(const char* device_path, std::ios_base::openmode mode) {
  device_.open(device_path, mode | std::ios_base::binary);
  return device_.is_open() ? ErrorCode::kSuccess : ErrorCode::kErrorDeviceUnknown;
}

size_t ReaderWriter::Read(uint64_t offset, char* buf, size_t buf_size) {
  std::lock_guard<std::mutex> lock(device_mutex_);

  device_.seekg(offset);
  if (device_.good())
    device_.read(buf, buf_size);
  if (device_.eof())
    throw FormatException();  // no data to read
  if (!device_.good())
    throw std::ios_base::failure("");
  return buf_size;
}

size_t ReaderWriter::Write(const char* buf, size_t buf_size, uint64_t offset) {
  std::lock_guard<std::mutex> lock(device_mutex_);

  device_.seekp(offset);
  device_.write(buf, buf_size);
  if (!device_.good())
    throw std::ios_base::failure("");
  return buf_size;
}

void ReaderWriter::SaveSection(Section section) {
  SectionLayout::Header header = {section.size(), section.next_offset()};
  Write<SectionLayout::Header>(header, section.base_offset());
}

std::unique_ptr<Entry> ReaderWriter::LoadEntry(uint64_t entry_offset, char* name_buf) {
  EntryLayout::HeaderUnion entry_header = Read<EntryLayout::HeaderUnion>(entry_offset);

  // TODO check endianness
  switch (static_cast<Entry::Type>(entry_header.common.type)) {
    case Entry::Type::kNone:
      return std::make_unique<NoneEntry>(entry_offset, uint64_t(entry_header.none.head_offset));
    case Entry::Type::kDirectory:
      if (name_buf != nullptr)
        strcpy(name_buf, std::string(entry_header.directory.name, sizeof entry_header.directory.name).c_str());
      return std::make_unique<DirectoryEntry>(entry_offset);
    case Entry::Type::kFile:
      // TODO what's better?
      if (name_buf != nullptr) {
        strncpy(name_buf, entry_header.file.name, sizeof entry_header.file.name);
        name_buf[sizeof entry_header.file.name] = '\0';
      }
      return std::make_unique<FileEntry>(entry_offset, uint64_t(entry_header.file.size));
  }

  throw FormatException();  // unknown entry type
}

}  // namespace linfs

}  // namespace fs
