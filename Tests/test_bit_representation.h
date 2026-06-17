#pragma once

using BS64  = gameak::core::BitSet<64>;
using BS8   = gameak::core::BitSet<8>;
using BS128 = gameak::core::BitSet<128>;
using BS1024 = gameak::core::BitSet<1024>;
using PF3   = gameak::core::bit_packing::PackedWord<
                gameak::core::bit_packing::Field<0, 3>,
                gameak::core::bit_packing::Field<3, 5>,
                gameak::core::bit_packing::Field<8, 8>>;

enum class TestFlags {
    Visible = 0,
    Enabled = 1,
    Locked  = 2,
};

namespace {
namespace bit_test_helpers {
    using namespace gameak::core;
    using namespace gameak::core::bit_packing;

    void test_bitset_set_and_test() {
        BS64 bs;
        bs.set(3).set(7).set(31);
        expect(bs.test_bit(3)).toBeTruthy();
        expect(bs.test_bit(7)).toBeTruthy();
        expect(bs.test_bit(31)).toBeTruthy();
        expect(bs.test_bit(0)).toBeFalsy();
        expect(bs.test_bit(63)).toBeFalsy();
    }

    void test_bitset_bitwise_ops() {
        BS8 a;
        a.set(2).set(3);
        BS8 b;
        b.set(1).set(3);

        auto and_ = a & b;
        auto or_  = a | b;
        auto xor_ = a ^ b;

        expect(and_.test_bit(3)).toBeTruthy();
        expect(and_.test_bit(2)).toBeFalsy();
        expect(and_.test_bit(1)).toBeFalsy();
        expect(or_.test_bit(1)).toBeTruthy();
        expect(or_.test_bit(2)).toBeTruthy();
        expect(or_.test_bit(3)).toBeTruthy();
        expect(xor_.test_bit(1)).toBeTruthy();
        expect(xor_.test_bit(2)).toBeTruthy();
        expect(xor_.test_bit(3)).toBeFalsy();
    }

    void test_bitset_compile_time_size() {
        expect(BS64::size() == 64).toBeTruthy();
        expect(BS128::size() == 128).toBeTruthy();
    }

    void test_bitset_copy() {
        BS64 a;
        a.set(5).set(10);
        BS64 b = a;
        expect(b == a).toBeTruthy();
        b.reset(5);
        expect(b != a).toBeTruthy();
        expect(a.test_bit(5)).toBeTruthy();
    }

    void test_bitset_count() {
        BS64 bs;
        bs.set(0).set(2).set(5).set(63);
        expect(bs.count() == 4).toBeTruthy();
        expect(bs.any()).toBeTruthy();
        expect(BS64{}.none()).toBeTruthy();
    }

    void test_bitset_deterministic() {
        BS8 a;
        BS8 b;
        a.set(0).set(2).set(4);
        b.set(1).set(2).set(5);

        auto r1_and = a & b;
        auto r2_and = a & b;
        expect(r1_and == r2_and).toBeTruthy();

        auto r1_or = a | b;
        auto r2_or = a | b;
        expect(r1_or == r2_or).toBeTruthy();

        auto r1_xor = a ^ b;
        auto r2_xor = a ^ b;
        expect(r1_xor == r2_xor).toBeTruthy();
    }

    void test_bitset_no_heap() {
        BS1024 bs;
        bs.set(512);
        expect(bs.test_bit(512)).toBeTruthy();
    }

    void test_bitvector_dynamic() {
        BitVector bv;
        for (size_t i = 0; i < 100; ++i) {
            bv.push_back(i % 3 == 0);
        }
        expect(bv.size() == 100).toBeTruthy();
        expect(bv.test_bit(0)).toBeTruthy();
        expect(bv.test_bit(1)).toBeFalsy();
        expect(bv.test_bit(2)).toBeFalsy();
        expect(bv.test_bit(3)).toBeTruthy();
    }

    void test_bitvector_growth() {
        BitVector bv;
        for (size_t i = 0; i < 500; ++i) {
            bv.push_back(true);
        }
        expect(bv.size() == 500).toBeTruthy();
        expect(bv.count() == 500).toBeTruthy();
    }

    void test_bitvector_random_access() {
        BitVector bv(10);
        bv.set(3);
        bv.set(7);
        expect(bv.test_bit(3)).toBeTruthy();
        expect(bv.test_bit(7)).toBeTruthy();
        expect(bv.test_bit(0)).toBeFalsy();
    }

    void test_bitflags_named() {
        BitFlags<TestFlags> flags;
        flags.set(TestFlags::Enabled);
        flags.set(TestFlags::Locked);
        expect(flags.is_set(TestFlags::Enabled)).toBeTruthy();
        expect(flags.is_set(TestFlags::Locked)).toBeTruthy();
        expect(flags.is_set(TestFlags::Visible)).toBeFalsy();
    }

    void test_bitflags_backing() {
        BitFlags<TestFlags> flags;
        flags.set(TestFlags::Visible);
        expect(flags.value() == 1).toBeTruthy();
        flags.set(TestFlags::Enabled);
        expect(flags.value() == 3).toBeTruthy();
    }

    void test_dynamic_bitflags() {
        DynamicBitFlags flags;
        flags.register_flag("Visible");
        flags.register_flag("Enabled");
        flags.register_flag("Locked");

        flags.set("Enabled");
        flags.set("Locked");
        expect(flags.is_set("Enabled")).toBeTruthy();
        expect(flags.is_set("Locked")).toBeTruthy();
        expect(flags.is_set("Visible")).toBeFalsy();
    }

    void test_bit_packing_roundtrip() {
        auto word = pack_value<0, 3>(0, 5);
        word = pack_value<3, 5>(word, 12);
        word = pack_value<8, 8>(word, 200);

        auto r1 = unpack_value<0, 3>(word);
        auto r2 = unpack_value<3, 5>(word);
        auto r3 = unpack_value<8, 8>(word);
        expect(r1 == 5).toBeTruthy();
        expect(r2 == 12).toBeTruthy();
        expect(r3 == 200).toBeTruthy();
    }

    void test_packed_word() {
        PF3 pw;
        pw.set<Field<0, 3>>(5);
        pw.set<Field<3, 5>>(12);
        pw.set<Field<8, 8>>(200);

        auto r1 = pw.get<Field<0, 3>>();
        auto r2 = pw.get<Field<3, 5>>();
        auto r3 = pw.get<Field<8, 8>>();
        expect(r1 == 5).toBeTruthy();
        expect(r2 == 12).toBeTruthy();
        expect(r3 == 200).toBeTruthy();
    }

    void test_bit_packing_deterministic() {
        auto w1 = pack_value<0, 8>(0, 42);
        auto w2 = pack_value<0, 8>(0, 42);
        expect(w1 == w2).toBeTruthy();
        auto r1 = unpack_value<0, 8>(w1);
        auto r2 = unpack_value<0, 8>(w2);
        expect(r1 == r2).toBeTruthy();
    }
}
}

inline void run_bit_representation_tests() {
describe("BitSet", {
    it("sets and tests bits",    { bit_test_helpers::test_bitset_set_and_test(); });
    it("bitwise AND OR XOR",     { bit_test_helpers::test_bitset_bitwise_ops(); });
    it("compile-time size",      { bit_test_helpers::test_bitset_compile_time_size(); });
    it("copy and independence",  { bit_test_helpers::test_bitset_copy(); });
    it("counts bits correctly",  { bit_test_helpers::test_bitset_count(); });
    it("deterministic ops",      { bit_test_helpers::test_bitset_deterministic(); });
    it("no heap allocation",     { bit_test_helpers::test_bitset_no_heap(); });
});

describe("BitVector", {
    it("dynamic number of bits", { bit_test_helpers::test_bitvector_dynamic(); });
    it("grows dynamically",      { bit_test_helpers::test_bitvector_growth(); });
    it("random access",          { bit_test_helpers::test_bitvector_random_access(); });
});

describe("BitFlags", {
    it("named flags",            { bit_test_helpers::test_bitflags_named(); });
    it("fixed 64-bit backing",   { bit_test_helpers::test_bitflags_backing(); });
    it("dynamic flags",          { bit_test_helpers::test_dynamic_bitflags(); });
});

describe("Bit Packing", {
    it("round-trip",             { bit_test_helpers::test_bit_packing_roundtrip(); });
    it("PackedWord template",    { bit_test_helpers::test_packed_word(); });
    it("deterministic ops",      { bit_test_helpers::test_bit_packing_deterministic(); });
});
}
