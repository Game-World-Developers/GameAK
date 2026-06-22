#pragma once

#include "controller.h"
#include "GameAk/Core/flat_vector.h"
#include "GameAk/Core/result.h"

#include <functional>
#include <memory>
#include <string>

namespace gameak::runtime {

using RuleId = uint64_t;

using RuleCondition = std::function<bool(StateView&)>;
using RuleAction = std::function<void(StateView&, CommandProducer&, EphemeralProducer&)>;

struct RuleDef {
    std::string name;
    RuleCondition condition;
    RuleAction action;
    int priority{0};
};

class RuleSystem {
public:
    RuleId add_rule(RuleDef rule);

    RuleId add_rule(std::string name, RuleCondition condition, RuleAction action, int priority = 0);

    // ── Conversation-style chaining ─────────────────────────────────
    RuleSystem& rule(RuleDef r) {
        add_rule(std::move(r));
        return *this;
    }

    RuleSystem& rule(std::string name, RuleCondition condition, RuleAction action, int priority = 0) {
        add_rule(std::move(name), std::move(condition), std::move(action), priority);
        return *this;
    }

    void remove_rule(RuleId id);

    void clear_rules();

    size_t rule_count() const;

    Controller build();

private:
    struct RuleEntry {
        RuleId id;
        RuleDef def;
    };

    std::shared_ptr<core::flat_vector<RuleEntry, 8>> rules_{std::make_shared<core::flat_vector<RuleEntry, 8>>()};
    RuleId next_id_{1};
};

} // namespace gameak::runtime
