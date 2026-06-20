#pragma once

inline void run_dsl_config_tests() {
    using namespace gameak::runtime;

describe("DSL - Config Builder", {
    it("config_builder_log_level", {
        auto builder = Runtime<>::configure()
            .log_level(LogLevel::Info);
        auto rt = std::move(builder).build();
        expect(rt.config().log_level == LogLevel::Info).toBeTruthy();
    });

    it("config_builder_fixed_timestep", {
        auto builder = Runtime<>::configure()
            .fixed_timestep(1.0f / 60.0f);
        auto rt = std::move(builder).build();
        expect(rt.fixed_timestep() == 1.0f / 60.0f).toBeTruthy();
    });

    it("config_builder_defaults", {
        auto builder = Runtime<>::configure();
        auto rt = std::move(builder).build();
        expect(rt.config().log_level == LogLevel::Warn).toBeTruthy();
        expect(rt.fixed_timestep() == 0.0f).toBeTruthy();
    });
});
}
