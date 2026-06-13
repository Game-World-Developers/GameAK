#include "Controller.h"
#include "DataBlock.h"
#include "BlockType.h"

namespace gameak::runtime {

core::Result<void> CommandProducer::produce(Command command) {
    auto result = submit_fn_(std::move(command));
    if (!result) {
        return result.error();
    }
    return {};
}

} // namespace gameak::runtime
