#pragma once

#include <AK/Backend/ExecContext.hpp>
#include <AK/Core/Macros.hpp>
#include <AK/Core/Types.hpp>

#include <cstring>

namespace GameAK::FSM {

static constexpr usize kMaxStates = 64;
static constexpr usize kMaxTransitionsPerState = 16;
static constexpr usize kMaxDepth = 64;

template <typename StateEnum, typename EventEnum = u32>
class StateMachine {
public:
  using Action = void (*)(ExecContext &);
  using Guard = bool (*)(ExecContext &);

  struct StateConfig {
    StateEnum id;
    StateEnum parent;
    StateEnum initial_child;
  };

  explicit StateMachine(StateEnum root_id) noexcept
      : m_root(root_id), m_leaf(root_id), m_root_id(root_id), m_state_count(1) {
    __builtin_memset(m_states, 0, sizeof(m_states));
    m_states[0].id = root_id;
    m_states[0].parent = root_id;
    m_states[0].initial_child = root_id;
  }

  void add_state(StateConfig cfg) noexcept {
    if (GAMEAK_UNLIKELY(m_state_count >= kMaxStates))
      return;
    for (usize i = 0; i < m_state_count; ++i) {
      if (m_states[i].id == cfg.id)
        return;
    }
    m_states[m_state_count].id = cfg.id;
    m_states[m_state_count].parent = cfg.parent;
    m_states[m_state_count].initial_child = cfg.initial_child;
    ++m_state_count;
  }

  void add_transition(StateEnum from, EventEnum event, StateEnum to,
                      Guard guard = nullptr) noexcept {
    StateNode *node = find_node(from);
    if (GAMEAK_UNLIKELY(!node))
      return;
    if (GAMEAK_UNLIKELY(node->transition_count >= kMaxTransitionsPerState))
      return;
    auto &t = node->transitions[node->transition_count];
    t.event = event;
    t.to = to;
    t.guard = guard;
    ++node->transition_count;
  }

  void set_on_enter(StateEnum id, Action fn) noexcept {
    StateNode *node = find_node(id);
    if (GAMEAK_LIKELY(node))
      node->on_enter_fn = fn;
  }

  void set_on_exit(StateEnum id, Action fn) noexcept {
    StateNode *node = find_node(id);
    if (GAMEAK_LIKELY(node))
      node->on_exit_fn = fn;
  }

  void set_on_update(StateEnum id, Action fn) noexcept {
    StateNode *node = find_node(id);
    if (GAMEAK_LIKELY(node))
      node->on_update_fn = fn;
  }

  bool handle_event(EventEnum event, ExecContext &ctx) noexcept {
    StateEnum cur = m_leaf;
    while (true) {
      StateNode *node = find_node(cur);
      if (!node)
        return false;
      for (u8 i = 0; i < node->transition_count; ++i) {
        auto &t = node->transitions[i];
        if (t.event != event)
          continue;
        if (t.guard && !t.guard(ctx))
          continue;
        transition_to(t.to, ctx);
        return true;
      }
      if (cur == m_root_id)
        break;
      cur = node->parent;
    }
    return false;
  }

  void update(ExecContext &ctx) noexcept {
    for (usize i = 0; i < m_state_count; ++i) {
      if (is_active(m_states[i].id) && m_states[i].on_update_fn)
        m_states[i].on_update_fn(ctx);
    }
  }

  void transition_to(StateEnum target, ExecContext &ctx) noexcept {
    if (target == m_leaf)
      return;

    StateEnum current_path[kMaxDepth];
    StateEnum target_path[kMaxDepth];
    usize current_len = build_path(m_leaf, current_path);
    usize target_len = build_path(target, target_path);
    if (current_len == 0 || target_len == 0)
      return;

    usize lca_idx = 0;
    while (lca_idx < current_len && lca_idx < target_len &&
           current_path[lca_idx] == target_path[lca_idx]) {
      ++lca_idx;
    }
    --lca_idx;

    for (usize i = current_len; i > lca_idx + 1; --i) {
      StateNode *node = find_node(current_path[i - 1]);
      if (node && node->on_exit_fn)
        node->on_exit_fn(ctx);
    }

    for (usize i = lca_idx + 1; i < target_len; ++i) {
      StateNode *node = find_node(target_path[i]);
      if (node && node->on_enter_fn)
        node->on_enter_fn(ctx);
    }

    m_leaf = resolve_leaf(target);
  }

  StateEnum current() const noexcept { return m_leaf; }

  bool is_active(StateEnum id) const noexcept {
    StateEnum cur = m_leaf;
    while (true) {
      if (cur == id)
        return true;
      if (cur == m_root_id)
        return false;
      const StateNode *node = find_node(cur);
      if (!node)
        return false;
      cur = node->parent;
    }
  }

private:
  struct TransitionData {
    EventEnum event;
    StateEnum to;
    Guard guard;
  };

  struct StateNode {
    StateEnum id;
    StateEnum parent;
    StateEnum initial_child;
    Action on_enter_fn;
    Action on_exit_fn;
    Action on_update_fn;
    TransitionData transitions[kMaxTransitionsPerState];
    u8 transition_count;
  };

  StateNode *find_node(StateEnum id) noexcept {
    for (usize i = 0; i < m_state_count; ++i) {
      if (m_states[i].id == id)
        return &m_states[i];
    }
    return nullptr;
  }

  const StateNode *find_node(StateEnum id) const noexcept {
    for (usize i = 0; i < m_state_count; ++i) {
      if (m_states[i].id == id)
        return &m_states[i];
    }
    return nullptr;
  }

  usize build_path(StateEnum id, StateEnum *out) const noexcept {
    StateEnum chain[kMaxDepth];
    usize clen = 0;
    StateEnum cur = id;
    while (true) {
      chain[clen++] = cur;
      if (cur == m_root_id)
        break;
      const StateNode *node = find_node(cur);
      if (!node || node->parent == cur)
        break;
      cur = node->parent;
    }
    for (usize i = 0; i < clen; ++i)
      out[i] = chain[clen - 1 - i];
    return clen;
  }

  StateEnum resolve_leaf(StateEnum id) const noexcept {
    StateEnum cur = id;
    usize guard = 0;
    while (guard++ < kMaxDepth) {
      const StateNode *node = find_node(cur);
      if (!node || node->initial_child == cur)
        return cur;
      cur = node->initial_child;
    }
    return cur;
  }

  StateNode m_states[kMaxStates];
  StateEnum m_root;
  StateEnum m_leaf;
  StateEnum m_root_id;
  usize m_state_count;
};

} // namespace GameAK::FSM
