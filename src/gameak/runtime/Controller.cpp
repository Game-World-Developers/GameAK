#include "Controller.h"
#include "Runtime.h"
#include "Command.h"

namespace gameak::runtime {

core::Result<void> CommandProducer::produce(Command command) {
    auto result = runtime_.submit_command(std::move(command));
    if (!result) {
        return result.error();
    }
    return {};
}

} // namespace gameak::runtime
