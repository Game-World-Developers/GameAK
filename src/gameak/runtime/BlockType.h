#pragma once

#include <cstdint>
#include <string>

namespace gameak::runtime {

struct BlockTypeDescriptor {
    uint32_t type_id;
    size_t size;
    size_t alignment;
    const char* name;
};

} // namespace gameak::runtime
