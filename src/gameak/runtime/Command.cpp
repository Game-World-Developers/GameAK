#include "Command.h"

namespace gameak::runtime {

CommandType Command::type() const {
    return static_cast<CommandType>(payload_.index());
}

} // namespace gameak::runtime
