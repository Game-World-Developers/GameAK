#pragma once

#include "BlockType.h"
#include "Command.h"
#include "Controller.h"
#include "Scheduler.h"
#include "gameak/core/Identity.h"
#include "gameak/core/Result.h"

#include <cstdint>
#include <memory>
#include <unordered_map>
#include <vector>

namespace gameak::runtime {

enum class LogLevel : uint32_t {
    Trace,
    Debug,
    Info,
    Warn,
    Error,
    Critical,
    Off,
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
    size_t block_count(uint32_t type_id) const;

    const RuntimeConfig& config() const { return config_; }

    Scheduler& scheduler() { return *scheduler_; }
    const Scheduler& scheduler() const { return *scheduler_; }
    const std::unordered_map<core::Identity, DataBlock>& blocks() const { return blocks_; }
    std::unordered_map<core::Identity, DataBlock>& mutable_blocks() { return blocks_; }
    const std::unordered_map<uint32_t, BlockTypeDescriptor>& block_types() const { return types_; }
    uint64_t& next_identity() { return next_identity_; }

private:
    void apply_log_level(LogLevel level);

    RuntimeConfig config_;
    std::unique_ptr<Scheduler> scheduler_;
    std::unordered_map<core::Identity, DataBlock> blocks_;
    std::unordered_map<uint32_t, BlockTypeDescriptor> types_;
    std::vector<Controller> controllers_;
    CommandId next_command_id_{0};
    uint64_t next_identity_{0};
};

} // namespace gameak::runtime
