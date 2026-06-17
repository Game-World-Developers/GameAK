#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <initializer_list>

namespace gameak::core {

template <size_t N>
class BitSet {
    static constexpr size_t kWords = (N + 63) / 64;

    std::array<uint64_t, kWords> words_{};

    static constexpr size_t word_index(size_t pos) { return pos / 64; }
    static constexpr size_t bit_index(size_t pos)  { return pos % 64; }
    static constexpr uint64_t bit_mask(size_t pos) { return uint64_t{1} << bit_index(pos); }

public:
    BitSet() = default;

    BitSet(std::initializer_list<size_t> bits) {
        for (auto b : bits) set(b);
    }

    BitSet& set(size_t pos) {
        words_[word_index(pos)] |= bit_mask(pos);
        return *this;
    }

    BitSet& reset(size_t pos) {
        words_[word_index(pos)] &= ~bit_mask(pos);
        return *this;
    }

    BitSet& flip(size_t pos) {
        words_[word_index(pos)] ^= bit_mask(pos);
        return *this;
    }

    bool test_bit(size_t pos) const {
        return (words_[word_index(pos)] & bit_mask(pos)) != 0;
    }

    bool operator[](size_t pos) const { return test_bit(pos); }

    BitSet& set_all() {
        for (auto& w : words_) w = ~uint64_t{0};
        return *this;
    }

    BitSet& reset_all() {
        for (auto& w : words_) w = 0;
        return *this;
    }

    size_t count() const {
        size_t c = 0;
        for (auto w : words_) c += __builtin_popcountll(w);
        return c;
    }

    bool any() const { return count() > 0; }
    bool none() const { return count() == 0; }

    static constexpr size_t size() { return N; }

    bool operator==(const BitSet& other) const { return words_ == other.words_; }
    bool operator!=(const BitSet& other) const { return words_ != other.words_; }

    BitSet operator~() const {
        BitSet r;
        for (size_t i = 0; i < kWords; ++i) r.words_[i] = ~words_[i];
        return r;
    }

    BitSet operator&(const BitSet& other) const {
        BitSet r;
        for (size_t i = 0; i < kWords; ++i) r.words_[i] = words_[i] & other.words_[i];
        return r;
    }

    BitSet operator|(const BitSet& other) const {
        BitSet r;
        for (size_t i = 0; i < kWords; ++i) r.words_[i] = words_[i] | other.words_[i];
        return r;
    }

    BitSet operator^(const BitSet& other) const {
        BitSet r;
        for (size_t i = 0; i < kWords; ++i) r.words_[i] = words_[i] ^ other.words_[i];
        return r;
    }

    BitSet& operator&=(const BitSet& other) {
        for (size_t i = 0; i < kWords; ++i) words_[i] &= other.words_[i];
        return *this;
    }

    BitSet& operator|=(const BitSet& other) {
        for (size_t i = 0; i < kWords; ++i) words_[i] |= other.words_[i];
        return *this;
    }

    BitSet& operator^=(const BitSet& other) {
        for (size_t i = 0; i < kWords; ++i) words_[i] ^= other.words_[i];
        return *this;
    }
};

} // namespace gameak::core
