#pragma once

#include "controller.h"
#include "GameAk/Core/result.h"

#include <algorithm>
#include <functional>
#include <memory>
#include <queue>
#include <unordered_map>
#include <vector>

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

    EventLoop() : queue_(std::make_shared<std::queue<PendingEvent>>()) {}

    HandlerId on(EventTypeId type, Handler handler) {
        HandlerId id = next_id_++;
        handlers_[type].emplace_back(id, std::move(handler));
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
        queue_->push({type, std::move(data)});
    }

    size_t pending_count() const {
        return queue_->size();
    }

    Controller build() {
        auto handlers = std::make_shared<HandlerMap>(handlers_);
        auto queue = queue_;

        return [handlers, queue](StateView&, CommandProducer& producer, EphemeralProducer& ephem) -> core::Result<void> {

            while (!queue->empty()) {
                auto event = std::move(queue->front());
                queue->pop();

                auto it = handlers->find(event.type);
                if (it == handlers->end()) continue;

                for (auto& [id, handler] : it->second) {
                    (void)id;
                    handler(event.data, producer, ephem);
                }
            }

            return {};
        };
    }

private:
    struct PendingEvent {
        EventTypeId type;
        EventData data;
    };

    using HandlerMap = std::unordered_map<EventTypeId, std::vector<std::pair<HandlerId, Handler>>>;

    HandlerMap handlers_;
    std::shared_ptr<std::queue<PendingEvent>> queue_;
    HandlerId next_id_{1};
};

} // namespace gameak::runtime
