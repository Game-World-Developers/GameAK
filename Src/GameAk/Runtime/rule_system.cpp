#include "GameAk/Runtime/rule_system.h"
#include "GameAk/Runtime/data_block.h"

#include <algorithm>

namespace gameak::runtime {

RuleId RuleSystem::add_rule(RuleDef rule) {
    RuleId id = next_id_++;
    rules_.push_back({id, std::move(rule)});
    return id;
}

RuleId RuleSystem::add_rule(std::string name, RuleCondition condition, RuleAction action, int priority) {
    RuleDef def;
    def.name = std::move(name);
    def.condition = std::move(condition);
    def.action = std::move(action);
    def.priority = priority;
    return add_rule(std::move(def));
}

void RuleSystem::remove_rule(RuleId id) {
    auto it = std::remove_if(rules_.begin(), rules_.end(),
        [id](const RuleEntry& e) { return e.id == id; });
    rules_.erase(it, rules_.end());
}

void RuleSystem::clear_rules() {
    rules_.clear();
}

size_t RuleSystem::rule_count() const {
    return rules_.size();
}

Controller RuleSystem::build() {
    auto rules = std::make_shared<core::flat_vector<RuleEntry, 8>>(rules_);

    return [rules](StateView& view, CommandProducer& producer, EphemeralProducer& ephem) -> core::Result<void> {
        // Sort by priority descending (stable sort preserves registration order for equal priorities)
        std::stable_sort(rules->begin(), rules->end(),
            [](const RuleEntry& a, const RuleEntry& b) {
                return a.def.priority > b.def.priority;
            });

        for (auto& entry : *rules) {
            if (entry.def.condition(view)) {
                entry.def.action(view, producer, ephem);
            }
        }

        return {};
    };
}

} // namespace gameak::runtime
