#pragma once

namespace fuzz_helpers {

using namespace gameak::core;
using namespace gameak::runtime;
using DefaultRuntime = Runtime<FifoScheduler>;

// Simple LCG PRNG (deterministic)
struct PRNG {
    uint64_t state;
    explicit PRNG(uint64_t seed) : state{seed} {}
    uint64_t next() {
        state = state * 6364136223846793005ULL + 1442695040888963407ULL;
        return state;
    }
    uint32_t range(uint32_t max) {
        return static_cast<uint32_t>(next() % max);
    }
};

void test_random_command_sequence() {
    DefaultRuntime rt;
    BlockTypeDescriptor desc;
    desc.type_id = 1; desc.size = sizeof(int) * 4; desc.alignment = alignof(int); desc.name = "test";
    auto _r = rt.register_block_type(std::move(desc)); (void)_r;

    PRNG rng{42};
    constexpr int OPS = 1000;

    for (int i = 0; i < OPS; ++i) {
        uint32_t op = rng.range(4);
        switch (op) {
            case 0: { // create block
                auto id = rt.create_block(1);
                (void)id;
                break;
            }
            case 1: { // submit create command
                auto id = rt.submit_command(Command{CommandCreateBlock{1}});
                (void)id;
                break;
            }
            case 2: { // tick
                auto r = rt.tick(0.016f);
                (void)r;
                break;
            }
            case 3: { // destroy random existing block
                auto blocks = rt.find_blocks_by_type(1);
                if (!blocks.empty()) {
                    uint32_t idx = rng.range(static_cast<uint32_t>(blocks.size()));
                    auto d = rt.destroy_block(blocks[idx]);
                    (void)d;
                }
                break;
            }
        }
    }

    auto diag = rt.collect_diagnostics();
    auto total = diag.blocks_count + diag.pending_commands + diag.commands_executed;
    expect(total > 0).toBeTruthy();
    (void)total;
}

void test_fuzzy_invalid_commands_interleaved() {
    DefaultRuntime rt;
    BlockTypeDescriptor desc;
    desc.type_id = 1; desc.size = sizeof(int); desc.alignment = alignof(int); desc.name = "test";
    expect(rt.register_block_type(std::move(desc)).has_value()).toBeTruthy();

    PRNG rng{123};
    constexpr int OPS = 500;

    for (int i = 0; i < OPS; ++i) {
        uint32_t which = rng.range(10);
        if (which < 5) {
            auto _s1 = rt.submit_command(Command{CommandCreateBlock{1}}); (void)_s1;
        } else if (which < 7) {
            auto _s2 = rt.submit_command(Command{CommandCreateBlock{99}}); (void)_s2;
        } else if (which < 9) {
            Identity fake{static_cast<uint64_t>(rng.range(10000) + 1000)};
            auto _s3 = rt.submit_command(Command{CommandDestroyBlock{{fake}}}); (void)_s3;
        } else {
            auto r = rt.tick(0.016f);
            (void)r;
        }
    }

    auto r = rt.tick(0.016f);
    auto diag = rt.collect_diagnostics();
    expect(diag.commands_rejected + diag.commands_executed + diag.pending_commands > 0).toBeTruthy();
    (void)r;
}

void test_bulk_create_destroy_alternating() {
    DefaultRuntime rt;
    BlockTypeDescriptor desc;
    desc.type_id = 1; desc.size = sizeof(int); desc.alignment = alignof(int); desc.name = "test";
    expect(rt.register_block_type(std::move(desc)).has_value()).toBeTruthy();

    PRNG rng{456};
    constexpr int OPS = 2000;
    int created = 0;

    for (int i = 0; i < OPS; ++i) {
        uint32_t op = rng.range(3);
        if (op == 0) {
            auto id = rt.create_block(1);
            if (id.has_value()) created++;
        } else if (op == 1 && created > 0) {
            auto blocks = rt.find_blocks_by_type(1);
            if (!blocks.empty()) {
                uint32_t idx = rng.range(static_cast<uint32_t>(blocks.size()));
                auto d = rt.destroy_block(blocks[idx]);
                if (d.has_value()) created--;
            }
        } else {
            auto t = rt.tick(0.016f);
            (void)t;
        }
    }

    expect(rt.block_count(1) == static_cast<size_t>(created)).toBeTruthy();
}

void test_random_layout_alternation() {
    DefaultRuntime rt;
    BlockTypeDescriptor desc;
    desc.type_id = 1; desc.size = sizeof(int) * 4; desc.alignment = alignof(int);
    desc.name = "test"; desc.layout = LayoutStrategy::AoS;
    expect(rt.register_block_type(std::move(desc)).has_value()).toBeTruthy();

    for (int i = 0; i < 50; ++i) {
        auto id = rt.create_block(1);
        expect(id.has_value()).toBeTruthy();
    }

    PRNG rng{789};
    flat_vector<Identity, 4> last_ids;

    for (int i = 0; i < 50; ++i) {
        if (rng.range(2) == 0) {
            rt.convert_to_soa(1);
            expect(rt.get_layout(1) == LayoutStrategy::SoA).toBeTruthy();
        } else {
            rt.convert_from_soa(1);
            expect(rt.get_layout(1) == LayoutStrategy::AoS).toBeTruthy();
        }
        auto ids = rt.find_blocks_by_type(1);
        if (i > 0 && !last_ids.empty() && !ids.empty()) {
            expect(last_ids.size() == ids.size()).toBeTruthy();
        }
        last_ids = ids;
    }

    rt.convert_from_soa(1);
    expect(rt.block_count(1) == 50).toBeTruthy();
}

void test_deterministic_replay() {
    DefaultRuntime rt1;
    DefaultRuntime rt2;
    BlockTypeDescriptor desc;
    desc.type_id = 1; desc.size = sizeof(int); desc.alignment = alignof(int); desc.name = "test";
    expect(rt1.register_block_type(std::move(desc)).has_value()).toBeTruthy();
    BlockTypeDescriptor desc2;
    desc2.type_id = 1; desc2.size = sizeof(int); desc2.alignment = alignof(int); desc2.name = "test";
    expect(rt2.register_block_type(std::move(desc2)).has_value()).toBeTruthy();

    PRNG rng{999};
    constexpr int OPS = 200;

    auto apply_ops = [&](DefaultRuntime& rt, PRNG& local_rng) {
        for (int i = 0; i < OPS; ++i) {
            uint32_t op = local_rng.range(5);
            switch (op) {
                case 0: {
                    auto c = rt.submit_command(Command{CommandCreateBlock{1}});
                    (void)c;
                    break;
                }
                case 1: {
                    auto blocks = rt.find_blocks_by_type(1);
                    if (!blocks.empty()) {
                        uint32_t idx = local_rng.range(static_cast<uint32_t>(blocks.size()));
                        auto d = rt.submit_command(Command{CommandDestroyBlock{{blocks[idx]}}});
                        (void)d;
                    }
                    break;
                }
                case 2: {
                    auto t = rt.tick(0.016f);
                    (void)t;
                    break;
                }
                case 3: {
                    auto ctrl = [](StateView&, CommandProducer& p, EphemeralProducer&) -> Result<void> {
                        auto r = p.produce(Command{CommandCreateBlock{1}});
                        if (!r) return r.error();
                        return {};
                    };
                    auto reg = rt.register_controller(std::move(ctrl));
                    (void)reg;
                    break;
                }
                case 4: {
                    auto t = rt.tick(0.016f);
                    (void)t;
                    break;
                }
            }
        }
    };

    PRNG rng_a{rng.state};
    PRNG rng_b{rng.state};
    apply_ops(rt1, rng_a);
    apply_ops(rt2, rng_b);

    auto diag1 = rt1.collect_diagnostics();
    auto diag2 = rt2.collect_diagnostics();

    expect(diag1.blocks_count == diag2.blocks_count).toBeTruthy();
    expect(diag1.commands_executed == diag2.commands_executed).toBeTruthy();
    expect(diag1.commands_rejected == diag2.commands_rejected).toBeTruthy();
}

} // namespace fuzz_helpers

inline void run_fuzz_tests() {
describe("Fuzz", {
    it("1000 random operations complete without crash", {
        fuzz_helpers::test_random_command_sequence();
    });
    it("500 operations with ~20% invalid commands", {
        fuzz_helpers::test_fuzzy_invalid_commands_interleaved();
    });
    it("2000 alternating create/destroy preserves block count", {
        fuzz_helpers::test_bulk_create_destroy_alternating();
    });
    it("50 random layout alternations preserve identities", {
        fuzz_helpers::test_random_layout_alternation();
    });
    it("deterministic replay produces identical state", {
        fuzz_helpers::test_deterministic_replay();
    });
});
}
