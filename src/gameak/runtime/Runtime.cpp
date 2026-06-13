#include "Runtime.h"
#include "FifoScheduler.h"

#include <algorithm>
#include <spdlog/spdlog.h>

namespace gameak::runtime {

namespace {

spdlog::level::level_enum to_spdlog_level(LogLevel level) {
    switch (level) {
        case LogLevel::Trace:    return spdlog::level::trace;
        case LogLevel::Debug:    return spdlog::level::debug;
        case LogLevel::Info:     return spdlog::level::info;
        case LogLevel::Warn:     return spdlog::level::warn;
        case LogLevel::Error:    return spdlog::level::err;
        case LogLevel::Critical: return spdlog::level::critical;
        case LogLevel::Off:      return spdlog::level::off;
    }
    return spdlog::level::warn;
}

} // namespace

Runtime::Runtime(RuntimeConfig config)
    : config_{config}
    , scheduler_{std::make_unique<FifoScheduler>()} {
    apply_log_level(config_.log_level);
    SPDLOG_DEBUG("Runtime created");
}

Runtime::~Runtime() {
    SPDLOG_DEBUG("Runtime destroyed");
}

void Runtime::apply_log_level(LogLevel level) {
    spdlog::set_level(to_spdlog_level(level));
    spdlog::set_pattern("[%l] %v");
}

core::Result<void> Runtime::register_block_type(BlockTypeDescriptor descriptor) {
    if (types_.contains(descriptor.type_id)) {
        SPDLOG_WARN("Block type {} already registered", descriptor.type_id);
        return core::Error{core::ErrorCode::DuplicateRegistration, "Block type already registered"};
    }
    types_[descriptor.type_id] = descriptor;
    SPDLOG_INFO("Registered block type {} ({})", descriptor.type_id, descriptor.name);
    return {};
}

core::Result<void> Runtime::register_controller(Controller controller) {
    controllers_.push_back(std::move(controller));
    SPDLOG_DEBUG("Controller registered (total: {})", controllers_.size());
    return {};
}

core::Result<core::Identity> Runtime::create_block(uint32_t type_id) {
    auto it = types_.find(type_id);
    if (it == types_.end()) {
        SPDLOG_ERROR("Cannot create block: type {} not registered", type_id);
        return core::Error{core::ErrorCode::TypeNotRegistered, "Block type not registered"};
    }
    auto& desc = it->second;
    core::Identity id{++next_identity_};
    blocks_.emplace(id, DataBlock{id, desc.type_id, desc.size, desc.alignment});
    SPDLOG_DEBUG("Created block {} of type {}", id.value(), type_id);
    return id;
}

core::Result<void> Runtime::destroy_block(core::Identity identity) {
    if (!identity.is_valid()) {
        SPDLOG_WARN("Cannot destroy block: invalid identity");
        return core::Error{core::ErrorCode::InvalidIdentity, "Identity is invalid"};
    }
    auto it = blocks_.find(identity);
    if (it == blocks_.end()) {
        SPDLOG_WARN("Cannot destroy block {}: not found", identity.value());
        return core::Error{core::ErrorCode::BlockNotFound, "Block not found"};
    }
    blocks_.erase(it);
    SPDLOG_DEBUG("Destroyed block {}", identity.value());
    return {};
}

core::Result<void> Runtime::submit_command(Command command) {
    scheduler_->enqueue(std::move(command));
    SPDLOG_TRACE("Command enqueued (pending: {})", scheduler_->pending_count());
    return {};
}

TickResult Runtime::tick() {
    TickResult result;

    SPDLOG_DEBUG("Tick start (pending commands: {}, controllers: {})",
                 scheduler_->pending_count(), controllers_.size());

    StateView state_view{*this};

    for (auto& controller : controllers_) {
        CommandProducer producer{*this};
        auto cr = controller(state_view, producer);
        if (!cr) {
            SPDLOG_WARN("Controller failed: {}", cr.error().message());
            result.status = ExecutionStatus::PartialFailure;
        }
        result.controllers_executed++;
    }

    SPDLOG_DEBUG("Controllers done, processing {} pending commands", scheduler_->pending_count());

    auto sr = scheduler_->process_pending(mutable_blocks(), types_, next_identity());
    result.commands_executed = scheduler_->executed_count();
    result.commands_rejected = scheduler_->rejected_count();

    if (!sr) {
        SPDLOG_ERROR("Scheduler critical failure");
        result.status = ExecutionStatus::CriticalFailure;
    } else if (result.commands_rejected > 0) {
        SPDLOG_WARN("Tick partial failure: {} executed, {} rejected",
                    result.commands_executed, result.commands_rejected);
        result.status = ExecutionStatus::PartialFailure;
    } else {
        SPDLOG_DEBUG("Tick success: {} commands executed", result.commands_executed);
        result.status = ExecutionStatus::Success;
    }

    return result;
}

bool Runtime::has_block(core::Identity identity) const {
    return blocks_.contains(identity);
}

size_t Runtime::block_count(uint32_t type_id) const {
    return std::count_if(blocks_.begin(), blocks_.end(),
                         [type_id](const auto& pair) {
                             return pair.second.type_id() == type_id;
                         });
}

} // namespace gameak::runtime
