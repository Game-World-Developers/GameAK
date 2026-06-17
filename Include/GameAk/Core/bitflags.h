#pragma once

#include <cstdint>
#include <string>
#include <type_traits>
#include <unordered_map>

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

class DynamicBitFlags {
    uint64_t bits_{0};
    std::unordered_map<std::string, uint8_t> name_to_bit_;

public:
    DynamicBitFlags() = default;

    uint8_t register_flag(const std::string& name) {
        uint8_t pos = static_cast<uint8_t>(name_to_bit_.size());
        if (pos >= 64) return 64;
        name_to_bit_[name] = pos;
        return pos;
    }

    void set(const std::string& name) {
        auto it = name_to_bit_.find(name);
        if (it != name_to_bit_.end()) {
            bits_ |= (uint64_t{1} << it->second);
        }
    }

    void reset(const std::string& name) {
        auto it = name_to_bit_.find(name);
        if (it != name_to_bit_.end()) {
            bits_ &= ~(uint64_t{1} << it->second);
        }
    }

    bool is_set(const std::string& name) const {
        auto it = name_to_bit_.find(name);
        if (it != name_to_bit_.end()) {
            return (bits_ & (uint64_t{1} << it->second)) != 0;
        }
        return false;
    }

    uint64_t value() const { return bits_; }
    bool any() const { return bits_ != 0; }
    bool none() const { return bits_ == 0; }

    bool operator==(const DynamicBitFlags& other) const { return bits_ == other.bits_; }
    bool operator!=(const DynamicBitFlags& other) const { return bits_ != other.bits_; }
};

} // namespace gameak::core
