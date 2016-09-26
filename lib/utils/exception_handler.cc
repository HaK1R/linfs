#include "lib/utils/exception_handler.h"

#include <cassert>
#include <ios>
#include <new>
#include <system_error>

#include "lib/utils/format_exception.h"

namespace fs {

namespace linfs {

ErrorCode ExceptionHandler::ToErrorCode(std::exception_ptr exception_pointer) noexcept {
  try {
    std::rethrow_exception(exception_pointer);
  }
  catch (const std::bad_alloc&) {
    return ErrorCode::kErrorNoMemory;
  }
  catch (const std::ios_base::failure& e) {
    if (e.code() == std::make_error_code(std::errc::io_error))
      return ErrorCode::kErrorInputOutput;
    return ErrorCode::kErrorDeviceUnknown;
  }
  catch (const FormatException&) {
    return ErrorCode::kErrorFormat;
  }
  catch (...) {
    assert(0 && "caught unknown exception");
    return ErrorCode::kErrorUnknown;
  }
}

}  // namespace linfs

}  // namespace fs
