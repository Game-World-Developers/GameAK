#pragma once

#include "block_manager.h"
#include "block_type.h"
#include "command.h"
#include "controller.h"
#include "event_bus.h"
#include "fifo_scheduler.h"
#include "relationship_manager.h"
#include "layout_manager.h"
#include "layout_strategy.h"
#include "runtime_builder.h"
#include "runtime_config.h"
#include "runtime_diagnostics.h"
#include "runtime_result.h"
#include "runtime_snapshot.h"
#include "GameAk/Core/identity.h"
#include "GameAk/Core/result.h"
#include "GameAk/Core/flat_vector.h"

#include <algorithm>
#include <cstdint>
#include <functional>
#include <span>
#include "GameAk/Core/logging.h"
#include <unordered_map>
#include <vector>

namespace gameak::runtime {

template <typename SchedulerType = FifoScheduler>
class Runtime {
public:
    explicit Runtime(RuntimeConfig config = {});
    ~Runtime();

    static RuntimeBuilder<> configure() {
        return {};
    }

    Runtime(const Runtime&) = delete;
    Runtime& operator=(const Runtime&) = delete;
    Runtime(Runtime&&) = default;
    Runtime& operator=(Runtime&&) = default;

    core::Result<void> register_block_type(BlockTypeDescriptor descriptor);
    core::Result<void> register_controller(Controller controller);
    core::Result<void> register_controller(Controller controller, int priority);

    core::Result<core::Identity> create_block(uint32_t type_id) {
        return block_mgr_.create(type_id, types_, next_identity_);
    }
    core::Result<void> destroy_block(core::Identity identity) {
        return block_mgr_.destroy(identity);
    }

    core::Result<CommandId> submit_command(Command command);
    core::Result<void> cancel_command(CommandId id);

    core::Result<void> replay_command(const Command& command);
    const std::vector<Command>& command_history() const { return scheduler_.history(); }

    TickResult tick(float time_delta = kDefaultTimeDelta);

    // Pause / Resume
    void pause() { paused_ = true; }
    void resume() { paused_ = false; }
    bool is_paused() const { return paused_; }

    // Fixed timestep
    void set_fixed_timestep(float dt) { fixed_timestep_ = dt; accumulator_ = 0.0f; }
    void clear_fixed_timestep() { fixed_timestep_ = 0.0f; accumulator_ = 0.0f; }
    float fixed_timestep() const { return fixed_timestep_; }

    bool has_block(core::Identity identity) const { return block_mgr_.has(identity); }

    const DataBlock* get_block(core::Identity identity) const {
        return block_mgr_.get_block(identity);
    }

    std::vector<core::Identity> find_blocks_by_type(uint32_t type_id) const {
        return block_mgr_.find_by_type(type_id);
    }
    std::vector<core::Identity> find_blocks(
        std::function<bool(const DataBlock&)> pred) const {
        return block_mgr_.find(std::move(pred));
    }

    size_t block_count(uint32_t type_id) const { return block_mgr_.count(type_id); }

    const RuntimeConfig& config() const { return config_; }

    SchedulerType& scheduler() { return scheduler_; }
    const SchedulerType& scheduler() const { return scheduler_; }
    const std::unordered_map<core::Identity, DataBlock>& raw_blocks() const { return block_mgr_.ref_blocks(); }
    const std::unordered_map<uint32_t, BlockTypeDescriptor>& block_types() const { return types_; }
    size_t pending_command_count() const { return scheduler_.pending_count(); }

    // ── Event System ───────────────────────────────────────────────
    using EventType = EventBus::Type;
    using Event     = EventBus::Event;
    using EventHandler = EventBus::Handler;
    using EventId      = EventBus::Id;

    EventId listen(EventType type, EventHandler handler) {
        return event_bus_.listen(type, std::move(handler));
    }
    void unlisten(EventId id) { event_bus_.unlisten(id); }

    EventId on_tick_begin(EventHandler handler) {
        return event_bus_.on_tick_begin(std::move(handler));
    }
    EventId on_tick_end(EventHandler handler) {
        return event_bus_.on_tick_end(std::move(handler));
    }
    EventId on_block_created(EventHandler handler) {
        return event_bus_.on_block_created(std::move(handler));
    }
    EventId on_block_destroyed(EventHandler handler) {
        return event_bus_.on_block_destroyed(std::move(handler));
    }

    // ── Conversational Callbacks ───────────────────────────────────
    EventId before_tick(std::function<void()> handler) {
        return event_bus_.on_tick_begin(
            [handler = std::move(handler)](const Event&) { handler(); });
    }

    EventId after_tick(std::function<void(const TickResult&)> handler) {
        after_tick_handlers_.push_back(std::move(handler));
        return ++next_after_tick_id_;
    }

    EventId when_block_created(std::function<void(core::Identity, uint32_t)> handler) {
        return event_bus_.on_block_created(
            [handler = std::move(handler)](const Event& e) { handler(e.identity, e.block_type_id); });
    }

    EventId when_block_destroyed(std::function<void(core::Identity)> handler) {
        return event_bus_.on_block_destroyed(
            [handler = std::move(handler)](const Event& e) { handler(e.identity); });
    }

    // ── Conversation-style query wrappers ───────────────────────────
    std::vector<core::Identity> blocks_of_type(uint32_t type_id) const {
        return find_blocks_by_type(type_id);
    }

    std::vector<core::Identity> blocks_where(
        std::function<bool(const DataBlock&)> pred) const {
        return find_blocks(std::move(pred));
    }

    // ── Serialization API ──────────────────────────────────────────
    Snapshot save() const;
    void load(const Snapshot& snapshot);

    // ── Block Relationships API ────────────────────────────────────
    core::Result<void> relate(core::Identity parent, core::Identity child) {
        return rel_mgr_.relate(parent, child, &block_mgr_.ref_blocks());
    }
    core::Result<void> unrelate(core::Identity parent, core::Identity child) {
        return rel_mgr_.unrelate(parent, child);
    }

    // ── Conversational Relationship DSL ────────────────────────────
    class RelationshipBuilder {
        Runtime* rt_;
        core::Identity parent_;
    public:
        explicit RelationshipBuilder(Runtime* rt, core::Identity parent)
            : rt_{rt}, parent_{parent} {}
        core::Result<void> to(core::Identity child) {
            return rt_->relate(parent_, child);
        }
    };

    class UnrelateBuilder {
        Runtime* rt_;
        core::Identity parent_;
    public:
        explicit UnrelateBuilder(Runtime* rt, core::Identity parent)
            : rt_{rt}, parent_{parent} {}
        core::Result<void> from(core::Identity child) {
            return rt_->unrelate(parent_, child);
        }
    };

    class BlockTypeBuilder {
        Runtime* rt_;
        std::string name_;
        BlockTypeDescriptor desc_{};
        bool has_explicit_id_{false};
    public:
        explicit BlockTypeBuilder(Runtime* rt, const char* name)
            : rt_{rt}, name_{name} {}

        template <typename T>
        BlockTypeBuilder& size_of() {
            desc_.size = sizeof(T);
            desc_.alignment = alignof(T);
            return *this;
        }

        BlockTypeBuilder& size(size_t s) { desc_.size = s; return *this; }
        BlockTypeBuilder& align(size_t a) { desc_.alignment = a; return *this; }

        template <typename T, typename U>
        BlockTypeBuilder& has(const char* field_name, U T::*member) {
            desc_.fields.push_back({field_name,
                reinterpret_cast<size_t>(&(static_cast<T*>(nullptr)->*member)),
                sizeof(U), alignof(U)});
            return *this;
        }

        BlockTypeBuilder& has(const char* field_name, size_t offset, size_t size) {
            desc_.fields.push_back({field_name, offset, size, 1});
            return *this;
        }

        BlockTypeBuilder& ephemeral() { desc_.ephemeral = true; return *this; }

        BlockTypeBuilder& semantic(const core::SemanticConstraint& sc) {
            desc_.semantic = &sc;
            return *this;
        }

        BlockTypeBuilder& id(uint32_t type_id) {
            desc_.type_id = type_id;
            has_explicit_id_ = true;
            return *this;
        }

        core::Result<void> done() && {
            desc_.name = std::move(name_);
            if (!has_explicit_id_) {
                desc_.type_id = rt_->next_block_type_id_++;
            }
            if (desc_.alignment == 0) desc_.alignment = 1;
            return rt_->register_block_type(std::move(desc_));
        }
    };

    class BlockQuery {
        const std::unordered_map<core::Identity, DataBlock>* blocks_;
        const std::unordered_map<std::string, uint32_t>* type_names_;
        uint32_t filter_type_{0};
        bool has_type_filter_{false};
        std::function<bool(const DataBlock&)> filter_pred_;
    public:
        explicit BlockQuery(const std::unordered_map<core::Identity, DataBlock>* blocks,
                            const std::unordered_map<std::string, uint32_t>* type_names)
            : blocks_{blocks}, type_names_{type_names} {}

        BlockQuery& of_type(uint32_t type_id) {
            filter_type_ = type_id;
            has_type_filter_ = true;
            return *this;
        }

        BlockQuery& of_type(const char* name) {
            auto it = type_names_->find(name);
            if (it != type_names_->end()) {
                filter_type_ = it->second;
                has_type_filter_ = true;
            }
            return *this;
        }

        BlockQuery& where(std::function<bool(const DataBlock&)> pred) {
            filter_pred_ = std::move(pred);
            return *this;
        }

        size_t count() const {
            size_t n = 0;
            for_each([&](const DataBlock&) { ++n; });
            return n;
        }

        template <typename U>
        std::vector<U> map(std::function<U(const DataBlock&)> f) const {
            std::vector<U> out;
            for_each([&](const DataBlock& b) { out.push_back(f(b)); });
            return out;
        }

        void each(std::function<void(const DataBlock&)> f) const {
            for_each(std::move(f));
        }

        bool any(std::function<bool(const DataBlock&)> pred) const {
            for (auto& [_, b] : *blocks_) {
                if (has_type_filter_ && b.type_id() != filter_type_) continue;
                if ((!filter_pred_ || filter_pred_(b)) && pred(b)) {
                    return true;
                }
            }
            return false;
        }

        bool all(std::function<bool(const DataBlock&)> pred) const {
            for (auto& [_, b] : *blocks_) {
                if (has_type_filter_ && b.type_id() != filter_type_) continue;
                if ((!filter_pred_ || filter_pred_(b)) && !pred(b)) {
                    return false;
                }
            }
            return true;
        }

        const DataBlock* first(std::function<bool(const DataBlock&)> pred) const {
            for (auto& [_, b] : *blocks_) {
                if (has_type_filter_ && b.type_id() != filter_type_) continue;
                if ((!filter_pred_ || filter_pred_(b)) && pred(b)) {
                    return &b;
                }
            }
            return nullptr;
        }

    private:
        void for_each(std::function<void(const DataBlock&)> f) const {
            for (auto& [_, b] : *blocks_) {
                if (has_type_filter_ && b.type_id() != filter_type_) continue;
                if (filter_pred_ && !filter_pred_(b)) continue;
                f(b);
            }
        }
    };

    // ── Block Type DSL ──────────────────────────────────────────────
    BlockTypeBuilder define(const char* name) {
        return BlockTypeBuilder{this, name};
    }

    template <typename T>
    BlockTypeBuilder define(const char* name) {
        BlockTypeBuilder builder{this, name};
        builder.template size_of<T>();
        return builder;
    }

    // ── Fluent Query DSL ────────────────────────────────────────────
    BlockQuery blocks() const {
        return BlockQuery{&block_mgr_.ref_blocks(), &type_name_to_id_};
    }

    RelationshipBuilder relate(core::Identity parent) {
        return RelationshipBuilder{this, parent};
    }

    UnrelateBuilder unrelate(core::Identity parent) {
        return UnrelateBuilder{this, parent};
    }
    std::vector<core::Identity> children_of(core::Identity parent) const {
        return rel_mgr_.children_of(parent);
    }
    std::vector<core::Identity> parents_of(core::Identity child) const {
        return rel_mgr_.parents_of(child);
    }

    // ── Layout API ─────────────────────────────────────────────────
    void convert_to_soa(uint32_t type_id)   { layout_mgr_.convert_to_soa(type_id, block_mgr_.mut_blocks(), types_); }
    void convert_from_soa(uint32_t type_id) { layout_mgr_.convert_from_soa(type_id, block_mgr_.mut_blocks(), types_); }
    void convert_to_aosoa(uint32_t type_id)   { layout_mgr_.convert_to_aosoa(type_id, block_mgr_.mut_blocks(), types_); }
    void convert_from_aosoa(uint32_t type_id) { layout_mgr_.convert_from_aosoa(type_id, block_mgr_.mut_blocks(), types_); }

    LayoutStrategy get_layout(uint32_t type_id) const {
        auto it = types_.find(type_id);
        if (it == types_.end()) return LayoutStrategy::AoS;
        return it->second.layout;
    }

    bool has_layout_storage(uint32_t type_id) const { return layout_mgr_.has_storage(type_id); }
    size_t soa_block_count(uint32_t type_id)    const { return layout_mgr_.soa_block_count(type_id); }
    std::span<const std::byte> soa_field_data(uint32_t type_id, size_t field_index) const {
        return layout_mgr_.soa_field_data(type_id, field_index);
    }
    size_t soa_field_count(uint32_t type_id) const { return layout_mgr_.soa_field_count(type_id); }
    size_t aosoa_block_count(uint32_t type_id)    const { return layout_mgr_.aosoa_block_count(type_id); }
    size_t aosoa_chunk_count(uint32_t type_id)    const { return layout_mgr_.aosoa_chunk_count(type_id); }
    std::span<const core::Identity> aosoa_chunk_identities(uint32_t type_id, size_t chunk_index) const {
        return layout_mgr_.aosoa_chunk_identities(type_id, chunk_index);
    }
    std::span<const std::byte> aosoa_field_data(uint32_t type_id, size_t chunk_index, size_t field_index) const {
        return layout_mgr_.aosoa_field_data(type_id, chunk_index, field_index);
    }
    size_t aosoa_field_count(uint32_t type_id) const { return layout_mgr_.aosoa_field_count(type_id); }

    // ── Ephemeral API ──────────────────────────────────────────────
    core::Result<core::Identity> create_ephemeral_block(uint32_t type_id) {
        return block_mgr_.create_ephemeral(type_id, types_, next_identity_);
    }
    void destroy_all_ephemeral() {
        block_mgr_.destroy_all_ephemeral(types_);
    }

    bool is_ephemeral_type(uint32_t type_id) const {
        auto it = types_.find(type_id);
        if (it == types_.end()) return false;
        return it->second.ephemeral;
    }

    // ── Diagnostics API ────────────────────────────────────────────
    Diagnostics collect_diagnostics() const {
        return Diagnostics{
            .pending_commands    = scheduler_.pending_count(),
            .commands_executed   = scheduler_.executed_count(),
            .commands_rejected   = scheduler_.rejected_count(),
            .commands_skipped    = scheduler_.skipped_count(),
            .blocks_count        = block_mgr_.total_block_count(),
            .controllers_count   = controllers_.size(),
            .next_identity       = next_identity_,
        };
    }

private:
    TickResult execute_single_tick(float time_delta);

    void apply_log_level(LogLevel level);

    RuntimeConfig config_;
    SchedulerType scheduler_;
    std::unordered_map<uint32_t, BlockTypeDescriptor> types_;
    gameak::core::flat_vector<ControllerEntry, 4> controllers_;

    CommandId next_command_id_{0};
    uint64_t next_identity_{0};

    bool paused_{false};
    float fixed_timestep_{0.0f};
    float accumulator_{0.0f};

    EventBus event_bus_;
    LayoutManager layout_mgr_;
    BlockManager block_mgr_;
    RelationshipManager rel_mgr_;

    std::unordered_map<std::string, uint32_t> type_name_to_id_;
    uint32_t next_block_type_id_{1};

    std::vector<std::function<void(const TickResult&)>> after_tick_handlers_;
    uint64_t next_after_tick_id_{0};
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
#ifdef GAME_AK_HAVE_SPDLOG
    switch (level) {
        case LogLevel::Trace: spdlog::set_level(spdlog::level::trace); break;
        case LogLevel::Debug: spdlog::set_level(spdlog::level::debug); break;
        case LogLevel::Info:  spdlog::set_level(spdlog::level::info);  break;
        case LogLevel::Warn:  spdlog::set_level(spdlog::level::warn);  break;
        case LogLevel::Error: spdlog::set_level(spdlog::level::err);   break;
        case LogLevel::Critical: spdlog::set_level(spdlog::level::critical); break;
        case LogLevel::Off:   spdlog::set_level(spdlog::level::off);   break;
    }
#else
    (void)level;
#endif
}

template <typename S>
core::Result<void> Runtime<S>::register_block_type(BlockTypeDescriptor descriptor) {
    if (this->types_.contains(descriptor.type_id)) {
        return core::Error{core::ErrorCode::DuplicateRegistration, "Block type already registered"};
    }
    if (!descriptor.name.empty() && this->type_name_to_id_.contains(descriptor.name)) {
        return core::Error{core::ErrorCode::DuplicateRegistration, "Block type name already registered"};
    }

    if (descriptor.semantic) {
        if (descriptor.size == 0) {
            if (!descriptor.semantic->valid()) {
                return core::Error{core::ErrorCode::InvalidOperation,
                                   "Semantic constraint present but invalid"};
            }
            size_t inferred = core::bytes_for(*descriptor.semantic);
            if (inferred == 0) {
                return core::Error{core::ErrorCode::InvalidOperation,
                                   "Semantic constraint present but could not infer size"};
            }
            descriptor.size = inferred;
            if (descriptor.alignment == 0) {
                descriptor.alignment = inferred;
            }
        }
    }

    this->types_[descriptor.type_id] = descriptor;
    this->type_name_to_id_[descriptor.name] = descriptor.type_id;
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

    this->event_bus_.fire(EventBus::Type::TickBegin);

    this->scheduler_.reset_counts();

    // Controller Execution Phase
    // Sort by priority (higher = first); stable to preserve registration order for equal priorities
    std::stable_sort(this->controllers_.begin(), this->controllers_.end(),
        [](const ControllerEntry& a, const ControllerEntry& b) {
            return a.priority > b.priority;
        });

    size_t controller_count = 0;
    for (auto& entry : this->controllers_) {
        StateView state_view{this->block_mgr_.ref_blocks(), this->types_, time_delta};
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
    bool track_blocks = this->event_bus_.has_handler(EventBus::Type::BlockCreated) ||
                        this->event_bus_.has_handler(EventBus::Type::BlockDestroyed);
    std::unordered_map<core::Identity, DataBlock> before_blocks;
    if (track_blocks) {
        before_blocks = this->block_mgr_.ref_blocks();
    }

    auto process_result = this->scheduler_.process_pending(
        this->block_mgr_.mut_blocks(), this->types_, this->next_identity_);

    // Fire block events by diffing before/after state
    if (track_blocks) {
        for (const auto& [id, block] : this->block_mgr_.ref_blocks()) {
            if (!before_blocks.contains(id)) {
                this->event_bus_.fire(EventBus::Type::BlockCreated, id, block.type_id());
            }
        }
        for (const auto& [id, block] : before_blocks) {
            (void)block;
            if (!this->block_mgr_.has(id)) {
                this->event_bus_.fire(EventBus::Type::BlockDestroyed, id, 0);
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

    this->block_mgr_.rebuild_counts();

    // Destroy ephemeral blocks before TickEnd event
    this->destroy_all_ephemeral();

    // Fire after_tick callbacks with the TickResult
    for (auto& handler : this->after_tick_handlers_) {
        handler(result);
    }

    this->event_bus_.fire(EventBus::Type::TickEnd);

    return result;
}

// ── Serialization ──────────────────────────────────────────────────

template <typename S>
Snapshot Runtime<S>::save() const {
    std::unordered_map<core::Identity, DataBlock> persistent_blocks;
    for (const auto& [id, block] : this->block_mgr_.ref_blocks()) {
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
    this->block_mgr_.set_blocks(snapshot.blocks);
    this->types_         = snapshot.types;
    this->next_identity_ = snapshot.next_identity;
}

} // namespace gameak::runtime
