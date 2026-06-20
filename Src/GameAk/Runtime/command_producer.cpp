#include "GameAk/Runtime/command_producer.h"

namespace gameak::runtime {

core::Result<void> CommandProducer::produce(Command command) {
    auto result = submit_fn_(std::move(command));
    if (!result) {
        return result.error();
    }
    return {};
}

} // namespace gameak::runtime
