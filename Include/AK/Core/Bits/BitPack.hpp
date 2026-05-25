#pragma once

#include <AK/Core/Bits/BitOps.hpp>
#include <AK/Core/Types.hpp>

namespace GameAK::Bits {

class BitCount {
public:
  explicit constexpr BitCount(usize n) noexcept : m_n(n) {}
  constexpr usize get() const noexcept { return m_n; }

private:
  usize m_n;
};

class BitPack {
public:
  BitPack(u8 *buffer) noexcept : m_buffer(buffer), m_bit_offset(0) {}

  void write(u64 value, BitCount bits) noexcept {
    for (usize i = 0; i < bits.get(); ++i) {
      write_bit((value >> i) & 1);
    }
  }

  u64 read(BitCount bits) noexcept {
    u64 value = 0;
    for (usize i = 0; i < bits.get(); ++i) {
      value |= (read_bit() << i);
    }
    return value;
  }

  void reset() noexcept { m_bit_offset = 0; }

  usize bit_offset() const noexcept { return m_bit_offset; }

private:
  static constexpr usize kBitsPerByte = 8;

  u8 *m_buffer;
  usize m_bit_offset;

  void write_bit(u8 bit) noexcept {
    usize byte = m_bit_offset / kBitsPerByte;
    usize off = m_bit_offset % kBitsPerByte;

    if (bit) {
      m_buffer[byte] |= (1 << off);
    } else {
      m_buffer[byte] &= ~(1 << off);
    }
    ++m_bit_offset;
  }

  u64 read_bit() noexcept {
    usize byte = m_bit_offset / kBitsPerByte;
    usize off = m_bit_offset % kBitsPerByte;

    u64 result = (u64(m_buffer[byte]) >> off) & 1;
    ++m_bit_offset;
    return result;
  }
};
} // namespace GameAK::Bits
