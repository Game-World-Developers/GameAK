#pragma once

#include "GameAk/Core/rb_tree.h"

#include <cstdint>
#include <string>

namespace gameak::core {

class DynamicBitFlags {
    uint64_t bits_{0};
    core::rb_tree<std::string, uint8_t> name_to_bit_;

public:
    DynamicBitFlags() = default;

    uint8_t register_flag(const std::string& name) {
        uint8_t pos = static_cast<uint8_t>(name_to_bit_.size());
        if (pos >= 64) return 64;
        name_to_bit_.insert(name, pos);
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
