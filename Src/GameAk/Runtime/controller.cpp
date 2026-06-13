#include "GameAk/Runtime/controller.h"
#include "GameAk/Runtime/data_block.h"
#include "GameAk/Runtime/block_type.h"

namespace gameak::runtime {

core::Result<void> CommandProducer::produce(Command command) {
    auto result = submit_fn_(std::move(command));
    if (!result) {
        return result.error();
    }
    return {};
}

} // namespace gameak::runtime
