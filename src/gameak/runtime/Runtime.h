#pragma once

#include "BlockType.h"
#include "Command.h"
#include "Controller.h"
#include "FifoScheduler.h"
#include "gameak/core/Identity.h"
#include "gameak/core/Result.h"
#include "gameak/core/flat_vector.h"

#include <cstdint>
#include <spdlog/spdlog.h>
#include <unordered_map>
#include <vector>

namespace gameak::runtime {

enum class LogLevel : uint32_t {
    Trace, Debug, Info, Warn, Error, Critical, Off,
};

struct RuntimeConfig {
    LogLevel log_level{LogLevel::Warn};
};

enum class ExecutionStatus : uint32_t {
    Success,
    PartialFailure,
    CriticalFailure,
};

struct TickResult {
    size_t commands_executed{0};
    size_t commands_rejected{0};
    size_t controllers_executed{0};
    ExecutionStatus status{ExecutionStatus::Success};
    std::vector<RejectedCommand> rejected_commands;
};

template <typename SchedulerType = FifoScheduler>
class Runtime {
public:
    explicit Runtime(RuntimeConfig config = {});
    ~Runtime();

    Runtime(const Runtime&) = delete;
    Runtime& operator=(const Runtime&) = delete;
    Runtime(Runtime&&) = delete;
    Runtime& operator=(Runtime&&) = delete;

    core::Result<void> register_block_type(BlockTypeDescriptor descriptor);
    core::Result<void> register_controller(Controller controller);

    core::Result<core::Identity> create_block(uint32_t type_id);
    core::Result<void> destroy_block(core::Identity identity);

    core::Result<CommandId> submit_command(Command command);
    void cancel_command(CommandId id);

    TickResult tick();

    bool has_block(core::Identity identity) const;
    size_t block_count(uint32_t type_id) const {
        auto it = type_counts_.find(type_id);
        return it != type_counts_.end() ? it->second : 0;
    }

    const RuntimeConfig& config() const { return config_; }

    SchedulerType& scheduler() { return scheduler_; }
    const SchedulerType& scheduler() const { return scheduler_; }
    const std::unordered_map<core::Identity, DataBlock>& blocks() const { return blocks_; }
    std::unordered_map<core::Identity, DataBlock>& mutable_blocks() { return blocks_; }
    const std::unordered_map<uint32_t, BlockTypeDescriptor>& block_types() const { return types_; }
    uint64_t& next_identity() { return next_identity_; }

private:
    void apply_log_level(LogLevel level);
    void rebuild_type_counts();

    RuntimeConfig config_;
    SchedulerType scheduler_;
    std::unordered_map<core::Identity, DataBlock> blocks_;
    std::unordered_map<uint32_t, BlockTypeDescriptor> types_;
    gameak::core::flat_vector<Controller, 4> controllers_;

    CommandId next_command_id_{0};
    uint64_t next_identity_{0};
    std::unordered_map<uint32_t, size_t> type_counts_;
};

// -------------------------------------------------------------------
// Template implementation (must be visible at point of instantiation)
// -------------------------------------------------------------------

template <typename S>
Runtime<S>::Runtime(RuntimeConfig config)
    : config_{config} {
    this->apply_log_level(this->config_.log_level);
    SPDLOG_DEBUG("Runtime created");
}

template <typename S>
Runtime<S>::~Runtime() {
    SPDLOG_DEBUG("Runtime destroyed");
}

template <typename S>
void Runtime<S>::apply_log_level(LogLevel level) {
    switch (level) {
        case LogLevel::Trace: spdlog::set_level(spdlog::level::trace); break;
        case LogLevel::Debug: spdlog::set_level(spdlog::level::debug); break;
        case LogLevel::Info:  spdlog::set_level(spdlog::level::info);  break;
        case LogLevel::Warn:  spdlog::set_level(spdlog::level::warn);  break;
        case LogLevel::Error: spdlog::set_level(spdlog::level::err);   break;
        case LogLevel::Critical: spdlog::set_level(spdlog::level::critical); break;
        case LogLevel::Off:   spdlog::set_level(spdlog::level::off);   break;
    }
}

template <typename S>
core::Result<void> Runtime<S>::register_block_type(BlockTypeDescriptor descriptor) {
    if (this->types_.contains(descriptor.type_id)) {
        return core::Error{core::ErrorCode::DuplicateRegistration, "Block type already registered"};
    }
    this->types_[descriptor.type_id] = descriptor;
    SPDLOG_DEBUG("Registered block type: id={}, name={}, size={}",
                 descriptor.type_id, descriptor.name, descriptor.size);
    return {};
}

template <typename S>
core::Result<void> Runtime<S>::register_controller(Controller controller) {
    if (!controller) {
        return core::Error{core::ErrorCode::InvalidOperation, "Controller is empty"};
    }
    this->controllers_.push_back(std::move(controller));
    SPDLOG_DEBUG("Registered controller (total={})", this->controllers_.size());
    return {};
}

template <typename S>
core::Result<core::Identity> Runtime<S>::create_block(uint32_t type_id) {
    auto it = this->types_.find(type_id);
    if (it == this->types_.end()) {
        return core::Error{core::ErrorCode::TypeNotRegistered, "Block type not registered"};
    }
    auto& desc = it->second;
    core::Identity id{++this->next_identity_};
    this->blocks_.emplace(id, DataBlock{id, desc.type_id, desc.size, desc.alignment});
    return id;
}

template <typename S>
core::Result<void> Runtime<S>::destroy_block(core::Identity identity) {
    if (!identity.is_valid()) {
        return core::Error{core::ErrorCode::InvalidIdentity, "Identity is invalid"};
    }
    auto it = this->blocks_.find(identity);
    if (it == this->blocks_.end()) {
        return core::Error{core::ErrorCode::BlockNotFound, "Block not found"};
    }
    this->blocks_.erase(it);
    return {};
}

template <typename S>
core::Result<CommandId> Runtime<S>::submit_command(Command command) {
    command.set_id(++this->next_command_id_);
    this->scheduler_.enqueue(std::move(command));
    return command.id();
}

template <typename S>
void Runtime<S>::cancel_command(CommandId id) {
    this->scheduler_.cancel(id);
}

template <typename S>
TickResult Runtime<S>::tick() {
    TickResult result;

    // Controller Execution Phase
    size_t controller_count = 0;
    for (auto& controller : this->controllers_) {
        StateView state_view{this->blocks_, this->types_};
        CommandProducer producer{
            [this](Command cmd) -> core::Result<CommandId> {
                return this->submit_command(std::move(cmd));
            }
        };
        auto cr = controller(state_view, producer);
        if (!cr) {
            SPDLOG_WARN("Controller failed: {}", cr.error().message());
            result.status = ExecutionStatus::PartialFailure;
        }
        controller_count++;
    }
    result.controllers_executed = controller_count;

    // Command Processing Phase
    auto process_result = this->scheduler_.process_pending(this->blocks_, this->types_, this->next_identity_);

    auto rejected = this->scheduler_.take_rejected();
    result.commands_executed = this->scheduler_.executed_count();
    result.commands_rejected = rejected.size();
    result.rejected_commands = std::move(rejected);

    if (!process_result) {
        result.status = ExecutionStatus::CriticalFailure;
    } else if (result.commands_rejected > 0) {
        result.status = ExecutionStatus::PartialFailure;
    } else {
        result.status = ExecutionStatus::Success;
    }

    this->rebuild_type_counts();
    return result;
}

template <typename S>
bool Runtime<S>::has_block(core::Identity identity) const {
    return this->blocks_.contains(identity);
}

template <typename S>
void Runtime<S>::rebuild_type_counts() {
    this->type_counts_.clear();
    for (const auto& [id, block] : this->blocks_) {
        (void)id;
        this->type_counts_[block.type_id()]++;
    }
}

} // namespace gameak::runtime
