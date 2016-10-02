#include "lib/entries/entry.h"

#include <cstring>

#include "lib/entries/directory_entry.h"
#include "lib/entries/file_entry.h"
#include "lib/entries/none_entry.h"
#include "lib/entries/symlink_entry.h"
#include "lib/layout/entry_layout.h"
#include "lib/utils/byte_order.h"
#include "lib/utils/format_exception.h"
#include "lib/utils/reader_writer.h"

namespace fs {

namespace linfs {

namespace {

void CopyName(char* dest, const char* src) {
  if (dest != nullptr) {
    strncpy(dest, src, kNameMax);
    dest[kNameMax] = '\0';
  }
}

}  // namespace

std::unique_ptr<Entry> Entry::Load(uint64_t entry_offset, ReaderWriter* reader,
                                   char* name_buf) {
  EntryLayout::HeaderStorage header_storage;
  reader->Read(entry_offset, reinterpret_cast<char*>(&header_storage), sizeof header_storage);

  EntryLayout::HeaderUnion* header = reinterpret_cast<EntryLayout::HeaderUnion*>(&header_storage);
  switch (static_cast<Entry::Type>(header->none.common.type)) {
    case Entry::Type::kNone:
      return std::make_unique<NoneEntry>(entry_offset,
                                         ByteOrder::Unpack(header->none.head_offset));
    case Entry::Type::kDirectory:
      CopyName(name_buf, header->directory.name);
      return std::make_unique<DirectoryEntry>(entry_offset);
    case Entry::Type::kFile:
      CopyName(name_buf, header->file.name);
      return std::make_unique<FileEntry>(entry_offset,
                                         ByteOrder::Unpack(header->file.size));
    case Entry::Type::kSymlink:
      CopyName(name_buf, header->symlink.name);
      return std::make_unique<SymlinkEntry>(entry_offset);
  }

  throw FormatException();  // unknown entry type
}

Section Entry::CursorToSection(uint64_t& cursor, ReaderWriter* reader,
                               uint64_t start_position, bool check_cursor) {
  Section section = Section::Load(section_offset(), reader);
  cursor += start_position;
  while (cursor >= section.data_size() && section.next_offset()) {
    cursor -= section.data_size();
    section = Section::Load(section.next_offset(), reader);
  }

  if (check_cursor && cursor > section.data_size())
    throw FormatException();  // cursor is greater than entry's size

  return section;
}

}  // namespace linfs

}  // namespace fs
