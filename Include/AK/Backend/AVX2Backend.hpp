#pragma once

#include <AK/Backend/ScalarBuiltinsBackend.hpp>
#include <AK/Core/Types.hpp>
#include <immintrin.h>

namespace GameAK::Backend {

struct AVX2Backend : ScalarBuiltinsBackend {
#if defined(__AVX2__)
  static void bit_and_256(u64 *dst, const u64 *a, const u64 *b) noexcept {
    _mm256_store_si256(
        reinterpret_cast<__m256i *>(dst),
        _mm256_and_si256(
            _mm256_load_si256(reinterpret_cast<const __m256i *>(a)),
            _mm256_load_si256(reinterpret_cast<const __m256i *>(b))));
  }

  static void bit_or_256(u64 *dst, const u64 *a, const u64 *b) noexcept {
    _mm256_store_si256(
        reinterpret_cast<__m256i *>(dst),
        _mm256_or_si256(
            _mm256_load_si256(reinterpret_cast<const __m256i *>(a)),
            _mm256_load_si256(reinterpret_cast<const __m256i *>(b))));
  }

  static void bit_xor_256(u64 *dst, const u64 *a, const u64 *b) noexcept {
    _mm256_store_si256(
        reinterpret_cast<__m256i *>(dst),
        _mm256_xor_si256(
            _mm256_load_si256(reinterpret_cast<const __m256i *>(a)),
            _mm256_load_si256(reinterpret_cast<const __m256i *>(b))));
  }

  static void bit_not_256(u64 *dst, const u64 *a) noexcept {
    _mm256_store_si256(
        reinterpret_cast<__m256i *>(dst),
        _mm256_xor_si256(
            _mm256_load_si256(reinterpret_cast<const __m256i *>(a)),
            _mm256_set1_epi64x(-1LL)));
  }

  static usize popcount_256(const u64 *data) noexcept {
    const __m256i v = _mm256_load_si256(
        reinterpret_cast<const __m256i *>(data));
    const __m256i lut = _mm256_setr_epi8(
        0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4,
        0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4);
    __m256i lo = _mm256_and_si256(v, _mm256_set1_epi8(0x0F));
    __m256i hi = _mm256_and_si256(
        _mm256_srli_epi16(v, 4), _mm256_set1_epi8(0x0F));
    __m256i pop = _mm256_add_epi8(
        _mm256_shuffle_epi8(lut, lo),
        _mm256_shuffle_epi8(lut, hi));
    __m256i sums = _mm256_sad_epu8(pop, _mm256_setzero_si256());
    return static_cast<usize>(
        _mm256_extract_epi64(sums, 0) +
        _mm256_extract_epi64(sums, 2));
  }
#endif
};

} // namespace GameAK::Backend
