#include "lib/entries/directory_entry.h"

namespace fs {

namespace ffs {

std::shared_ptr<DirectoryEntry> DirectoryEntry::Create(const Section& section,
                                                       ErrorCode& error_code,
                                                       const char *name) {
  error_code = device.Write(SectionLayout::Header(section.size(), section.base_offset()), section.base_offset());
  if (error_code != ErrorCode::kSuccess)
    return std::shared_ptr<DirectoryEntry>();

  error_code = device.Write(EntryLayout::Header(Entry::Type::kDirectory, name), section.base_offset() + sizeof(SectionLayout::Header));
  if (error_code != ErrorCode::kSuccess)
    return std::shared_ptr<DirectoryEntry>();

  return make_shared<DirectoryEntry>(section.base_offset());
}

DirectoryEntry::DirectoryEntry(uint64_t base_offset)
  : Section(Type::kDirectory, base_offset) {}

DirectoryEntry::~DirectoryEntry() override {}

ErrorCode DirectoryEntry::AddEntry(std::shared_ptr<Entry> entry) {
  ErrorCode error_code;

  SectionDirectory sec_dir = device.LoadSection<SectionDirectory>(base_offset(), error_code);
  if (error_code != ErrorCode::kSuccess)
    return error_code;

  error_code = sec_dir->AddEntry(entry->base_offset(), EntryLayout::kHeaderDirectorySize);
  while (error_code == ErrorCode::kErrorNoMemory && sec_dir->next_offset()) {
    sec_dir = device.LoadSection<SectionDirectory>(sec_dir->next_offset(), error_code);
    if (error_code != ErrorCode::kSuccess)
      return error_code;

    error_code = sec_dir->AddEntry(entry->base_offset());
  }

  if (error_code != ErrorCode::kErrorNoMemory)
    return error_code; // kSuccess or any of kError*

  SectionDirectory next_sec_dir = device.AllocateSection<SectionDirectory>(error_code);
  if (error_code != ErrorCode::kSuccess)
    return error_code;

  error_code = next_sec_dir.FillZero();
  if (error_code == ErrorCode::kSuccess) {
    error_code = next_sec_dir.AddEntry(entry->base_offset());
    if (error_code != ErrorCode::kSuccess) {
      device.ReleaseSection(next_sec_dir);
      return error_code;
    }
  }

  // Update next directory entry stored in |sec_dir.next_offset()|
  error_code = device.Write<uint64_t>(next_sec_dir.base_offset(), sec_dir->base_offset() + offsetof(SectionLayout::Header, next_offset));
  if (error_code != ErrorCode::kSuccess)
    device.ReleaseSection(next_sec_dir);

  return error_code;
}

ErrorCode DirectoryEntry::RemoveEntry(std::shared_ptr<Entry> entry) {
  ErrorCode error_code;
  uint64_t prev_offset = 0;

  SectionDirectory sec_dir = device.LoadSection<SectionDirectory>(base_offset(), error_code);
  if (error_code != ErrorCode::kSuccess)
    return error_code;

  error_code = sec_dir->RemoveEntry(entry->base_offset(), EntryLayout::kHeaderDirectorySize);
  while (error_code == ErrorCode::kErrorNoMemory && sec_dir->next_offset()) {
    prev_offset = sec_dir->base_offset();
    sec_dir = device.LoadSection<SectionDirectory>(sec_dir->next_offset(), error_code);
    if (error_code != ErrorCode::kSuccess)
      return error_code;

    error_code = sec_dir->RemoveEntry(entry->base_offset());
  }

  if (error_code != ErrorCode::kSuccess)
    return error_code;

  // TODO: use Section::kOffsetNone?
  if (prev_offset && !sec_dir->HasEntries())
    device.UnlinkAndReleaseSection(sec_dir, prev_offset);

  return ErrorCode::kSuccess;
}

std::shared_ptr<Entry> DirectoryEntry::FindEntryByName(const char *entry_name, ErrorCode& error_code) {
  SectionDirectory sec_dir = device.LoadSection<SectionDirectory>(base_offset(), error_code);
  if (error_code != ErrorCode::kSuccess)
    return std::shared_ptr<Entry>();

  uint64_t start_offset = sizeof(SectionLayout::Header) + EntryLayout::kHeaderDirectorySize;

  while (1) {
    for (SectionDirectory::Iterator it(sec_dir.base_offset() + start_offset,
                                       sec_dir.base_offset() + sec_dir.size(), error_code);
         error_code == ErrorCode::kSuccess && it != Iterator(); ++it) {
      uint64_t it_offset = *it;
      char it_name[kNameMax + 1];
      std::shared_ptr<Entry> entry = device.LoadEntry(it_offset, it_name, error_code);
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

    sec_dir = device.LoadSection<SectionDirectory>(sec_dir->next_offset(), error_code);
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
