#pragma once

#include "block_type.h"
#include "command.h"
#include "controller.h"
#include "fifo_scheduler.h"
#include "layout_strategy.h"
#include "GameAk/Core/identity.h"
#include "GameAk/Core/result.h"
#include "GameAk/Core/flat_vector.h"

#include <algorithm>
#include <cstdint>
#include <functional>
#include "GameAk/Core/logging.h"
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
    core::Result<void> register_controller(Controller controller, int priority);

    core::Result<core::Identity> create_block(uint32_t type_id);
    core::Result<void> destroy_block(core::Identity identity);

    core::Result<CommandId> submit_command(Command command);
    core::Result<void> cancel_command(CommandId id);

    core::Result<void> replay_command(const Command& command);
    const std::vector<Command>& command_history() const { return scheduler_.history(); }

    TickResult tick(float time_delta = 0.016f);

    // Pause / Resume
    void pause() { paused_ = true; }
    void resume() { paused_ = false; }
    bool is_paused() const { return paused_; }

    // Fixed timestep
    void set_fixed_timestep(float dt) { fixed_timestep_ = dt; accumulator_ = 0.0f; }
    void clear_fixed_timestep() { fixed_timestep_ = 0.0f; accumulator_ = 0.0f; }
    float fixed_timestep() const { return fixed_timestep_; }

    bool has_block(core::Identity identity) const;

    // Query operations
    std::vector<core::Identity> find_blocks_by_type(uint32_t type_id) const;
    std::vector<core::Identity> find_blocks(
        std::function<bool(const DataBlock&)> pred) const;

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

    // ── Event System ───────────────────────────────────────────────
    enum class EventType : uint32_t {
        TickBegin,
        TickEnd,
        BlockCreated,
        BlockDestroyed,
    };

    struct Event {
        EventType type;
        core::Identity identity; // populated for BlockCreated / BlockDestroyed
        uint32_t block_type_id;  // populated for BlockCreated
    };

    using EventHandler = std::function<void(const Event&)>;
    using EventId = uint64_t;

    /// Register a callback for a specific event type.
    /// Returns an EventId that can be used to unlisten.
    EventId listen(EventType type, EventHandler handler);

    /// Remove a previously registered event handler.
    void unlisten(EventId id);

    // ── Serialization API ──────────────────────────────────────────
    /// A snapshot of the runtime state that can be saved and restored.
    struct Snapshot {
        std::unordered_map<core::Identity, DataBlock> blocks;
        std::unordered_map<uint32_t, BlockTypeDescriptor> types;
        uint64_t next_identity{0};
    };

    Snapshot save() const;
    void load(const Snapshot& snapshot);

    // ── Block Relationships API ────────────────────────────────────
    /// Create a parent→child relationship between two blocks.
    core::Result<void> relate(core::Identity parent, core::Identity child);

    /// Remove a parent→child relationship.
    core::Result<void> unrelate(core::Identity parent, core::Identity child);

    /// Get all children of a given block.
    std::vector<core::Identity> children_of(core::Identity parent) const;

    /// Get all parents of a given block.
    std::vector<core::Identity> parents_of(core::Identity child) const;

    // ── Layout API ─────────────────────────────────────────────────
    /// Convert all blocks of a type to SoA storage.
    void convert_to_soa(uint32_t type_id);
    /// Convert all blocks of a type back to AoS storage.
    void convert_from_soa(uint32_t type_id);

    LayoutStrategy get_layout(uint32_t type_id) const {
        auto it = types_.find(type_id);
        if (it == types_.end()) return LayoutStrategy::AoS;
        return it->second.layout;
    }

    bool has_layout_storage(uint32_t type_id) const {
        return layout_storage_.contains(type_id);
    }

    /// Get number of blocks stored in SoA mode for a type.
    size_t soa_block_count(uint32_t type_id) const {
        auto it = layout_storage_.find(type_id);
        if (it == layout_storage_.end()) return 0;
        return it->second.identities.size();
    }

    /// Get the raw SoA field data array for inspection/testing.
    const std::vector<std::byte>* soa_field_data(uint32_t type_id, size_t field_index) const {
        auto it = layout_storage_.find(type_id);
        if (it == layout_storage_.end()) return nullptr;
        if (field_index >= it->second.fields.size()) return nullptr;
        return &it->second.fields[field_index].data;
    }

    /// Get the number of SoA fields for a type.
    size_t soa_field_count(uint32_t type_id) const {
        auto it = layout_storage_.find(type_id);
        if (it == layout_storage_.end()) return 0;
        return it->second.fields.size();
    }

    // ── Ephemeral API ──────────────────────────────────────────────
    /// Create an ephemeral block directly. Only valid for types with ephemeral=true.
    core::Result<core::Identity> create_ephemeral_block(uint32_t type_id);

    /// Destroy all ephemeral data blocks. Called automatically at TickEnd.
    void destroy_all_ephemeral();

    bool is_ephemeral_type(uint32_t type_id) const {
        auto it = types_.find(type_id);
        if (it == types_.end()) return false;
        return it->second.ephemeral;
    }

    // ── Diagnostics API ────────────────────────────────────────────
    struct Diagnostics {
        size_t pending_commands{0};
        size_t commands_executed{0};
        size_t commands_rejected{0};
        size_t commands_skipped{0};
        size_t blocks_count{0};
        size_t controllers_count{0};
        uint64_t next_identity{0};
    };

    Diagnostics collect_diagnostics() const {
        return Diagnostics{
            .pending_commands    = scheduler_.pending_count(),
            .commands_executed   = scheduler_.executed_count(),
            .commands_rejected   = scheduler_.rejected_count(),
            .commands_skipped    = scheduler_.skipped_count(),
            .blocks_count        = blocks_.size(),
            .controllers_count   = controllers_.size(),
            .next_identity       = next_identity_,
        };
    }

private:
    struct SoAFieldArray {
        std::vector<std::byte> data; // flat array of N * field_size
        size_t field_size{0};
        size_t field_offset{0};
        size_t field_alignment{1};
    };

    struct SoAStorage {
        std::vector<core::Identity> identities;
        std::vector<SoAFieldArray> fields;
    };

    TickResult execute_single_tick(float time_delta);

    void fire_event(EventType type,
                    core::Identity identity = core::Identity{},
                    uint32_t block_type_id = {});

    void apply_log_level(LogLevel level);
    void rebuild_type_counts();

    // Layout management
    void ensure_soa_storage(uint32_t type_id);

    RuntimeConfig config_;
    SchedulerType scheduler_;
    std::unordered_map<core::Identity, DataBlock> blocks_;
    std::unordered_map<uint32_t, BlockTypeDescriptor> types_;
    gameak::core::flat_vector<ControllerEntry, 4> controllers_;

    CommandId next_command_id_{0};
    uint64_t next_identity_{0};
    std::unordered_map<uint32_t, size_t> type_counts_;

    bool paused_{false};
    float fixed_timestep_{0.0f};
    float accumulator_{0.0f};

    // Event system
    std::unordered_map<EventType, std::vector<std::pair<EventId, EventHandler>>> event_handlers_;
    EventId next_event_id_{1};

    // Block relationships (bidirectional adjacency)
    std::unordered_multimap<core::Identity, core::Identity> parent_to_children_;
    std::unordered_multimap<core::Identity, core::Identity> child_to_parents_;

    // Per-type layout storage (for SoA/AoSoA)
    std::unordered_map<uint32_t, SoAStorage> layout_storage_;
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
    return register_controller(std::move(controller), 0);
}

template <typename S>
core::Result<void> Runtime<S>::register_controller(Controller controller, int priority) {
    if (!controller) {
        return core::Error{core::ErrorCode::InvalidOperation, "Controller is empty"};
    }
    this->controllers_.push_back(ControllerEntry{std::move(controller), priority});
    SPDLOG_DEBUG("Registered controller with priority {} (total={})", priority, this->controllers_.size());
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
    this->type_counts_[type_id]++;
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
    auto type_id = it->second.type_id();
    this->blocks_.erase(it);
    auto type_it = this->type_counts_.find(type_id);
    if (type_it != this->type_counts_.end() && type_it->second > 0) {
        type_it->second--;
    }
    return {};
}

template <typename S>
core::Result<CommandId> Runtime<S>::submit_command(Command command) {
    CommandId id = ++this->next_command_id_;
    command.set_id(id);
    this->scheduler_.enqueue(std::move(command));
    return id;
}

template <typename S>
core::Result<void> Runtime<S>::cancel_command(CommandId id) {
    this->scheduler_.cancel(id);
    return {};
}

template <typename S>
core::Result<void> Runtime<S>::replay_command(const Command& command) {
    auto result = this->submit_command(Command{command.payload()});
    if (!result) return result.error();
    return {};
}

template <typename S>
typename Runtime<S>::EventId Runtime<S>::listen(EventType type, EventHandler handler) {
    EventId id = this->next_event_id_++;
    this->event_handlers_[type].emplace_back(id, std::move(handler));
    return id;
}

template <typename S>
void Runtime<S>::unlisten(EventId id) {
    for (auto& [type, handlers] : this->event_handlers_) {
        (void)type;
        auto it = std::remove_if(handlers.begin(), handlers.end(),
            [id](const auto& pair) { return pair.first == id; });
        if (it != handlers.end()) {
            handlers.erase(it, handlers.end());
            return;
        }
    }
}

template <typename S>
void Runtime<S>::fire_event(EventType type,
                            core::Identity identity,
                            uint32_t block_type_id)
{
    auto it = this->event_handlers_.find(type);
    if (it == this->event_handlers_.end()) return;
    Event event{type, identity, block_type_id};
    for (const auto& [id, handler] : it->second) {
        (void)id;
        handler(event);
    }
}

template <typename S>
TickResult Runtime<S>::tick(float time_delta) {
    // Paused → skip everything
    if (this->paused_) {
        return TickResult{};
    }

    // Fixed timestep mode → accumulate and run sub-ticks
    if (this->fixed_timestep_ > 0.0f) {
        this->accumulator_ += time_delta;
        TickResult combined;
        while (this->accumulator_ >= this->fixed_timestep_) {
            auto sub = execute_single_tick(this->fixed_timestep_);
            combined.commands_executed    += sub.commands_executed;
            combined.commands_rejected    += sub.commands_rejected;
            combined.controllers_executed += sub.controllers_executed;

            // Worst status wins
            if (sub.status == ExecutionStatus::CriticalFailure) {
                combined.status = ExecutionStatus::CriticalFailure;
            } else if (sub.status == ExecutionStatus::PartialFailure &&
                       combined.status != ExecutionStatus::CriticalFailure) {
                combined.status = ExecutionStatus::PartialFailure;
            }

            combined.rejected_commands.insert(
                combined.rejected_commands.end(),
                std::make_move_iterator(sub.rejected_commands.begin()),
                std::make_move_iterator(sub.rejected_commands.end()));

            this->accumulator_ -= this->fixed_timestep_;

            // Stop on critical failure to avoid cascading issues
            if (sub.status == ExecutionStatus::CriticalFailure) break;
        }
        return combined;
    }

    // Variable timestep mode — single tick as before
    return execute_single_tick(time_delta);
}

template <typename S>
TickResult Runtime<S>::execute_single_tick(float time_delta) {
    TickResult result;

    fire_event(EventType::TickBegin);

    this->scheduler_.reset_counts();

    // Controller Execution Phase
    // Sort by priority (higher = first); stable to preserve registration order for equal priorities
    std::stable_sort(this->controllers_.begin(), this->controllers_.end(),
        [](const ControllerEntry& a, const ControllerEntry& b) {
            return a.priority > b.priority;
        });

    size_t controller_count = 0;
    for (auto& entry : this->controllers_) {
        StateView state_view{this->blocks_, this->types_, time_delta};
        CommandProducer producer{
            [this](Command cmd) -> core::Result<CommandId> {
                return this->submit_command(std::move(cmd));
            }
        };
        EphemeralProducer ephem_producer{
            [this](uint32_t type_id) -> core::Result<core::Identity> {
                return this->create_ephemeral_block(type_id);
            }
        };
        auto cr = entry.controller(state_view, producer, ephem_producer);
        if (!cr) {
            SPDLOG_WARN("Controller failed: {}", cr.error().message());
            result.status = ExecutionStatus::PartialFailure;
        }
        controller_count++;
    }
    result.controllers_executed = controller_count;

    // Command Processing Phase
    // Snapshot blocks after controller execution (so ephemeral blocks are in both
    // before and after snapshots, preventing spurious BlockCreated events).
    bool track_blocks = this->event_handlers_.contains(EventType::BlockCreated) ||
                        this->event_handlers_.contains(EventType::BlockDestroyed);
    decltype(this->blocks_) before_blocks;
    if (track_blocks) {
        before_blocks = this->blocks_;
    }

    auto process_result = this->scheduler_.process_pending(this->blocks_, this->types_, this->next_identity_);

    // Fire block events by diffing before/after state
    if (track_blocks) {
        for (const auto& [id, block] : this->blocks_) {
            if (!before_blocks.contains(id)) {
                fire_event(EventType::BlockCreated, id, block.type_id());
            }
        }
        for (const auto& [id, block] : before_blocks) {
            (void)block;
            if (!this->blocks_.contains(id)) {
                fire_event(EventType::BlockDestroyed, id, 0);
            }
        }
    }

    auto rejected = this->scheduler_.take_rejected();
    result.commands_executed = this->scheduler_.executed_count();
    result.commands_rejected = rejected.size();
    result.rejected_commands = std::move(rejected);

    if (!process_result) {
        result.status = ExecutionStatus::CriticalFailure;
    } else if (result.commands_rejected > 0) {
        result.status = ExecutionStatus::PartialFailure;
    } else if (result.status != ExecutionStatus::PartialFailure) {
        result.status = ExecutionStatus::Success;
    }

    this->rebuild_type_counts();

    // Destroy ephemeral blocks before TickEnd event
    this->destroy_all_ephemeral();

    fire_event(EventType::TickEnd);

    return result;
}

template <typename S>
bool Runtime<S>::has_block(core::Identity identity) const {
    return this->blocks_.contains(identity);
}

template <typename S>
std::vector<core::Identity> Runtime<S>::find_blocks_by_type(uint32_t type_id) const {
    return find_blocks([type_id](const DataBlock& block) {
        return block.type_id() == type_id;
    });
}

template <typename S>
std::vector<core::Identity> Runtime<S>::find_blocks(
    std::function<bool(const DataBlock&)> pred) const
{
    std::vector<core::Identity> result;
    for (const auto& [id, block] : this->blocks_) {
        if (pred(block)) {
            result.push_back(id);
        }
    }
    return result;
}

// ── Serialization ──────────────────────────────────────────────────

template <typename S>
typename Runtime<S>::Snapshot Runtime<S>::save() const {
    // Filter out ephemeral blocks from snapshot
    decltype(this->blocks_) persistent_blocks;
    for (const auto& [id, block] : this->blocks_) {
        auto tit = this->types_.find(block.type_id());
        if (tit != this->types_.end() && tit->second.ephemeral) {
            continue;
        }
        persistent_blocks.emplace(id, block);
    }
    return Snapshot{
        .blocks        = std::move(persistent_blocks),
        .types         = this->types_,
        .next_identity = this->next_identity_,
    };
}

template <typename S>
void Runtime<S>::load(const Snapshot& snapshot) {
    this->blocks_        = snapshot.blocks;
    this->types_         = snapshot.types;
    this->next_identity_ = snapshot.next_identity;
    this->rebuild_type_counts();
}

// ── Block Relationships ────────────────────────────────────────────

template <typename S>
core::Result<void> Runtime<S>::relate(core::Identity parent, core::Identity child) {
    if (!parent.is_valid() || !child.is_valid()) {
        return core::Error{core::ErrorCode::InvalidIdentity, "Parent or child identity is invalid"};
    }
    if (!this->blocks_.contains(parent)) {
        return core::Error{core::ErrorCode::BlockNotFound, "Parent block not found"};
    }
    if (!this->blocks_.contains(child)) {
        return core::Error{core::ErrorCode::BlockNotFound, "Child block not found"};
    }
    if (parent == child) {
        return core::Error{core::ErrorCode::InvalidOperation, "Block cannot be related to itself"};
    }
    this->parent_to_children_.emplace(parent, child);
    this->child_to_parents_.emplace(child, parent);
    return {};
}

template <typename S>
core::Result<void> Runtime<S>::unrelate(core::Identity parent, core::Identity child) {
    if (!parent.is_valid() || !child.is_valid()) {
        return core::Error{core::ErrorCode::InvalidIdentity, "Parent or child identity is invalid"};
    }
    auto range = this->parent_to_children_.equal_range(parent);
    for (auto it = range.first; it != range.second; ++it) {
        if (it->second == child) {
            this->parent_to_children_.erase(it);
            break;
        }
    }
    auto crange = this->child_to_parents_.equal_range(child);
    for (auto it = crange.first; it != crange.second; ++it) {
        if (it->second == parent) {
            this->child_to_parents_.erase(it);
            break;
        }
    }
    return {};
}

template <typename S>
std::vector<core::Identity> Runtime<S>::children_of(core::Identity parent) const {
    std::vector<core::Identity> result;
    auto range = this->parent_to_children_.equal_range(parent);
    for (auto it = range.first; it != range.second; ++it) {
        result.push_back(it->second);
    }
    return result;
}

template <typename S>
std::vector<core::Identity> Runtime<S>::parents_of(core::Identity child) const {
    std::vector<core::Identity> result;
    auto range = this->child_to_parents_.equal_range(child);
    for (auto it = range.first; it != range.second; ++it) {
        result.push_back(it->second);
    }
    return result;
}

template <typename S>
void Runtime<S>::rebuild_type_counts() {
    this->type_counts_.clear();
    for (const auto& [id, block] : this->blocks_) {
        (void)id;
        this->type_counts_[block.type_id()]++;
    }
}

// ── Ephemeral Methods ─────────────────────────────────────────────

template <typename S>
core::Result<core::Identity> Runtime<S>::create_ephemeral_block(uint32_t type_id) {
    auto it = this->types_.find(type_id);
    if (it == this->types_.end()) {
        return core::Error{core::ErrorCode::TypeNotRegistered, "Block type not registered"};
    }
    if (!it->second.ephemeral) {
        return core::Error{core::ErrorCode::InvalidOperation, "Cannot create ephemeral block for non-ephemeral type"};
    }
    auto& desc = it->second;
    core::Identity id{++this->next_identity_};
    this->blocks_.emplace(id, DataBlock{id, desc.type_id, desc.size, desc.alignment});
    this->type_counts_[type_id]++;
    return id;
}

template <typename S>
void Runtime<S>::destroy_all_ephemeral() {
    std::vector<core::Identity> to_destroy;
    for (auto& [id, block] : this->blocks_) {
        auto tit = this->types_.find(block.type_id());
        if (tit != this->types_.end() && tit->second.ephemeral) {
            to_destroy.push_back(id);
        }
    }
    for (auto& id : to_destroy) {
        auto bit = this->blocks_.find(id);
        if (bit != this->blocks_.end()) {
            uint32_t type_id = bit->second.type_id();
            this->blocks_.erase(bit);
            auto tc_it = this->type_counts_.find(type_id);
            if (tc_it != this->type_counts_.end() && tc_it->second > 0) {
                tc_it->second--;
            }
        }
    }
}

// ── Layout Methods ────────────────────────────────────────────────

template <typename S>
void Runtime<S>::ensure_soa_storage(uint32_t type_id) {
    if (this->layout_storage_.contains(type_id)) return;

    auto it = this->types_.find(type_id);
    if (it == this->types_.end()) return;

    auto& desc = it->second;
    SoAStorage storage;

    // Initialize field arrays from BlockTypeDescriptor fields
    for (auto& field : desc.fields) {
        SoAFieldArray fa;
        fa.field_size     = field.size;
        fa.field_offset   = field.offset;
        fa.field_alignment = field.alignment;
        storage.fields.push_back(std::move(fa));
    }

    // If no explicit fields registered, create one field for the whole block
    if (desc.fields.empty()) {
        SoAFieldArray fa;
        fa.field_size     = desc.size;
        fa.field_offset   = 0;
        fa.field_alignment = desc.alignment;
        storage.fields.push_back(std::move(fa));
    }

    this->layout_storage_[type_id] = std::move(storage);
}

template <typename S>
void Runtime<S>::convert_to_soa(uint32_t type_id) {
    auto& desc = this->types_[type_id];
    if (desc.layout == LayoutStrategy::SoA) return;

    ensure_soa_storage(type_id);
    auto& storage = this->layout_storage_[type_id];

    // Collect existing blocks of this type
    std::vector<std::pair<core::Identity, DataBlock>> existing;
    for (auto& [id, block] : this->blocks_) {
        if (block.type_id() == type_id) {
            existing.emplace_back(id, std::move(block));
        }
    }

    // Remove AoS blocks, they'll be in SoA storage now
    for (auto& [id, _] : existing) {
        (void)_;
        this->blocks_.erase(id);
    }

    // Populate SoA storage
    for (auto& [id, block] : existing) {
        storage.identities.push_back(id);
        for (size_t fi = 0; fi < storage.fields.size(); ++fi) {
            auto& fa = storage.fields[fi];
            size_t old_pos = fa.field_offset;
            size_t copy_size = std::min(fa.field_size, block.size() - old_pos);
            size_t new_pos = (storage.identities.size() - 1) * fa.field_size + fa.field_offset;
            if (fa.data.size() < new_pos + copy_size) {
                fa.data.resize(new_pos + copy_size);
            }
            std::memcpy(fa.data.data() + new_pos,
                        static_cast<const std::byte*>(block.data()) + old_pos,
                        copy_size);
        }
    }

    desc.layout = LayoutStrategy::SoA;
}

template <typename S>
void Runtime<S>::convert_from_soa(uint32_t type_id) {
    auto& desc = this->types_[type_id];
    if (desc.layout != LayoutStrategy::SoA) return;

    auto it = this->layout_storage_.find(type_id);
    if (it == this->layout_storage_.end()) return;

    auto& storage = it->second;

    // Recreate AoS blocks from SoA storage
    for (size_t i = 0; i < storage.identities.size(); ++i) {
        core::Identity id = storage.identities[i];
        DataBlock block{id, type_id, desc.size, desc.alignment};
        for (size_t fi = 0; fi < storage.fields.size(); ++fi) {
            auto& fa = storage.fields[fi];
            size_t src_pos = i * fa.field_size + fa.field_offset;
            size_t copy_size = std::min(fa.field_size, desc.size - fa.field_offset);
            if (src_pos + copy_size <= fa.data.size()) {
                std::memcpy(static_cast<std::byte*>(block.data()) + fa.field_offset,
                            fa.data.data() + src_pos, copy_size);
            }
        }
        this->blocks_.emplace(id, std::move(block));
    }

    this->layout_storage_.erase(type_id);
    desc.layout = LayoutStrategy::AoS;
}

} // namespace gameak::runtime
