/// @file
/// @brief Multi-layer bit array — SOA-style layer management.
///
/// Wraps a contiguous storage region divided into identically-sized layers.
/// Supports per-layer queries and cross-layer bitwise operations (and, or, xor).

#pragma once

#include <AK/Backend/Backend.hpp>
#include <AK/Core/Bits/BitArray.hpp>
#include <AK/Core/Types.hpp>

namespace GameAK::Bits {

class BitLayer {
public:
  BitLayer(u64 *storage, usize layers, usize words_per_layer) noexcept
      : m_storage(storage), m_layers(layers), m_words(words_per_layer) {}

  // -------------------------
  // Access layer (bitmap view)
  // -------------------------
  BitArray<BitCheck::None> layer(usize index) noexcept {
    return BitArray<BitCheck::None>(m_storage + (index * m_words), m_words);
  }

  BitArray<BitCheck::None> layer(usize index) const noexcept {
    return BitArray<BitCheck::None>(
        const_cast<u64 *>(m_storage + (index * m_words)), m_words);
  }

  // -------------------------
  // Layer operations (SOA-style)
  // -------------------------

  void clear_layer(usize index) noexcept {
    (void)Backend::mem_set(m_storage + (index * m_words), 0, m_words * sizeof(u64));
  }

  void clear_all() noexcept {
    (void)Backend::mem_set(m_storage, 0, m_layers * m_words * sizeof(u64));
  }

  // -------------------------
  // Cross-layer operations
  // -------------------------

  void and_layers(usize dst, usize a, usize b) noexcept {
    Backend::bitset_and(m_storage + dst * m_words, m_storage + a * m_words,
                        m_storage + b * m_words, m_words);
  }

  void or_layers(usize dst, usize a, usize b) noexcept {
    Backend::bitset_or(m_storage + dst * m_words, m_storage + a * m_words,
                       m_storage + b * m_words, m_words);
  }

  void xor_layers(usize dst, usize a, usize b) noexcept {
    Backend::bitset_xor(m_storage + dst * m_words, m_storage + a * m_words,
                        m_storage + b * m_words, m_words);
  }

  // -------------------------
  // Queries
  // -------------------------

  usize popcount_layer(usize index) const noexcept {
    return Backend::bitset_popcount_range(m_storage + index * m_words, m_words);
  }

  usize layers() const noexcept { return m_layers; }
  usize words_per_layer() const noexcept { return m_words; }

private:
  u64 *m_storage;
  usize m_layers;
  usize m_words;
};

} // namespace GameAK::Bits
