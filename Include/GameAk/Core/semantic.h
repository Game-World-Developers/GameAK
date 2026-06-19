#pragma once

#include <cstddef>
#include <cstdint>

namespace gameak::core {

struct SemanticConstraint {
    int64_t min{0};
    int64_t max{0};
    bool is_bool{false};
    size_t enum_count{0};

    static constexpr SemanticConstraint range(int64_t min_val, int64_t max_val) {
        return SemanticConstraint{min_val, max_val, false, 0};
    }

    static constexpr SemanticConstraint boolean() {
        return SemanticConstraint{0, 0, true, 0};
    }

    static constexpr SemanticConstraint enumeration(size_t count) {
        return SemanticConstraint{0, 0, false, count};
    }

    bool valid() const {
        if (is_bool) return true;
        if (enum_count > 0) return true;
        if (max > min) return true;
        if (max == min && max != 0) return true;
        return false;
    }
};

constexpr size_t bits_for(const SemanticConstraint& sc) {
    if (sc.is_bool) return 1;
    if (sc.enum_count > 0) {
        if (sc.enum_count <= 1) return 1;
        size_t n = sc.enum_count - 1;
        size_t bits = 0;
        while (n > 0) { n >>= 1; ++bits; }
        return bits;
    }
    if (sc.max > sc.min) {
        uint64_t range = static_cast<uint64_t>(sc.max - sc.min);
        size_t bits = 0;
        while (range > 0) { range >>= 1; ++bits; }
        return bits;
    }
    if (sc.max == sc.min && sc.max != 0) {
        return 1;
    }
    return 0;
}

constexpr size_t bytes_for(const SemanticConstraint& sc) {
    size_t bits = bits_for(sc);
    if (bits == 0) return 0;
    return (bits + 7) / 8;
}

} // namespace gameak::core
