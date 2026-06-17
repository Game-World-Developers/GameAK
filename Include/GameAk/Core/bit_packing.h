#pragma once

#include <cstdint>
#include <type_traits>

namespace gameak::core::bit_packing {

template <size_t Offset, size_t Width>
struct Field {
    static_assert(Width > 0, "Field width must be > 0");
    static_assert(Offset + Width <= 64, "Field exceeds 64-bit word");

    static constexpr size_t offset = Offset;
    static constexpr size_t width  = Width;
    static constexpr uint64_t mask = ((uint64_t{1} << Width) - 1);

    template <typename T>
    static constexpr uint64_t pack(uint64_t word, T value) {
        auto v = static_cast<uint64_t>(value);
        return (word & ~(mask << Offset)) | ((v & mask) << Offset);
    }

    template <typename T = uint64_t>
    static constexpr T unpack(uint64_t word) {
        return static_cast<T>((word >> Offset) & mask);
    }
};

template <typename... Fields>
class PackedWord {
    uint64_t word_{0};

public:
    PackedWord() = default;
    explicit PackedWord(uint64_t initial) : word_{initial} {}

    template <typename Field, typename T>
    PackedWord& set(T value) {
        word_ = Field::pack(word_, value);
        return *this;
    }

    template <typename Field, typename T = uint64_t>
    T get() const {
        return Field::template unpack<T>(word_);
    }

    uint64_t value() const { return word_; }
    void set_value(uint64_t v) { word_ = v; }

    bool operator==(const PackedWord& other) const { return word_ == other.word_; }
    bool operator!=(const PackedWord& other) const { return word_ != other.word_; }
};

template <size_t Offset, size_t Width>
static constexpr uint64_t pack_value(uint64_t word, uint64_t value) {
    return Field<Offset, Width>::pack(word, value);
}

template <size_t Offset, size_t Width>
static constexpr uint64_t unpack_value(uint64_t word) {
    return Field<Offset, Width>::unpack(word);
}

} // namespace gameak::core::bit_packing
