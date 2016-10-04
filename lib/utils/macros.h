#pragma once

#include <type_traits>

namespace fs {

namespace linfs {

// Shortcuts for static_assert(s).
#define STATIC_ASSERT_BASE_OF(Base, Derived) \
  static_assert(std::is_base_of<Base, Derived>::value, \
                #Base " isn't a base type of " #Derived)

#define STATIC_ASSERT_STANDARD_LAYOUT(Type) \
  static_assert(std::is_standard_layout<Type>::value, \
                #Type " isn't a standard-layout type")

#define STATIC_ASSERT_TRIVIALLY_COPYABLE(Type) \
  static_assert(std::is_trivially_copyable<Type>::value, \
                #Type " isn't a trivially copyable type")

#define STATIC_ASSERT_STANDARD_LAYOUT_AND_TRIVIALLY_COPYABLE(Type) \
  STATIC_ASSERT_STANDARD_LAYOUT(Type); \
  STATIC_ASSERT_TRIVIALLY_COPYABLE(Type)

// Get the size of Type::member.
#define SIZEOF_MEMBER(Type, member) \
    sizeof(((Type*)0)->member)

// Define the compiler-specific syntax for packed types.
#ifdef _MSC_VER  // For Visual C++ compiler
#define PACK(...) \
  __pragma(pack(push, 1)) __VA_ARGS__ __pragma(pack(pop))
#else  // !_MSC_VER
#define PACK(...) \
  __VA_ARGS__ __attribute__((packed))
#endif  // _MSC_VER

}  // namespace linfs

}  // namespace fs
