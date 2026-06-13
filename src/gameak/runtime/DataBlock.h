#pragma once

#include "BlockType.h"
#include "gameak/core/Identity.h"

#include <cstddef>
#include <vector>

namespace gameak::runtime {

class DataBlock {
public:
    DataBlock(core::Identity identity, uint32_t type_id, size_t size, size_t alignment)
        : identity_{identity}, type_id_{type_id}, data_(size, std::byte{0}) {}

    core::Identity identity() const { return identity_; }
    uint32_t type_id() const { return type_id_; }
    size_t size() const { return data_.size(); }

    void* data() { return data_.data(); }
    const void* data() const { return data_.data(); }

private:
    core::Identity identity_;
    uint32_t type_id_;
    std::vector<std::byte> data_;
};

} // namespace gameak::runtime
