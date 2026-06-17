#pragma once

#include "flat_vector.h"

#include <cstddef>
#include <cstdint>

namespace gameak::core {

class BitVector {
    flat_vector<uint64_t, 2> words_;
    size_t bit_count_{0};

    static constexpr size_t kWordBits = 64;

    static size_t words_needed(size_t bits) {
        return (bits + kWordBits - 1) / kWordBits;
    }

public:
    BitVector() = default;

    explicit BitVector(size_t initial_bits) {
        resize(initial_bits);
    }

    void push_back(bool value) {
        size_t pos = bit_count_;
        size_t w = words_needed(pos + 1);
        if (w > words_.size()) {
            words_.resize(w);
        }
        if (value) {
            words_[pos / kWordBits] |= (uint64_t{1} << (pos % kWordBits));
        } else {
            words_[pos / kWordBits] &= ~(uint64_t{1} << (pos % kWordBits));
        }
        ++bit_count_;
    }

    void set(size_t pos) {
        if (pos >= bit_count_) resize(pos + 1);
        words_[pos / kWordBits] |= (uint64_t{1} << (pos % kWordBits));
    }

    void reset(size_t pos) {
        if (pos >= bit_count_) return;
        words_[pos / kWordBits] &= ~(uint64_t{1} << (pos % kWordBits));
    }

    void flip(size_t pos) {
        if (pos >= bit_count_) resize(pos + 1);
        words_[pos / kWordBits] ^= (uint64_t{1} << (pos % kWordBits));
    }

    bool test_bit(size_t pos) const {
        if (pos >= bit_count_) return false;
        return (words_[pos / kWordBits] & (uint64_t{1} << (pos % kWordBits))) != 0;
    }

    bool operator[](size_t pos) const { return test_bit(pos); }

    size_t size() const { return bit_count_; }
    bool empty() const { return bit_count_ == 0; }

    void resize(size_t new_bits) {
        size_t needed = words_needed(new_bits);
        if (needed > words_.size()) {
            words_.resize(needed);
        }
        bit_count_ = new_bits;
    }

    void clear() {
        words_.clear();
        bit_count_ = 0;
    }

    size_t count() const {
        size_t c = 0;
        for (auto w : words_) c += __builtin_popcountll(w);
        return c;
    }

    bool any() const { return count() > 0; }
    bool none() const { return count() == 0; }

    const uint64_t* data() const { return words_.data(); }
    size_t word_count() const { return words_.size(); }
};

} // namespace gameak::core
