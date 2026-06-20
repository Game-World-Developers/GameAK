#pragma once

#include <cstdint>

namespace gameak::runtime {

enum class LogLevel : uint32_t {
    Trace, Debug, Info, Warn, Error, Critical, Off,
};

} // namespace gameak::runtime
