#include "GameAk/Runtime/pipeline.h"
#include "GameAk/Runtime/data_block.h"

#include <algorithm>

namespace gameak::runtime {

StageId Pipeline::add_stage(std::string name, Controller controller) {
    StageId id = next_id_++;
    stages_.push_back({id, std::move(name), std::move(controller)});
    return id;
}

void Pipeline::remove_stage(StageId id) {
    auto it = std::remove_if(stages_.begin(), stages_.end(),
        [id](const Stage& s) { return s.id == id; });
    stages_.erase(it, stages_.end());
}

void Pipeline::clear_stages() {
    stages_.clear();
}

size_t Pipeline::stage_count() const {
    return stages_.size();
}

std::vector<StageResult> Pipeline::last_results() const {
    return *results_ptr_;
}

Controller Pipeline::build() {
    auto stages = std::make_shared<std::vector<Stage>>(stages_);
    auto results = results_ptr_;

    return [stages, results](StateView& view, CommandProducer& producer, EphemeralProducer& ephem) -> core::Result<void> {
        results->clear();

        for (auto& stage : *stages) {
            auto result = stage.controller(view, producer, ephem);
            StageResult sr;
            sr.id = stage.id;
            sr.name = stage.name;
            sr.success = result.has_value();
            if (!result) {
                sr.error = result.error();
            }
            results->push_back(std::move(sr));
        }

        return {};
    };
}

} // namespace gameak::runtime
