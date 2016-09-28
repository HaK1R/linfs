#include "lib/utils/reader_writer.h"

#include <ios>
#include <mutex>
#include <system_error>

#include "lib/utils/byte_order.h"
#include "lib/utils/format_exception.h"

namespace fs {

namespace linfs {

ReaderWriter::ReaderWriter(const char* device_path, std::ios_base::openmode mode)
    : device_path_(device_path), device_mode_(mode | std::ios_base::binary) {
  // Disable buffering for the stream.
  device_.rdbuf()->pubsetbuf(nullptr, 0);

  device_.open(device_path_.c_str(), device_mode_);
  if (!device_.is_open() || !device_.good())
    throw std::ios_base::failure("open");
}

std::unique_ptr<ReaderWriter> ReaderWriter::Duplicate() {
  std::ios_base::openmode clear_mask = std::ios_base::binary |
                                       std::ios_base::in | std::ios_base::out;

  return std::make_unique<ReaderWriter>(device_path_.c_str(),
                                        device_mode_ & clear_mask);
}

size_t ReaderWriter::Read(uint64_t offset, char* buf, size_t buf_size) {
  std::lock_guard<std::mutex> lock(device_mutex_);

  device_.seekg(offset);
  if (device_.good())
    device_.read(buf, buf_size);
  if (device_.eof())
    throw FormatException();  // no data to read
  if (!device_.good())
    throw std::ios_base::failure("read", std::make_error_code(std::errc::io_error));
  return buf_size;
}

size_t ReaderWriter::Write(const char* buf, size_t buf_size, uint64_t offset) {
  std::lock_guard<std::mutex> lock(device_mutex_);

  device_.seekp(offset);
  device_.write(buf, buf_size);
  if (!device_.good())
    throw std::ios_base::failure("write", std::make_error_code(std::errc::io_error));
  return buf_size;
}

}  // namespace linfs

}  // namespace fs
