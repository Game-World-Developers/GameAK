#pragma once

#include <cstddef>
#include <cstdint>

namespace gameak::runtime {

struct Diagnostics {
    size_t pending_commands{0};
    size_t commands_executed{0};
    size_t commands_rejected{0};
    size_t commands_skipped{0};
    size_t blocks_count{0};
    size_t controllers_count{0};
    uint64_t next_identity{0};
};

} // namespace gameak::runtime
