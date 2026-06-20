#pragma once

#include "GameAk/Core/identity.h"
#include "GameAk/Core/result.h"

#include <cstdint>
#include <functional>

namespace gameak::runtime {

class EphemeralProducer {
public:
    using CreateFn = std::function<core::Result<core::Identity>(uint32_t type_id)>;

    explicit EphemeralProducer(CreateFn create_fn)
        : create_fn_(std::move(create_fn)) {}

    core::Result<core::Identity> create(uint32_t type_id) {
        return create_fn_(type_id);
    }

private:
    CreateFn create_fn_;
};

} // namespace gameak::runtime
