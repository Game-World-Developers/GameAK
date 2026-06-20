#pragma once

#include <cstdint>
#include <type_traits>

namespace gameak::core {

template <typename Enum>
class BitFlags {
    static_assert(std::is_enum_v<Enum>, "BitFlags requires an enum type");
    using Underlying = std::underlying_type_t<Enum>;

    uint64_t bits_{0};

public:
    BitFlags() = default;
    explicit BitFlags(uint64_t initial) : bits_{initial} {}

    BitFlags& set(Enum flag) {
        bits_ |= static_cast<uint64_t>(1) << static_cast<Underlying>(flag);
        return *this;
    }

    BitFlags& reset(Enum flag) {
        bits_ &= ~(static_cast<uint64_t>(1) << static_cast<Underlying>(flag));
        return *this;
    }

    BitFlags& toggle(Enum flag) {
        bits_ ^= (static_cast<uint64_t>(1) << static_cast<Underlying>(flag));
        return *this;
    }

    bool is_set(Enum flag) const {
        return (bits_ & (static_cast<uint64_t>(1) << static_cast<Underlying>(flag))) != 0;
    }

    bool any() const { return bits_ != 0; }
    bool none() const { return bits_ == 0; }
    uint64_t value() const { return bits_; }

    bool operator==(const BitFlags& other) const { return bits_ == other.bits_; }
    bool operator!=(const BitFlags& other) const { return bits_ != other.bits_; }

    BitFlags operator|(const BitFlags& other) const { return BitFlags{bits_ | other.bits_}; }
    BitFlags operator&(const BitFlags& other) const { return BitFlags{bits_ & other.bits_}; }
    BitFlags operator^(const BitFlags& other) const { return BitFlags{bits_ ^ other.bits_}; }

    BitFlags& operator|=(const BitFlags& other) { bits_ |= other.bits_; return *this; }
    BitFlags& operator&=(const BitFlags& other) { bits_ &= other.bits_; return *this; }
    BitFlags& operator^=(const BitFlags& other) { bits_ ^= other.bits_; return *this; }
};

} // namespace gameak::core
