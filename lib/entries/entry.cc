#include "lib/entries/entry.h"

#include <cstring>

#include "lib/entries/directory_entry.h"
#include "lib/entries/file_entry.h"
#include "lib/entries/none_entry.h"
#include "lib/layout/entry_layout.h"
#include "lib/utils/byte_order.h"
#include "lib/utils/format_exception.h"
#include "lib/utils/reader_writer.h"

namespace fs {

namespace linfs {

std::unique_ptr<Entry> Entry::Load(uint64_t entry_offset, ReaderWriter* reader,
                                   char* name_buf) {
  EntryLayout::HeaderUnion entry_header = reader->Read<EntryLayout::HeaderUnion>(entry_offset);

  switch (static_cast<Entry::Type>(entry_header.common.type)) {
    case Entry::Type::kNone:
      return std::make_unique<NoneEntry>(entry_offset,
                                         ByteOrder::Unpack(entry_header.none.head_offset));
    case Entry::Type::kDirectory:
      if (name_buf != nullptr) {
        strncpy(name_buf, entry_header.directory.name, sizeof entry_header.directory.name);
        name_buf[sizeof entry_header.directory.name] = '\0';
      }
      return std::make_unique<DirectoryEntry>(entry_offset);
    case Entry::Type::kFile:
      if (name_buf != nullptr) {
        strncpy(name_buf, entry_header.file.name, sizeof entry_header.file.name);
        name_buf[sizeof entry_header.file.name] = '\0';
      }
      return std::make_unique<FileEntry>(entry_offset, ByteOrder::Unpack(entry_header.file.size));
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
