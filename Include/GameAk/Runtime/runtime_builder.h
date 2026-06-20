#pragma once

#include "runtime_config.h"
#include "fifo_scheduler.h"

namespace gameak::runtime {

template <typename SchedulerType = FifoScheduler>
class RuntimeBuilder {
    RuntimeConfig config_;
    float fixed_dt_{0.0f};
    bool has_fixed_dt_{false};
public:
    RuntimeBuilder() = default;

    RuntimeBuilder& log_level(LogLevel level) {
        config_.log_level = level;
        return *this;
    }

    RuntimeBuilder& fixed_timestep(float dt) {
        fixed_dt_ = dt;
        has_fixed_dt_ = true;
        return *this;
    }

    template <typename S>
    RuntimeBuilder<S> scheduler() {
        RuntimeBuilder<S> b;
        b.config_ = config_;
        b.fixed_dt_ = fixed_dt_;
        b.has_fixed_dt_ = has_fixed_dt_;
        return b;
    }

    Runtime<SchedulerType> build() && {
        Runtime<SchedulerType> rt{config_};
        if (has_fixed_dt_) {
            rt.set_fixed_timestep(fixed_dt_);
        }
        return rt;
    }
};

} // namespace gameak::runtime
