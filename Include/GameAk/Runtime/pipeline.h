#pragma once

#include "controller.h"
#include "GameAk/Core/result.h"

#include <memory>
#include <string>
#include <vector>

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

    void remove_stage(StageId id);

    void clear_stages();

    size_t stage_count() const;

    Controller build();

    std::vector<StageResult> last_results() const;

private:
    struct Stage {
        StageId id;
        std::string name;
        Controller controller;
    };

    std::vector<Stage> stages_;
    StageId next_id_{1};
    std::shared_ptr<std::vector<StageResult>> results_ptr_{std::make_shared<std::vector<StageResult>>()};
};

} // namespace gameak::runtime
