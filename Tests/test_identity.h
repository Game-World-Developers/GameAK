#pragma once

inline void run_identity_tests() {
    using namespace gameak::core;

describe("Core - Identity", {
    it("creates a valid identity", {
        Identity id{42};
        expect(id.value() == 42).toBeTruthy();
        expect(id.is_valid()).toBeTruthy();
    });

    it("invalid identity is zero", {
        auto id = Identity::invalid();
        expect(id.is_valid()).toBeFalsy();
        expect(id.value() == 0).toBeTruthy();
    });

    it("compares identities", {
        Identity a{1};
        Identity b{1};
        Identity c{2};
        expect(a == b).toBeTruthy();
        expect(a != c).toBeTruthy();
        expect(a < c).toBeTruthy();
        expect(c > a).toBeTruthy();
    });

    it("supports full relational comparison", {
        Identity a{1};
        Identity b{1};
        Identity c{2};
        expect(a <= b).toBeTruthy();
        expect(a >= b).toBeTruthy();
        expect(a <= c).toBeTruthy();
        expect(c >= a).toBeTruthy();
        expect(c <= a).toBeFalsy();
        expect(a >= c).toBeFalsy();
    });

    it("to_string returns expected format", {
        Identity id{42};
        auto s = id.to_string();
        expect(s.find("42") != std::string::npos).toBeTruthy();
        expect(s.find("Identity") != std::string::npos).toBeTruthy();
    });

    it("is hashable", {
        Identity a{1};
        Identity b{1};
        Identity c{2};
        std::hash<Identity> hasher;
        expect(hasher(a) == hasher(b)).toBeTruthy();
        expect(hasher(a) != hasher(c)).toBeTruthy();
    });
});
}
