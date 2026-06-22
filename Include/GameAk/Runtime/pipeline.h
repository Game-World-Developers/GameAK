#pragma once

#include "controller.h"
#include "GameAk/Core/flat_vector.h"
#include "GameAk/Core/result.h"

#include <memory>
#include <string>

// Pipeline uses shared_ptr for results propagation

namespace gameak::runtime {

using StageId = uint64_t;

struct StageResult {
    StageId id;
    std::string name;
    bool success{false};
    core::Error error;
};

class Pipeline {
public:
    StageId add_stage(std::string name, Controller controller);

    // ── Conversation-style chaining ─────────────────────────────────
    Pipeline& stage(std::string name, Controller controller) {
        add_stage(std::move(name), std::move(controller));
        return *this;
    }

    void remove_stage(StageId id);

    void clear_stages();

    size_t stage_count() const;

    Controller build();

    core::flat_vector<StageResult, 8> last_results() const;

private:
    struct Stage {
        StageId id;
        std::string name;
        Controller controller;
    };

    std::shared_ptr<core::flat_vector<Stage, 8>> stages_{std::make_shared<core::flat_vector<Stage, 8>>()};
    StageId next_id_{1};
    std::shared_ptr<core::flat_vector<StageResult, 8>> results_ptr_{std::make_shared<core::flat_vector<StageResult, 8>>()};
};

} // namespace gameak::runtime
