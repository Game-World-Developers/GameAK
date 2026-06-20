#pragma once

#include "GameAk/Core/identity.h"

#include <algorithm>
#include <cstdint>
#include <functional>
#include <unordered_map>
#include <vector>
#include <utility>

namespace gameak::runtime {

class EventBus {
public:
    enum class Type : uint32_t {
        TickBegin,
        TickEnd,
        BlockCreated,
        BlockDestroyed,
    };

    struct Event {
        Type type;
        core::Identity identity;
        uint32_t block_type_id;
    };

    using Handler = std::function<void(const Event&)>;
    using Id = uint64_t;

    Id listen(Type type, Handler handler) {
        Id id = ++next_id_;
        handlers_[type].emplace_back(id, std::move(handler));
        return id;
    }

    void unlisten(Id id) {
        for (auto& [type, vec] : handlers_) {
            (void)type;
            auto it = std::remove_if(vec.begin(), vec.end(),
                [id](const auto& pair) { return pair.first == id; });
            if (it != vec.end()) {
                vec.erase(it, vec.end());
                return;
            }
        }
    }

    void fire(Type type,
              core::Identity identity = core::Identity{},
              uint32_t block_type_id = {})
    {
        auto it = handlers_.find(type);
        if (it == handlers_.end()) return;
        Event event{type, identity, block_type_id};
        for (const auto& [id, handler] : it->second) {
            (void)id;
            handler(event);
        }
    }

    Id on_tick_begin(Handler handler) {
        return listen(Type::TickBegin, std::move(handler));
    }

    Id on_tick_end(Handler handler) {
        return listen(Type::TickEnd, std::move(handler));
    }

    Id on_block_created(Handler handler) {
        return listen(Type::BlockCreated, std::move(handler));
    }

    Id on_block_destroyed(Handler handler) {
        return listen(Type::BlockDestroyed, std::move(handler));
    }

    bool has_handler(Type type) const {
        return handlers_.contains(type);
    }

private:
    std::unordered_map<Type, std::vector<std::pair<Id, Handler>>> handlers_;
    Id next_id_{1};
};

} // namespace gameak::runtime
