#pragma once

#include "GameAk/Core/identity.h"
#include "GameAk/Core/result.h"

#include <cstdint>
#include <unordered_map>
#include <vector>

namespace gameak::runtime {
class DataBlock;
}

namespace gameak::runtime {

class RelationshipManager {
public:
    RelationshipManager() = default;

    core::Result<void> relate(core::Identity parent, core::Identity child,
                              const std::unordered_map<core::Identity, DataBlock>* blocks = nullptr);
    core::Result<void> unrelate(core::Identity parent, core::Identity child);

    std::vector<core::Identity> children_of(core::Identity parent) const;
    std::vector<core::Identity> parents_of(core::Identity child) const;

    // Exposed for save/load
    const std::unordered_multimap<core::Identity, core::Identity>& parent_to_children() const {
        return parent_to_children_;
    }
    std::unordered_multimap<core::Identity, core::Identity>& mut_parent_to_children() {
        return parent_to_children_;
    }

private:
    std::unordered_multimap<core::Identity, core::Identity> parent_to_children_;
    std::unordered_multimap<core::Identity, core::Identity> child_to_parents_;
};

// ── Inline implementation ──────────────────────────────────────────

inline core::Result<void> RelationshipManager::relate(
    core::Identity parent, core::Identity child,
    const std::unordered_map<core::Identity, DataBlock>* blocks)
{
    if (!parent.is_valid() || !child.is_valid()) {
        return core::Error{core::ErrorCode::InvalidIdentity, "Parent or child identity is invalid"};
    }
    if (blocks) {
        if (!blocks->contains(parent)) {
            return core::Error{core::ErrorCode::BlockNotFound, "Parent block not found"};
        }
        if (!blocks->contains(child)) {
            return core::Error{core::ErrorCode::BlockNotFound, "Child block not found"};
        }
    }
    if (parent == child) {
        return core::Error{core::ErrorCode::InvalidOperation, "Block cannot be related to itself"};
    }
    parent_to_children_.emplace(parent, child);
    child_to_parents_.emplace(child, parent);
    return {};
}

inline core::Result<void> RelationshipManager::unrelate(core::Identity parent, core::Identity child) {
    if (!parent.is_valid() || !child.is_valid()) {
        return core::Error{core::ErrorCode::InvalidIdentity, "Parent or child identity is invalid"};
    }
    auto range = parent_to_children_.equal_range(parent);
    for (auto it = range.first; it != range.second; ++it) {
        if (it->second == child) {
            parent_to_children_.erase(it);
            break;
        }
    }
    auto crange = child_to_parents_.equal_range(child);
    for (auto it = crange.first; it != crange.second; ++it) {
        if (it->second == parent) {
            child_to_parents_.erase(it);
            break;
        }
    }
    return {};
}

inline std::vector<core::Identity> RelationshipManager::children_of(core::Identity parent) const {
    std::vector<core::Identity> result;
    auto range = parent_to_children_.equal_range(parent);
    for (auto it = range.first; it != range.second; ++it) {
        result.push_back(it->second);
    }
    return result;
}

inline std::vector<core::Identity> RelationshipManager::parents_of(core::Identity child) const {
    std::vector<core::Identity> result;
    auto range = child_to_parents_.equal_range(child);
    for (auto it = range.first; it != range.second; ++it) {
        result.push_back(it->second);
    }
    return result;
}

} // namespace gameak::runtime
