#pragma once

inline void run_flat_vector_tests() {
    using namespace gameak::core;

    using FlatVec2 = flat_vector<int, 2>;
    using FlatVec3 = flat_vector<int, 3>;
    using FlatVec4 = flat_vector<int, 4>;
    using MoveOnlyVec = flat_vector<std::unique_ptr<int>, 2>;

describe("Core - flat_vector", {
    it("stores elements inline for small sizes", {
        FlatVec4 v;
        expect(v.empty()).toBeTruthy();
        expect(v.capacity() == 4).toBeTruthy();

        v.push_back(10);
        v.push_back(20);
        v.push_back(30);

        expect(v.size() == 3).toBeTruthy();
        expect(v[0] == 10).toBeTruthy();
        expect(v[1] == 20).toBeTruthy();
        expect(v[2] == 30).toBeTruthy();
        expect(v.data() != nullptr).toBeTruthy();
    });

    it("grows to heap when inline capacity exceeded", {
        FlatVec2 v;
        v.push_back(1);
        v.push_back(2);
        expect(v.size() == 2).toBeTruthy();

        v.push_back(3);
        expect(v.size() == 3).toBeTruthy();
        expect(v[0] == 1).toBeTruthy();
        expect(v[1] == 2).toBeTruthy();
        expect(v[2] == 3).toBeTruthy();
    });

    it("supports pop_back and clear", {
        FlatVec2 v;
        v.push_back(1);
        v.push_back(2);
        v.push_back(3);
        expect(v.size() == 3).toBeTruthy();

        v.pop_back();
        expect(v.size() == 2).toBeTruthy();
        expect(v.back() == 2).toBeTruthy();

        v.clear();
        expect(v.empty()).toBeTruthy();
    });

    it("supports iteration", {
        FlatVec3 v;
        v.push_back(1);
        v.push_back(2);
        v.push_back(3);

        int sum = 0;
        for (auto x : v) { sum += x; }
        expect(sum == 6).toBeTruthy();
    });

    it("supports resize", {
        FlatVec2 v;
        v.push_back(10);
        v.push_back(20);
        v.push_back(30);

        v.resize(2);
        expect(v.size() == 2).toBeTruthy();
        expect(v[0] == 10).toBeTruthy();
        expect(v[1] == 20).toBeTruthy();
    });

    it("front and back access", {
        FlatVec3 v;
        v.push_back(10);
        v.push_back(20);
        v.push_back(30);
        expect(v.front() == 10).toBeTruthy();
        expect(v.back() == 30).toBeTruthy();
    });

    it("emplace_back constructs in place", {
        FlatVec3 v;
        v.emplace_back(1);
        v.emplace_back(2);
        expect(v.size() == 2).toBeTruthy();
        expect(v[0] == 1).toBeTruthy();
        expect(v[1] == 2).toBeTruthy();
    });

    it("reserve grows capacity", {
        FlatVec2 v;
        expect(v.capacity() == 2).toBeTruthy();
        v.reserve(10);
        expect(v.capacity() >= 10).toBeTruthy();
    });

    it("at access", {
        FlatVec3 v;
        v.push_back(5);
        v.push_back(6);
        expect(v.at(0) == 5).toBeTruthy();
        expect(v.at(1) == 6).toBeTruthy();
    });

    it("erase single element", {
        FlatVec3 v;
        v.push_back(1);
        v.push_back(2);
        v.push_back(3);
        auto it = v.erase(v.begin() + 1);
        expect(v.size() == 2).toBeTruthy();
        expect(v[0] == 1).toBeTruthy();
        expect(v[1] == 3).toBeTruthy();
        expect(*it == 3).toBeTruthy();
    });

    it("erase range", {
        FlatVec3 v;
        v.push_back(1);
        v.push_back(2);
        v.push_back(3);
        v.erase(v.begin() + 1, v.end());
        expect(v.size() == 1).toBeTruthy();
        expect(v[0] == 1).toBeTruthy();
    });

    it("copy semantics", {
        FlatVec2 v;
        v.push_back(1);
        v.push_back(2);
        v.push_back(3);
        FlatVec2 w = v;
        expect(w.size() == 3).toBeTruthy();
        expect(w[0] == 1).toBeTruthy();
        expect(w[1] == 2).toBeTruthy();
        expect(w[2] == 3).toBeTruthy();
    });

    it("move semantics leaves source empty", {
        FlatVec2 v;
        v.push_back(1);
        v.push_back(2);
        v.push_back(3);
        FlatVec2 w = std::move(v);
        expect(w.size() == 3).toBeTruthy();
        expect(w[0] == 1).toBeTruthy();
        expect(v.empty()).toBeTruthy();
    });

    it("push_back by move", {
        FlatVec2 v;
        v.push_back(100);
        v.push_back(200);
        expect(v.size() == 2).toBeTruthy();
        expect(v[0] == 100).toBeTruthy();
        expect(v[1] == 200).toBeTruthy();
    });

    it("reserve is no-op when already large enough", {
        FlatVec2 v;
        v.reserve(10);
        auto cap = v.capacity();
        v.reserve(5);
        expect(v.capacity() == cap).toBeTruthy();
    });

    it("resize default-constructs new elements", {
        FlatVec2 v;
        v.push_back(1);
        v.push_back(2);
        v.resize(5);
        expect(v.size() == 5).toBeTruthy();
        expect(v[0] == 1).toBeTruthy();
        expect(v[1] == 2).toBeTruthy();
        expect(v[2] == 0).toBeTruthy();
        expect(v[3] == 0).toBeTruthy();
        expect(v[4] == 0).toBeTruthy();
    });

    it("heap fallback with move-only type", {
        MoveOnlyVec v;
        v.push_back(std::make_unique<int>(10));
        v.push_back(std::make_unique<int>(20));
        expect(v.size() == 2).toBeTruthy();
        expect(*v[0] == 10).toBeTruthy();
        expect(*v[1] == 20).toBeTruthy();

        // Trigger heap fallback
        v.push_back(std::make_unique<int>(30));
        expect(v.size() == 3).toBeTruthy();
        expect(*v[0] == 10).toBeTruthy();
        expect(*v[1] == 20).toBeTruthy();
        expect(*v[2] == 30).toBeTruthy();
    });
});
}
