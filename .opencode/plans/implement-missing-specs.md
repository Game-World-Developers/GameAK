# Plano: Implementar 6 Specs Faltantes

## Visão Geral

Implementar SPEC-018 a SPEC-023 (6 specs com veredito READY).

## Ordem de Implementação

1. SPEC-019 (standalone Core) → 2. SPEC-018 (modifica Runtime) → 3. SPEC-020 (FSM Controller) → 4. SPEC-021 (Rule Systems) → 5. SPEC-022 (Pipelines) → 6. SPEC-023 (Event Loops)

---

## SPEC-019: Bit Representation

### Arquivos a criar

#### `Include/GameAk/Core/bitset.h`
```cpp
#pragma once
#include <array>
#include <cstdint>
#include <cstddef>
#include <initializer_list>

namespace gameak::core {

template <size_t N>
class BitSet {
    static constexpr size_t kWords = (N + 63) / 64;
    std::array<uint64_t, kWords> words_{};

    static constexpr size_t word_index(size_t pos) { return pos / 64; }
    static constexpr size_t bit_index(size_t pos)  { return pos % 64; }

public:
    BitSet() = default;
    BitSet(std::initializer_list<size_t> bits);

    BitSet& set(size_t pos);
    BitSet& reset(size_t pos);
    BitSet& flip(size_t pos);
    bool test(size_t pos) const;
    size_t count() const;
    bool any() const;
    bool none() const;
    static constexpr size_t size() { return N; }

    bool operator==(const BitSet&) const;
    BitSet operator~() const;
    BitSet operator&(const BitSet&) const;
    BitSet operator|(const BitSet&) const;
    BitSet operator^(const BitSet&) const;
    BitSet& operator&=(const BitSet&);
    BitSet& operator|=(const BitSet&);
    BitSet& operator^=(const BitSet&);
};

}
```

#### `Include/GameAk/Core/bitvector.h`
```cpp
#pragma once
#include "flat_vector.h"
#include <cstdint>

namespace gameak::core {

class BitVector {
    flat_vector<uint64_t, 2> words_;
    size_t bit_count_{0};

public:
    BitVector() = default;
    explicit BitVector(size_t initial_bits);

    void push_back(bool value);
    void set(size_t pos);
    void reset(size_t pos);
    void flip(size_t pos);
    bool test(size_t pos) const;
    bool operator[](size_t pos) const;

    size_t size() const;
    bool empty() const;
    void resize(size_t new_bits);
    void clear();
    size_t count() const;
    bool any() const;
    bool none() const;
};

}
```

#### `Include/GameAk/Core/bitflags.h`
```cpp
#pragma once
#include <cstdint>
#include <type_traits>
#include <unordered_map>
#include <string>

namespace gameak::core {

template <typename Enum>
class BitFlags {
    static_assert(std::is_enum_v<Enum>);
    uint64_t bits_{0};
public:
    BitFlags() = default;
    explicit BitFlags(uint64_t bits);

    BitFlags& set(Enum flag);
    BitFlags& reset(Enum flag);
    BitFlags& toggle(Enum flag);
    bool is_set(Enum flag) const;
    bool any() const;
    bool none() const;
    uint64_t value() const;

    bool operator==(const BitFlags&) const;
    BitFlags operator|(const BitFlags&) const;
    BitFlags operator&(const BitFlags&) const;
    BitFlags operator^(const BitFlags&) const;
};

class DynamicBitFlags {
    uint64_t bits_{0};
    std::unordered_map<std::string, uint8_t> name_to_bit_;
public:
    DynamicBitFlags() = default;
    uint8_t register_flag(const std::string& name);
    void set(const std::string& name);
    void reset(const std::string& name);
    bool is_set(const std::string& name) const;
    uint64_t value() const;
    bool any() const;
    bool none() const;
};

}
```

#### `Include/GameAk/Core/bit_packing.h`
```cpp
#pragma once
#include <cstdint>

namespace gameak::core::bit_packing {

template <size_t Offset, size_t Width>
struct Field {
    static constexpr size_t offset = Offset;
    static constexpr size_t width  = Width;
    static constexpr uint64_t mask = ((uint64_t{1} << Width) - 1);

    template <typename T>
    static constexpr uint64_t pack(uint64_t word, T value);
    template <typename T = uint64_t>
    static constexpr T unpack(uint64_t word);
};

template <typename... Fields>
class PackedWord {
    uint64_t word_{0};
public:
    PackedWord() = default;
    explicit PackedWord(uint64_t word);

    template <typename Field, typename T>
    PackedWord& set(T value);
    template <typename Field, typename T = uint64_t>
    T get() const;

    uint64_t value() const;
    void set_value(uint64_t v);
    bool operator==(const PackedWord&) const;
};

// Free functions
template <typename Field>
static constexpr uint64_t pack_value(uint64_t word, uint64_t value);
template <typename Field>
static constexpr uint64_t unpack_value(uint64_t word);

}
```

#### `Tests/test_bit_representation.h`
Testes:
- `bitset_set_and_test`: set bits 3,7,31 → test() returns true
- `bitset_bitwise_ops`: A=0b1100, B=0b1010 → AND/OR/XOR
- `bitvector_dynamic_size`: append 100 bits, accessible by index
- `bitflags_named_access`: set Enabled/Locked, test Visible
- `bit_packing_roundtrip`: pack x:3, y:5, z:8 (16 bits) → unpack yields same values
- `bit_representation_copy`: copy BitSet → equal and independent
- `bitset_compile_time_size`: BitSet<64>::size() == 64
- `bitvector_dynamic_growth`: grow beyond initial capacity
- `bitflags_fixed_backing`: backed by 64-bit integer
- `bit_ops_deterministic`: same ops produce same results

---

## SPEC-018: Data Layout

### Arquivos a criar/modificar

#### `Include/GameAk/Runtime/layout_strategy.h` (NOVO)
```cpp
#pragma once
#include <cstdint>

namespace gameak::runtime {

enum class LayoutStrategy : uint32_t {
    AoS,
    SoA,
    AoSoA,
};

struct AoSoAConfig {
    uint32_t chunk_size{8};
};

}
```

#### `Include/GameAk/Runtime/block_type.h` (MODIFICAR)
Adicionar campos:
```cpp
#include "layout_strategy.h"

struct BlockTypeDescriptor {
    uint32_t type_id;
    size_t size;
    size_t alignment;
    const char* name;
    LayoutStrategy layout{LayoutStrategy::AoS};
    AoSoAConfig aosoa_config{};
};
```

#### `Include/GameAk/Runtime/command.h` (MODIFICAR)
Adicionar:
```cpp
enum class CommandType : uint32_t {
    CreateBlock,
    DestroyBlock,
    SetField,
    ResizeBlock,
    ConvertLayout,
};

struct CommandConvertLayout {
    uint32_t type_id;
    LayoutStrategy new_layout;
    AoSoAConfig aosoa_config{};
};

using CommandPayload = std::variant<..., CommandConvertLayout>;
```

#### `Include/GameAk/Core/error.h` (MODIFICAR)
Adicionar:
```cpp
LayoutMismatch,
```

#### `Include/GameAk/Runtime/scheduler_ops.h` (MODIFICAR)
Adicionar validação + execução para `ConvertLayout`.

#### `Include/GameAk/Runtime/runtime.h` (MODIFICAR)
Adicionar `per-type layout storage` — para SoA, armazenar fields em arrays contíguos separados. Para AoSoA, chunks de SoA.

Estratégia: 
- Manter blocks_ como `unordered_map<Identity, DataBlock>` para AoS (comportamento atual)
- Adicionar `std::unordered_map<uint32_t, TypeLayoutStorage> layout_storage_` para SoA/AoSoA
- `TypeLayoutStorage` contém: layout strategy + identities vector + per-field arrays

#### `Tests/test_data_layout.h` (NOVO)
Testes:
- `default_layout_is_aos`
- `layout_is_type_property`
- `aos_field_ordering`
- `soa_field_separation`
- `aosoa_chunk_layout`
- `layout_transparent_to_controllers`
- `layout_conversion_preserves_identity`
- `layout_conversion_preserves_values`
- `multiple_layouts_coexist`

---

## SPEC-020: Finite State Machines

### `Include/GameAk/Runtime/fsm.h` (NOVO)
```cpp
#pragma once
#include "GameAk/Runtime/controller.h"
#include "GameAk/Runtime/command.h"
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

namespace gameak::runtime {

using FsmState = std::string;
using FsmEvent = std::string;

struct FsmTransition {
    FsmState from;
    FsmEvent event;
    FsmState to;
};

using FsmAction = std::function<void(CommandProducer&)>;

class Fsm {
public:
    explicit Fsm(FsmState initial_state);

    void add_state(FsmState state);
    void add_transition(FsmState from, FsmEvent event, FsmState to);
    void on_entry(FsmState state, FsmAction action);
    void on_exit(FsmState state, FsmAction action);

    Controller build();

private:
    FsmState initial_state_;
    std::unordered_set<FsmState> states_;
    // from+event → to
    std::unordered_map<FsmState, std::unordered_map<FsmEvent, FsmState>> transitions_;
    std::unordered_map<FsmState, FsmAction> entry_actions_;
    std::unordered_map<FsmState, FsmAction> exit_actions_;
};

/// Command to submit an event to an FSM
struct CommandSubmitFsmEvent {
    core::Identity fsm_identity;
    FsmEvent event;
};

}
```

### `Src/GameAk/Runtime/fsm.cpp` (NOVO)
Implementação do builder + Controller callable.

### `Tests/test_fsm.h`
Testes:
- `fsm_starts_in_initial_state`
- `fsm_transitions_on_event`
- `fsm_produces_commands_on_transition`
- `fsm_ignores_undefined_transitions`
- `fsm_entry_actions`
- `fsm_exit_actions`
- `fsm_deterministic_execution`
- `fsm_is_controller`
- `fsm_no_mutable_state`
- `fsm_explicit_transitions_only`
- `fsm_single_active_state`
- `fsm_event_channel`

---

## SPEC-021: Rule Systems

### `Include/GameAk/Runtime/rule_system.h` (NOVO)
```cpp
#pragma once
#include "GameAk/Runtime/controller.h"
#include <functional>
#include <vector>

namespace gameak::runtime {

using RuleId = uint64_t;
using RuleCondition = std::function<bool(StateView&)>;
using RuleAction = std::function<void(StateView&, CommandProducer&)>;

struct Rule {
    std::string name;
    RuleCondition condition;
    RuleAction action;
    int priority{0};
};

class RuleSystem {
public:
    RuleId add_rule(std::string name, RuleCondition condition, RuleAction action, int priority = 0);
    void remove_rule(RuleId id);
    void clear_rules();
    Controller build();

private:
    std::vector<std::pair<RuleId, Rule>> rules_;
    RuleId next_id_{1};
};

}
```

### `Src/GameAk/Runtime/rule_system.cpp` (NOVO)
Implementação.

### `Tests/test_rule_system.h`
Testes:
- `matching_condition_fires`
- `non_matching_condition_does_not_fire`
- `multiple_rules_fire`
- `rule_priority_order`
- `rule_action_produces_command`
- `empty_rule_system`
- `rule_system_is_controller`
- `deterministic_evaluation`
- `no_mutable_state`
- `unique_priorities`
- `rules_produce_commands`

---

## SPEC-022: Runtime Pipelines

### `Include/GameAk/Runtime/pipeline.h`
```cpp
#pragma once
#include "GameAk/Runtime/controller.h"
#include <string>
#include <vector>

namespace gameak::runtime {

using StageId = uint64_t;

struct StageResult {
    StageId id;
    std::string name;
    bool success;
    core::Error error;
};

class Pipeline {
public:
    StageId add_stage(std::string name, Controller controller);
    void remove_stage(StageId id);
    void clear_stages();
    Controller build();

    // Diagnostics
    std::vector<StageResult> last_results() const;

private:
    struct Stage {
        StageId id;
        std::string name;
        Controller controller;
    };
    std::vector<Stage> stages_;
    StageId next_id_{1};
    std::vector<StageResult> last_results_;
};

}
```

### `Src/GameAk/Runtime/pipeline.cpp`

### `Tests/test_pipeline.h`
Testes:
- `stages_execute_in_order`
- `stage_output_feeds_next`
- `empty_pipeline`
- `stage_is_controller`
- `pipeline_stage_diagnostics`
- `pipeline_is_controller`
- `deterministic_stage_order`
- `stage_failure_does_not_skip`

---

## SPEC-023: Event Loops

### `Include/GameAk/Runtime/event_loop.h`
```cpp
#pragma once
#include "GameAk/Runtime/controller.h"
#include <functional>
#include <queue>
#include <unordered_map>
#include <variant>

namespace gameak::runtime {

using EventTypeId = uint32_t;
using HandlerId = uint64_t;

template <typename EventData>
using EventHandler = std::function<void(const EventData&, CommandProducer&)>;

template <typename EventData>
class EventLoop {
public:
    using Handler = EventHandler<EventData>;

    HandlerId on(EventTypeId type, Handler handler);
    void off(HandlerId id);
    void enqueue(EventTypeId type, EventData data);
    Controller build();

private:
    struct PendingEvent {
        EventTypeId type;
        EventData data;
    };

    std::unordered_map<EventTypeId, std::vector<std::pair<HandlerId, Handler>>> handlers_;
    std::queue<PendingEvent> queue_;
    HandlerId next_id_{1};
};

}
```

### `Src/GameAk/Runtime/event_loop.cpp`
Implementação.

### `Tests/test_event_loop.h`
Testes:
- `event_dispatches_to_handler`
- `event_with_no_handlers`
- `multiple_handlers_for_event`
- `fifo_event_processing`
- `handler_receives_event_data`
- `event_loop_is_controller`
- `controller_enqueues_event`
- `handlers_produce_commands`
- `no_state_between_ticks`

---

## Modificações Transversais

### `Makefile`
Adicionar ao `SRC_RUNTIME`:
```
Src/GameAk/Runtime/fsm.cpp \
Src/GameAk/Runtime/rule_system.cpp \
Src/GameAk/Runtime/pipeline.cpp \
Src/GameAk/Runtime/event_loop.cpp
```

### `Tests/test_runtime.cpp`
Adicionar includes:
```cpp
#include "test_bit_representation.h"
#include "test_data_layout.h"
#include "test_fsm.h"
#include "test_rule_system.h"
#include "test_pipeline.h"
#include "test_event_loop.h"
```

Adicionar chamadas:
```cpp
run_bit_representation_tests();
run_data_layout_tests();
run_fsm_tests();
run_rule_system_tests();
run_pipeline_tests();
run_event_loop_tests();
```

### `Include/GameAk/Core/error.h`
Adicionar `LayoutMismatch` ao enum `ErrorCode`.

---

## Verificação

```bash
make clean && make && make test
```

Todos os testes existentes + novos testes devem passar.
