#pragma once

#include <type_traits>

namespace fs {

namespace linfs {

#define STATIC_ASSERT_STANDARD_LAYOUT(Type) \
  static_assert(std::is_standard_layout<Type>::value, \
                #Type " isn't a standard-layout type")

#define STATIC_ASSERT_TRIVIALLY_COPYABLE(Type) \
  static_assert(std::is_trivially_copyable<Type>::value, \
                #Type " isn't a trivially copyable type")

#define STATIC_ASSERT_STANDARD_LAYOUT_AND_TRIVIALLY_COPYABLE(Type) \
  STATIC_ASSERT_STANDARD_LAYOUT(Type); \
  STATIC_ASSERT_TRIVIALLY_COPYABLE(Type)

}  // namespace linfs

}  // namespace fs
