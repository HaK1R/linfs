// error_code.h -- Error codes

#pragma once

#include <cstdint>

namespace fs {

enum class ErrorCode : uint8_t {
  // Common:
  kSuccess = 0,             // ok: no error
  kErrorUnknown,            // error:  (sorry, we don't know it)
  kErrorNoMemory,           // error: no memory
  kErrorExists,             // error: file or directory already exists
  kErrorNotFound,           // error: file or directory not found
  kErrorBusy,               // error: resource is busy
  kErrorNotSupported,       // error: operation not supported
  kErrorNameTooLong,        // error: file name exceeds kNameMax, or
                            //        path exceeds kPathMax

  // Device and system errors:
  kErrorDeviceUnknown,      // error: unknown error while opening a device
  kErrorInvalidSignature,   // error: invalid device signature
  kErrorFormat,             // error: invalid format, device is corrupted
  kErrorInputOutput,        // error: input/output system error

  // Directory errors:
  kErrorIsDirectory,        // error: it is a directory
  kErrorNotDirectory,       // error: not a directory
  kErrorDirectoryNotEmpty,  // error: directory not empty
};

}  // namespace fs
