/// @file
/// @brief Bit-level serialisation: write and read arbitrary-width values
///        to/from a byte buffer.
///
/// BitPack provides a serial stream of bits backed by a u8 buffer. Callers
/// write or read values bit-by-bit (or via the convenience write/read
/// methods that handle multi-bit extraction internally). The internal
/// bit offset advances after every operation.
///
/// BitCount is a strong typedef used to pass bit-widths to BitPack methods.

#pragma once

#include <AK/Core/Bits/BitOps.hpp>
#include <AK/Core/Types.hpp>

namespace GameAK::Bits {

/// Strong typedef wrapping a bit count.
///
/// Prevents accidental mixing of bit counts with raw integer values
/// in BitPack method signatures.
class BitCount {
public:
  explicit constexpr BitCount(usize n) noexcept : m_n(n) {}
  constexpr usize get() const noexcept { return m_n; }

private:
  usize m_n;
};

/// Bit-level packer over an external u8 buffer.
///
/// Writes and reads values of arbitrary bit-width (up to 64 bits) to/from
/// a byte buffer. The buffer is not owned — the caller is responsible
/// for providing and managing the underlying storage.
///
/// Typical use:
///   @code
///   u8 buf[16];
///   BitPack pack(buf);
///   pack.write(0xAA, BitCount(8));
///   pack.read(BitCount(8)); // 0xAA
///   @endcode
class BitPack {
public:
  /// Constructs a bit packer over an externally-owned byte buffer.
  ///
  /// @param buffer  Backing byte array. Must not be null.
  BitPack(u8 *buffer) noexcept : m_buffer(buffer), m_bit_offset(0) {}

  /// Writes the lowest @p bits bits of @p value.
  ///
  /// Bits are written least-significant first.
  ///
  /// @param value  Source value whose low bits are written.
  /// @param bits   Number of bits to write.
  void write(u64 value, BitCount bits) noexcept {
    for (usize i = 0; i < bits.get(); ++i) {
      write_bit((value >> i) & 1);
    }
  }

  /// Reads @p bits bits from the stream and returns them in the low
  /// positions of the result.
  ///
  /// Bits are read least-significant first.
  ///
  /// @param bits  Number of bits to read.
  /// @return      Value reconstructed from the read bits.
  u64 read(BitCount bits) noexcept {
    u64 value = 0;
    for (usize i = 0; i < bits.get(); ++i) {
      value |= (read_bit() << i);
    }
    return value;
  }

  /// Resets the stream position to the beginning.
  ///
  /// Does not zero the buffer — data remains but will be overwritten
  /// by subsequent writes.
  void reset() noexcept { m_bit_offset = 0; }

  /// @return Current bit offset (total bits written or read so far).
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
