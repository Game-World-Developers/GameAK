#pragma once

#include "controller.h"
#include "GameAk/Core/result.h"

#include <functional>
#include <memory>
#include <string>
#include <vector>

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

    void remove_rule(RuleId id);

    void clear_rules();

    size_t rule_count() const;

    Controller build();

private:
    struct RuleEntry {
        RuleId id;
        RuleDef def;
    };

    std::vector<RuleEntry> rules_;
    RuleId next_id_{1};
};

} // namespace gameak::runtime
