#include "lib/util/exception_handler.h"

#include <cassert>
#include <ios>
#include <new>

#include "lib/util/format_error.h"

namespace fs {

namespace linfs {

ErrorCode ExceptionHandler::ToErrorCode(std::exception_ptr exception_pointer) noexcept {
  try {
    std::rethrow_exception(exception_pointer);
  } catch (const std::bad_alloc&) {
    return ErrorCode::kErrorNoMemory;
  } catch (const std::ios_base::failure&) {
    return ErrorCode::kErrorInputOutput;
  } catch (const FormatError&) {
    return ErrorCode::kErrorFormat;
  } catch (...) {
    assert(0 && "caught unknown exception");
    return ErrorCode::kErrorUnknown;
  }
}

}  // namespace linfs

}  // namespace fs
