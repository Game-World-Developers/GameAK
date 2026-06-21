#pragma once

#include "controller.h"
#include "GameAk/Core/flat_vector.h"
#include "GameAk/Core/rb_tree.h"
#include "GameAk/Core/result.h"

#include <algorithm>
#include <functional>
#include <memory>

namespace gameak::runtime {

using EventTypeId = uint32_t;
using HandlerId = uint64_t;

/// Event Loop controller.
/// Template parameter EventData is the type of event payload.
/// Event types are identified by integer/enum values (EventTypeId).
template <typename EventData>
class EventLoop {
public:
    using Handler = std::function<void(const EventData&, CommandProducer&, EphemeralProducer&)>;

    EventLoop() : pending_(std::make_shared<core::flat_vector<PendingEvent, 4>>()) {}

    HandlerId on(EventTypeId type, Handler handler) {
        HandlerId id = next_id_++;
        auto it = handlers_.find(type);
        if (it != handlers_.end()) {
            it->second.emplace_back(id, std::move(handler));
        } else {
            handlers_.insert(type, {{id, std::move(handler)}});
        }
        return id;
    }

    // ── Conversation-style chaining ─────────────────────────────────
    EventLoop& handle(EventTypeId type, Handler handler) {
        on(type, std::move(handler));
        return *this;
    }

    void off(HandlerId id) {
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

    void enqueue(EventTypeId type, EventData data) {
        pending_->push_back({type, std::move(data)});
    }

    size_t pending_count() const {
        return pending_->size();
    }

    Controller build() {
        auto handlers = std::make_shared<HandlerMap>(std::move(handlers_));
        auto pending = pending_;

        return [handlers, pending](StateView&, CommandProducer& producer, EphemeralProducer& ephem) -> core::Result<void> {

            for (auto& event : *pending) {
                auto it = handlers->find(event.type);
                if (it == handlers->end()) continue;

                for (auto& [id, handler] : it->second) {
                    (void)id;
                    handler(event.data, producer, ephem);
                }
            }
            pending->clear();

            return {};
        };
    }

private:
    struct PendingEvent {
        EventTypeId type;
        EventData data;
    };

    using HandlerMap = core::rb_tree<EventTypeId, core::flat_vector<std::pair<HandlerId, Handler>, 4>>;

    HandlerMap handlers_;
    std::shared_ptr<core::flat_vector<PendingEvent, 4>> pending_;
    HandlerId next_id_{1};
};

} // namespace gameak::runtime
