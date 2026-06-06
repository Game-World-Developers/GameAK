#pragma once

#include <AK/Backend/ExecutionProfile.hpp>
#include <AK/Core/Types.hpp>

namespace GameAK {

/// Execution context: carries frame-global metadata and the system's
/// ExecutionProfile. When ECS is added, this will also hold World &world.
struct ExecContext {
  Backend::ExecutionProfile profile;
  u64                      frame_index;
  f64                      delta_time;
};

} // namespace GameAK
