#pragma once

#include <AK/Backend/ScalarBuiltinsBackend.hpp>
#include <AK/Core/Bits/BitArray.hpp>
#include <AK/Core/Types.hpp>

namespace GameAK::Bits {

using Backend::ScalarBuiltinsBackend;

template <typename Backend = ScalarBuiltinsBackend>
class BitLayer {
public:
  BitLayer(u64 *storage, usize layers, usize words_per_layer) noexcept
      : m_storage(storage), m_layers(layers), m_words(words_per_layer) {}

  BitArray<Backend, BitCheck::None> layer(usize index) noexcept {
    return BitArray<Backend, BitCheck::None>(
        m_storage + (index * m_words), m_words);
  }

  BitArray<Backend, BitCheck::None> layer(usize index) const noexcept {
    return BitArray<Backend, BitCheck::None>(
        const_cast<u64 *>(m_storage + (index * m_words)), m_words);
  }

  void clear_layer(usize index) noexcept {
    (void)Backend::mem_set(m_storage + (index * m_words), 0,
                           m_words * sizeof(u64));
  }

  void clear_all() noexcept {
    (void)Backend::mem_set(m_storage, 0,
                           m_layers * m_words * sizeof(u64));
  }

  void and_layers(usize dst, usize a, usize b) noexcept {
    usize i = 0;
    u64 *dd = m_storage + dst * m_words;
    u64 *aa = m_storage + a * m_words;
    u64 *bb = m_storage + b * m_words;
    for (; i + 4 <= m_words; i += 4)
      Backend::bit_and_256(dd + i, aa + i, bb + i);
    for (; i < m_words; ++i)
      dd[i] = aa[i] & bb[i];
  }

  void or_layers(usize dst, usize a, usize b) noexcept {
    usize i = 0;
    u64 *dd = m_storage + dst * m_words;
    u64 *aa = m_storage + a * m_words;
    u64 *bb = m_storage + b * m_words;
    for (; i + 4 <= m_words; i += 4)
      Backend::bit_or_256(dd + i, aa + i, bb + i);
    for (; i < m_words; ++i)
      dd[i] = aa[i] | bb[i];
  }

  void xor_layers(usize dst, usize a, usize b) noexcept {
    usize i = 0;
    u64 *dd = m_storage + dst * m_words;
    u64 *aa = m_storage + a * m_words;
    u64 *bb = m_storage + b * m_words;
    for (; i + 4 <= m_words; i += 4)
      Backend::bit_xor_256(dd + i, aa + i, bb + i);
    for (; i < m_words; ++i)
      dd[i] = aa[i] ^ bb[i];
  }

  usize popcount_layer(usize index) const noexcept {
    usize total = 0;
    usize i = 0;
    const u64 *data = m_storage + index * m_words;
    for (; i + 4 <= m_words; i += 4)
      total += Backend::popcount_256(data + i);
    for (; i < m_words; ++i)
      total += static_cast<usize>(Bits::popcount(data[i]));
    return total;
  }

  usize layers() const noexcept { return m_layers; }
  usize words_per_layer() const noexcept { return m_words; }

private:
  u64 *m_storage;
  usize m_layers;
  usize m_words;
};

} // namespace GameAK::Bits
