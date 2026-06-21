#pragma once

#include "block_type.h"
#include "GameAk/Core/flat_vector.h"
#include "GameAk/Core/identity.h"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <span>

namespace gameak::runtime {

class DataBlock {
public:
    DataBlock(core::Identity identity, uint32_t type_id, size_t size, size_t /*alignment*/)
        : identity_{identity}, type_id_{type_id} { data_.resize(size); }

    core::Identity identity() const { return identity_; }
    uint32_t type_id() const { return type_id_; }
    size_t size() const { return data_.size(); }

    void* data() { return data_.data(); }
    const void* data() const { return data_.data(); }

    template <typename T>
    T& field(size_t offset) {
        assert(offset + sizeof(T) <= data_.size() && "field<T> out of bounds");
        assert((reinterpret_cast<uintptr_t>(data_.data() + offset) & (alignof(T) - 1)) == 0
               && "field<T> misaligned access");
        return *reinterpret_cast<T*>(data_.data() + offset);
    }

    template <typename T>
    const T& field(size_t offset) const {
        assert(offset + sizeof(T) <= data_.size() && "field<T> out of bounds");
        assert((reinterpret_cast<uintptr_t>(data_.data() + offset) & (alignof(T) - 1)) == 0
               && "field<T> misaligned access");
        return *reinterpret_cast<const T*>(data_.data() + offset);
    }

    std::span<std::byte> as_span() { return data_; }
    std::span<const std::byte> as_span() const { return data_; }

    void resize(size_t new_size) { data_.resize(new_size); }

private:
    core::Identity identity_;
    uint32_t type_id_;
    core::flat_vector<std::byte, 1> data_;
};

} // namespace gameak::runtime
