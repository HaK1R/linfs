#pragma once

#include <cstdint>
#include <type_traits>

namespace fs {

namespace linfs {

struct ByteOrder {
  // I think that two orders will be enough.
  struct LittleEndian { };
  struct BigEndian { };

  // The system's byte order.  Normally can be explored at compile time using
  // default system headers (like endian.h) but not on Windows.  Therefore
  // this implementation requires uses SYSTEM_ORDER macro which is specified
  // in project's Makefile.  The user is in charge for its correctness.
  using SystemOrder = ByteOrder::SYSTEM_ORDER;

  // The device's byte order.  The library can work on heterogeneous
  // architectures, dealing with data of different endianness.  Thus we must
  // specify the specific byte order used for all devices.  This can be
  // changed depending on user's needs.  You cannot operate with devices
  // which have other byte order than DEVICE_ORDER.
  using DeviceOrder = ByteOrder::DEVICE_ORDER;

  template <typename T,
            std::enable_if_t<std::is_integral<T>::value &&
                             std::is_same<DeviceOrder, SystemOrder>::value>* = nullptr>
  static T Pack(T value) { return value; }

  template <typename T,
            std::enable_if_t<std::is_integral<T>::value &&
                             !std::is_same<DeviceOrder, SystemOrder>::value>* = nullptr>
  static T Pack(T value) {
    return SwapBytes(value);
  }

  template <typename T>
  static T Unpack(T value) { return Pack(value); }

  // Helper routines for swapping bytes.
  static uint8_t SwapBytes(uint8_t val) {
    return val;
  }
  static uint16_t SwapBytes(uint16_t val) {
    return (val << 8) | (val >> 8);
  }
  static uint32_t SwapBytes(uint32_t val) {
    return uint32_t(SwapBytes(uint16_t(val))) << 16 |
           uint32_t(SwapBytes(uint16_t(val >> 16)));
  }
  static uint64_t SwapBytes(uint64_t val) {
    return uint64_t(SwapBytes(uint32_t(val))) << 32 |
           uint64_t(SwapBytes(uint32_t(val >> 32)));
  }
};

}  // namespace linfs

}  // namespace fs
