#pragma once

inline void run_semantic_tests() {
    using namespace gameak::core;

describe("Core - Semantic", {

    it("semantic_bits_for_bool", {
        SemanticConstraint sc;
        sc.is_bool = true;
        expect(bits_for(sc) == 1).toBeTruthy();
        expect(sc.valid()).toBeTruthy();
    });

    it("semantic_bits_for_enum", {
        SemanticConstraint sc;
        sc.enum_count = 4;
        expect(bits_for(sc) == 2).toBeTruthy();

        SemanticConstraint sc2;
        sc2.enum_count = 1;
        expect(bits_for(sc2) == 1).toBeTruthy();

        SemanticConstraint sc3;
        sc3.enum_count = 8;
        expect(bits_for(sc3) == 3).toBeTruthy();
    });

    it("semantic_bits_for_range_100", {
        SemanticConstraint sc;
        sc.min = 0;
        sc.max = 100;
        expect(bits_for(sc) == 7).toBeTruthy();
        expect(sc.valid()).toBeTruthy();
    });

    it("semantic_bits_for_single_value", {
        SemanticConstraint sc;
        sc.min = 42;
        sc.max = 42;
        expect(bits_for(sc) == 1).toBeTruthy();
        expect(sc.valid()).toBeTruthy();
    });

    it("semantic_bits_for_zero_range", {
        SemanticConstraint sc;
        expect(bits_for(sc) == 0).toBeTruthy();
        expect(sc.valid()).toBeFalsy();
    });

    it("semantic_bits_for_negative_range", {
        SemanticConstraint sc;
        sc.min = -100;
        sc.max = 100;
        expect(bits_for(sc) == 8).toBeTruthy();
        expect(sc.valid()).toBeTruthy();
    });

    it("semantic_bits_for_large_range", {
        SemanticConstraint sc;
        sc.min = 0;
        sc.max = 1000000;
        expect(bits_for(sc) == 20).toBeTruthy();
    });

    it("semantic_bits_for_range_255", {
        SemanticConstraint sc;
        sc.min = 0;
        sc.max = 255;
        expect(bits_for(sc) == 8).toBeTruthy();
    });

    it("semantic_bytes_for_bool", {
        SemanticConstraint sc;
        sc.is_bool = true;
        expect(bytes_for(sc) == 1).toBeTruthy();
    });

    it("semantic_bytes_for_range", {
        SemanticConstraint sc;
        sc.min = 0;
        sc.max = 100;
        expect(bytes_for(sc) == 1).toBeTruthy();

        SemanticConstraint sc2;
        sc2.min = 0;
        sc2.max = 65535;
        expect(bytes_for(sc2) == 2).toBeTruthy();
    });

    it("semantic_validation", {
        SemanticConstraint empty;
        expect(empty.valid()).toBeFalsy();

        SemanticConstraint as_bool;
        as_bool.is_bool = true;
        expect(as_bool.valid()).toBeTruthy();

        SemanticConstraint as_enum;
        as_enum.enum_count = 4;
        expect(as_enum.valid()).toBeTruthy();

        SemanticConstraint as_range;
        as_range.min = 0;
        as_range.max = 10;
        expect(as_range.valid()).toBeTruthy();

        SemanticConstraint single;
        single.min = 5;
        single.max = 5;
        expect(single.valid()).toBeTruthy();

        SemanticConstraint zero;
        zero.min = 0;
        zero.max = 0;
        expect(zero.valid()).toBeFalsy();
    });

    it("semantic_factory_range", {
        auto sc = SemanticConstraint::range(0, 100);
        expect(sc.min == 0).toBeTruthy();
        expect(sc.max == 100).toBeTruthy();
        expect(sc.is_bool).toBeFalsy();
        expect(sc.enum_count == 0).toBeTruthy();
        expect(bits_for(sc) == 7).toBeTruthy();
    });

    it("semantic_factory_boolean", {
        auto sc = SemanticConstraint::boolean();
        expect(sc.is_bool).toBeTruthy();
        expect(bits_for(sc) == 1).toBeTruthy();
    });

    it("semantic_factory_enumeration", {
        auto sc = SemanticConstraint::enumeration(4);
        expect(sc.enum_count == 4).toBeTruthy();
        expect(bits_for(sc) == 2).toBeTruthy();
    });
});

describe("Runtime - Semantic Registration", {
    using namespace gameak::runtime;
    using DefaultRuntime = Runtime<FifoScheduler>;

    it("semantic_registration_infers_size", {
        DefaultRuntime rt;
        BlockTypeDescriptor desc;
        desc.type_id = 1;
        desc.name = "health";
        desc.size = 0;
        desc.alignment = 0;

        SemanticConstraint sc;
        sc.min = 0;
        sc.max = 100;

        desc.semantic = &sc;
        auto r = rt.register_block_type(desc);
        expect(r.has_value()).toBeTruthy();
        expect(rt.block_types().at(1).size == 1).toBeTruthy();
        expect(rt.block_types().at(1).alignment == 1).toBeTruthy();
    });

    it("semantic_registration_explicit_size", {
        DefaultRuntime rt;
        BlockTypeDescriptor desc;
        desc.type_id = 2;
        desc.name = "score";
        desc.size = 8;
        desc.alignment = 8;

        SemanticConstraint sc;
        sc.min = 0;
        sc.max = 100;

        desc.semantic = &sc;
        auto r = rt.register_block_type(desc);
        expect(r.has_value()).toBeTruthy();
        expect(rt.block_types().at(2).size == 8).toBeTruthy();
        expect(rt.block_types().at(2).alignment == 8).toBeTruthy();
    });

    it("semantic_registration_no_semantic", {
        DefaultRuntime rt;
        BlockTypeDescriptor desc;
        desc.type_id = 3;
        desc.name = "legacy";
        desc.size = 4;
        desc.alignment = 4;

        auto r = rt.register_block_type(desc);
        expect(r.has_value()).toBeTruthy();
        expect(rt.block_types().at(3).size == 4).toBeTruthy();
        expect(rt.block_types().at(3).alignment == 4).toBeTruthy();
    });

    it("semantic_registration_invalid_constraint", {
        DefaultRuntime rt;
        BlockTypeDescriptor desc;
        desc.type_id = 4;
        desc.name = "bad";
        desc.size = 0;

        SemanticConstraint sc;
        desc.semantic = &sc;
        auto r = rt.register_block_type(desc);
        expect(r.has_value()).toBeFalsy();
        expect(r.error().code() == ErrorCode::InvalidOperation).toBeTruthy();
    });

    it("semantic_registration_size_zero_no_constraint_is_backward_compatible", {
        DefaultRuntime rt;
        BlockTypeDescriptor desc;
        desc.type_id = 5;
        desc.name = "no_size_no_sc";
        desc.size = 0;

        auto r = rt.register_block_type(desc);
        expect(r.has_value()).toBeTruthy();
        expect(rt.block_types().at(5).size == 0).toBeTruthy();
    });

    it("semantic_registration_creates_blocks_with_inferred_size", {
        DefaultRuntime rt;
        BlockTypeDescriptor desc;
        desc.type_id = 10;
        desc.name = "inferred_health";
        desc.size = 0;

        SemanticConstraint sc;
        sc.min = 0;
        sc.max = 100;

        desc.semantic = &sc;
        expect(rt.register_block_type(desc).has_value()).toBeTruthy();

        auto block = rt.create_block(10);
        expect(block.has_value()).toBeTruthy();
        expect(rt.block_types().at(10).size == 1).toBeTruthy();

        const auto& dt = static_cast<const DefaultRuntime&>(rt);
        auto blocks = dt.find_blocks_by_type(10);
        expect(blocks.size() == 1).toBeTruthy();
    });

    it("semantic_registration_bool_type", {
        DefaultRuntime rt;
        BlockTypeDescriptor desc;
        desc.type_id = 11;
        desc.name = "alive_flag";
        desc.size = 0;

        SemanticConstraint sc;
        sc.is_bool = true;

        desc.semantic = &sc;
        expect(rt.register_block_type(desc).has_value()).toBeTruthy();
        expect(rt.block_types().at(11).size == 1).toBeTruthy();
    });

    it("semantic_registration_enum_type", {
        DefaultRuntime rt;
        BlockTypeDescriptor desc;
        desc.type_id = 12;
        desc.name = "faction";
        desc.size = 0;

        SemanticConstraint sc;
        sc.enum_count = 4;

        desc.semantic = &sc;
        expect(rt.register_block_type(desc).has_value()).toBeTruthy();
        expect(rt.block_types().at(12).size == 1).toBeTruthy();
    });

    it("semantic_registration_wide_range_uses_two_bytes", {
        DefaultRuntime rt;
        BlockTypeDescriptor desc;
        desc.type_id = 13;
        desc.name = "temperature";
        desc.size = 0;

        SemanticConstraint sc;
        sc.min = 0;
        sc.max = 10000;

        desc.semantic = &sc;
        expect(rt.register_block_type(desc).has_value()).toBeTruthy();
        expect(rt.block_types().at(13).size == 2).toBeTruthy();
    });
});
}
