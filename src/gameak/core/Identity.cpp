#include "Identity.h"

#include <format>

namespace gameak::core {

std::string Identity::to_string() const {
    return std::format("Identity({})", id_);
}

} // namespace gameak::core
