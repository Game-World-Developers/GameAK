#pragma once

#include "cmd_create_handler.h"
#include "cmd_destroy_handler.h"
#include "cmd_convert_layout_handler.h"
#include "cmd_resize_handler.h"
#include "cmd_set_field_handler.h"

#include "GameAk/Core/rb_tree.h"

#include <memory>

namespace gameak::runtime::detail {

class CommandDispatcher {
public:
    CommandDispatcher() {
        register_handler(std::make_unique<CreateBlockHandler>());
        register_handler(std::make_unique<DestroyBlockHandler>());
        register_handler(std::make_unique<SetFieldHandler>());
        register_handler(std::make_unique<ResizeBlockHandler>());
        register_handler(std::make_unique<ConvertLayoutHandler>());
    }

    void register_handler(std::unique_ptr<ICommandHandler> handler) {
        handlers_.insert(handler->type(), std::move(handler));
    }

    core::Result<void> validate(
        const Command& command,
        const core::rb_tree<core::Identity, DataBlock>& blocks,
        const core::rb_tree<uint32_t, BlockTypeDescriptor>& types) const
    {
        auto it = handlers_.find(command.type());
        if (it == handlers_.end()) {
            return core::Error{core::ErrorCode::InternalError, "Unknown command type"};
        }
        return it->second->validate(command.payload(), blocks, types);
    }

    core::Result<void> execute(
        Command& command,
        CommandContext& ctx) const
    {
        auto it = handlers_.find(command.type());
        if (it == handlers_.end()) {
            return core::Error{core::ErrorCode::InternalError, "Unknown command type"};
        }
        return it->second->execute(command.payload(), ctx);
    }

private:
    core::rb_tree<CommandType, std::unique_ptr<ICommandHandler>> handlers_;
};

// Global dispatcher instance (lazy, header-only)
inline CommandDispatcher& get_dispatcher() {
    static CommandDispatcher disp;
    return disp;
}

// Free functions (drop-in replacements)
inline core::Result<void> validate_command(
    const Command& command,
    const core::rb_tree<core::Identity, DataBlock>& blocks,
    const core::rb_tree<uint32_t, BlockTypeDescriptor>& types)
{
    return get_dispatcher().validate(command, blocks, types);
}

inline core::Result<void> execute_command(
    Command& command,
    CommandContext& ctx)
{
    return get_dispatcher().execute(command, ctx);
}

} // namespace gameak::runtime::detail
