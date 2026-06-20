#include "GameAk/Runtime/command.h"

#include <type_traits>

namespace gameak::runtime {

// Verify CommandType enum matches std::variant alternative order.
// This ensures Command::type() (payload_.index() cast to CommandType) is correct.
static_assert(std::is_same_v<std::variant_alternative_t<0, CommandPayload>, CommandCreateBlock>,
              "Variant index 0 must be CommandCreateBlock");
static_assert(std::is_same_v<std::variant_alternative_t<1, CommandPayload>, CommandDestroyBlock>,
              "Variant index 1 must be CommandDestroyBlock");
static_assert(std::is_same_v<std::variant_alternative_t<2, CommandPayload>, CommandSetField>,
              "Variant index 2 must be CommandSetField");
static_assert(std::is_same_v<std::variant_alternative_t<3, CommandPayload>, CommandResizeBlock>,
              "Variant index 3 must be CommandResizeBlock");
static_assert(std::is_same_v<std::variant_alternative_t<4, CommandPayload>, CommandConvertLayout>,
              "Variant index 4 must be CommandConvertLayout");

// Also verify that the CommandType enum has the same number of values as variant alternatives
static_assert(static_cast<size_t>(CommandType::ConvertLayout) + 1 == std::variant_size_v<CommandPayload>,
              "CommandType enum must have same number of entries as CommandPayload variant");

CommandType Command::type() const {
    return static_cast<CommandType>(payload_.index());
}

} // namespace gameak::runtime
