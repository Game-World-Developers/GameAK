#pragma once

#include "GameAk/Core/flat_vector.h"
#include "GameAk/Core/identity.h"
#include "GameAk/Core/rb_tree.h"
#include "GameAk/Core/result.h"

#include <cstdint>

namespace gameak::runtime {
class DataBlock;
}

namespace gameak::runtime {

class RelationshipManager {
public:
    RelationshipManager() = default;

    core::Result<void> relate(core::Identity parent, core::Identity child,
                              const core::rb_tree<core::Identity, DataBlock>* blocks = nullptr);
    core::Result<void> unrelate(core::Identity parent, core::Identity child);

    core::flat_vector<core::Identity> children_of(core::Identity parent) const;
    core::flat_vector<core::Identity> parents_of(core::Identity child) const;

    // Exposed for save/load
    using RelMap = core::rb_tree<core::Identity, core::flat_vector<core::Identity>>;
    const RelMap& parent_to_children() const { return parent_to_children_; }
    RelMap& mut_parent_to_children() { return parent_to_children_; }

private:
    RelMap parent_to_children_;
    RelMap child_to_parents_;
};

// ── Inline implementation ──────────────────────────────────────────

inline core::Result<void> RelationshipManager::relate(
    core::Identity parent, core::Identity child,
    const core::rb_tree<core::Identity, DataBlock>* blocks)
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
    {
        auto it = parent_to_children_.find(parent);
        if (it != parent_to_children_.end()) {
            it->second.push_back(child);
        } else {
            parent_to_children_.insert(parent, {child});
        }
    }
    {
        auto it = child_to_parents_.find(child);
        if (it != child_to_parents_.end()) {
            it->second.push_back(parent);
        } else {
            child_to_parents_.insert(child, {parent});
        }
    }
    return {};
}

inline core::Result<void> RelationshipManager::unrelate(core::Identity parent, core::Identity child) {
    if (!parent.is_valid() || !child.is_valid()) {
        return core::Error{core::ErrorCode::InvalidIdentity, "Parent or child identity is invalid"};
    }
    auto pit = parent_to_children_.find(parent);
    if (pit != parent_to_children_.end()) {
        auto& children = pit->second;
        for (size_t i = 0; i < children.size(); ++i) {
            if (children[i] == child) {
                children.erase(children.begin() + i);
                if (children.empty()) parent_to_children_.erase(parent);
                break;
            }
        }
    }
    auto cit = child_to_parents_.find(child);
    if (cit != child_to_parents_.end()) {
        auto& parents = cit->second;
        for (size_t i = 0; i < parents.size(); ++i) {
            if (parents[i] == parent) {
                parents.erase(parents.begin() + i);
                if (parents.empty()) child_to_parents_.erase(child);
                break;
            }
        }
    }
    return {};
}

inline core::flat_vector<core::Identity> RelationshipManager::children_of(core::Identity parent) const {
    auto it = parent_to_children_.find(parent);
    if (it != parent_to_children_.end()) {
        return it->second;
    }
    return {};
}

inline core::flat_vector<core::Identity> RelationshipManager::parents_of(core::Identity child) const {
    auto it = child_to_parents_.find(child);
    if (it != child_to_parents_.end()) {
        return it->second;
    }
    return {};
}

} // namespace gameak::runtime
