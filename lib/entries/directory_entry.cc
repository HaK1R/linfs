#include "lib/entries/directory_entry.h"

namespace fs {

namespace ffs {

std::shared_ptr<DirectoryEntry> DirectoryEntry::Create(const Section& section,
                                                       ReaderWriter* writer,
                                                       ErrorCode& error_code,
                                                       const char *name) {
  error_code = device.Write(EntryLayout::Header(Entry::Type::kDirectory, name), section.data_offset());
  if (error_code != ErrorCode::kSuccess)
    return std::shared_ptr<DirectoryEntry>();

  return make_shared<DirectoryEntry>(section.data_offset());
}

DirectoryEntry::DirectoryEntry(uint64_t base_offset)
  : Section(Type::kDirectory, base_offset) {}

DirectoryEntry::~DirectoryEntry() override {}

ErrorCode DirectoryEntry::AddEntry(std::shared_ptr<Entry> entry,
                                   ReaderWriter* reader_writer) {
  ErrorCode error_code;

  SectionDirectory sec_dir = reader_writer->LoadSection<SectionDirectory>(section_offset(), error_code);
  if (error_code != ErrorCode::kSuccess)
    return error_code;

  error_code = sec_dir.AddEntry(entry->base_offset(), reader_writer, sizeof(EntryLayout::DirectoryHeader));
  while (error_code == ErrorCode::kErrorNoMemory && sec_dir.next_offset()) {
    sec_dir = reader_writer->LoadSection<SectionDirectory>(sec_dir.next_offset(), error_code);
    if (error_code != ErrorCode::kSuccess)
      return error_code;

    error_code = sec_dir.AddEntry(entry->base_offset(), reader_writer);
  }

  if (error_code != ErrorCode::kErrorNoMemory)
    return error_code; // kSuccess or any of kError*

  SectionDirectory next_sec_dir = device.AllocateSection<SectionDirectory>(error_code);
  if (error_code != ErrorCode::kSuccess)
    return error_code;

  error_code = next_sec_dir.Clear(reader_writer);
  if (error_code != ErrorCode::kSuccess) {
    device.ReleaseSection(next_sec_dir);
    return error_code;
  }

  error_code = next_sec_dir.AddEntry(entry->base_offset(), reader_writer);
  if (error_code != ErrorCode::kSuccess) {
    device.ReleaseSection(next_sec_dir);
    return error_code;
  }

  // Update next directory entry stored in |sec_dir.next_offset()|
  error_code = sec_dir.SetNext(next_sec_dir.base_offset(), reader_writer);
  if (error_code != ErrorCode::kSuccess)
    device.ReleaseSection(next_sec_dir);

  return error_code;
}

ErrorCode DirectoryEntry::RemoveEntry(std::shared_ptr<Entry> entry, ReaderWriter* reader_writer) {
  ErrorCode error_code;

  SectionDirectory sec_dir = reader_writer->LoadSection<SectionDirectory>(section_offset(), error_code);
  if (error_code != ErrorCode::kSuccess)
    return error_code;

  sec_dir.RemoveEntry(entry->base_offset(), reader_writer, error_code, sizeof(EntryLayout::DirectoryHeader));
  while (error_code == ErrorCode::kErrorNotFound && sec_dir.next_offset()) {
    SectionDirectory prev_sec_dir = sec_dir;
    sec_dir = reader_writer->LoadSection<SectionDirectory>(sec_dir.next_offset(), error_code);
    if (error_code != ErrorCode::kSuccess)
      return error_code;

    bool has_entries = sec_dir.RemoveEntry(entry->base_offset(), reader_writer, error_code);
    if (error_code == ErrorCode::kSuccess && !has_entries) {
      if (prev_sec_dir.SetNext(sec_dir.next_offset(), reader_writer) != ErrorCode::kSuccess)
        device.ReleaseSection(next_sec_dir);
      break;
    }
  }

  return error_code;
}

std::shared_ptr<Entry> DirectoryEntry::FindEntryByName(const char *entry_name, ReaderWriter* reader_writer, ErrorCode& error_code) {
  SectionDirectory sec_dir = reader_writer->LoadSection<SectionDirectory>(base_offset(), error_code);
  if (error_code != ErrorCode::kSuccess)
    return std::shared_ptr<Entry>();

  uint64_t start_offset = sizeof(SectionLayout::Header) + EntryLayout::kHeaderDirectorySize;

  while (1) {
    for (SectionDirectory::Iterator it(sec_dir.base_offset() + start_offset,
                                       sec_dir.base_offset() + sec_dir.size(), error_code);
         error_code == ErrorCode::kSuccess && it != Iterator(); ++it) {
      uint64_t it_offset = *it;
      char it_name[kNameMax + 1];
      std::shared_ptr<Entry> entry = reader_writer->LoadEntry(it_offset, it_name, error_code);
      if (error_code != ErrorCode::kSuccess)
        break;

      if (!strcmp(entry_name, it_name))
        return entry;
    }

    if (error_code != ErrorCode::kSuccess)
      return error_code;

    if (!sec_dir.next_offset()) {
      error_code = ErrorCode::kErrorNotFound;
      return std::shared_ptr<Entry>();
    }

    sec_dir = reader_writer->LoadSection<SectionDirectory>(sec_dir->next_offset(), error_code);
    if (error_code != ErrorCode::kSuccess)
      return std::shared_ptr<Entry>();

    start_offset = sizeof(SectionLayout::Header);
  }

  //SectionDirectory dir;
  //ec = device.LoadSection(section_offset(), dir);

  //while (1) {
  //  for (auto entry_offset : dir->entries_offsets) {
  //  }
  //  if (!dir->HasNext())
  //    return not_found;
  //  ec = device.LoadSection(dir.next_offset(), dir);
  //  if (ec != ok)
  //    return ec;
  //}
}

ErrorCode DirectoryEntry::GetNextEntryName(const char *prev, char* next_buf) {
  // TODO make it metter!
  ErrorCode error_code;

  shared_ptr<Entry> prev_entry = FindEntryByName(prev, error_code);
  // TODO Rule: entry != nullptr <=> error_code == ErrorCode::kSuccess
  if (!prev_entry)
    return error_code;

  // TODO get next
  return error_code;
}

}  // namespace ffs

}  // namespace fs
