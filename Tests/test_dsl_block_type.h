#pragma once

inline void run_dsl_block_type_tests() {
    using namespace gameak::core;
    using namespace gameak::runtime;
    using DefaultRuntime = Runtime<FifoScheduler>;

describe("DSL - Block Type", {
    it("define_block_type_with_fields", {
        struct Player { int x; int y; int hp; };
        DefaultRuntime rt;
        auto&& builder = rt.define("Player")
            .size_of<Player>()
            .has("x", &Player::x)
            .has("y", &Player::y)
            .has("hp", &Player::hp);
        auto r = std::move(builder).done();
        expect(r.has_value()).toBeTruthy();
    });

    it("define_block_type_minimal", {
        struct Player { int x; int y; int hp; };
        DefaultRuntime rt;
        auto&& builder = rt.define<Player>("Player");
        auto r = std::move(builder).done();
        expect(r.has_value()).toBeTruthy();
    });

    it("define_block_type_explicit", {
        DefaultRuntime rt;
        auto&& builder = rt.define("Score").size(4).align(4).has("value", 0, 4);
        auto r = std::move(builder).done();
        expect(r.has_value()).toBeTruthy();
    });

    it("define_block_type_duplicate_rejected", {
        DefaultRuntime rt;
        {
            auto&& b1 = rt.define("Dup").size(4).align(4);
            auto r1 = std::move(b1).done();
            expect(r1.has_value()).toBeTruthy();
        }
        {
            auto&& b2 = rt.define("Dup").size(4).align(4);
            auto r2 = std::move(b2).done();
            expect(r2.has_value()).toBeFalsy();
            expect(r2.error().code() == ErrorCode::DuplicateRegistration).toBeTruthy();
        }
    });

    it("define_ephemeral_block_type", {
        DefaultRuntime rt;
        auto&& builder = rt.define("Temp").size(4).align(4).ephemeral();
        auto r = std::move(builder).done();
        expect(r.has_value()).toBeTruthy();
        expect(rt.is_ephemeral_type(1)).toBeTruthy();
    });

    it("define_with_semantic_infers_size", {
        DefaultRuntime rt;
        auto&& builder = rt.define("Health").size(0)
            .semantic(SemanticConstraint::range(0, 100));
        auto r = std::move(builder).done();
        expect(r.has_value()).toBeTruthy();
        auto& types = rt.block_types();
        auto it = types.find(1);
        expect(it != types.end()).toBeTruthy();
        expect(it->second.size == 1).toBeTruthy();
    });
});
}
