#pragma once

#include "log_level.h"

namespace gameak::runtime {

struct RuntimeConfig {
    LogLevel log_level{LogLevel::Warn};
};

} // namespace gameak::runtime
