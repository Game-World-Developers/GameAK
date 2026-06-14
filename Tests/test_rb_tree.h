#pragma once

inline void run_rb_tree_tests() {
    using namespace gameak::core;
    using RBTreeIntStr = rb_tree<int, std::string>;

    auto rb_rotation_test = [](const int* keys, int n) {
        RBTreeIntStr tree;
        for (int i = 0; i < n; ++i) tree.insert(keys[i], "x");
        expect(tree.size() == (size_t)n).toBeTruthy();
        expect(tree.check_invariants()).toBeTruthy();
        int prev = -1;
        int cnt = 0;
        bool sorted = true;
        for (auto iter = tree.begin(); iter != tree.end(); ++iter) {
            if (iter->first <= prev) sorted = false;
            prev = iter->first;
            ++cnt;
        }
        expect(sorted).toBeTruthy();
        expect(cnt == n).toBeTruthy();
    };

describe("Core - rb_tree", {
    it("inserts finds and iterates", {
        RBTreeIntStr tree;
        expect(tree.empty()).toBeTruthy();
        tree.insert(3, "three");
        tree.insert(1, "one");
        tree.insert(4, "four");
        tree.insert(2, "two");
        expect(tree.size() == 4).toBeTruthy();
        expect(tree.contains(3)).toBeTruthy();
        expect(tree.contains(5)).toBeFalsy();
        auto iter = tree.find(2);
        expect(iter != tree.end()).toBeTruthy();
        expect(iter->second == "two").toBeTruthy();
        int prev = 0;
        int count = 0;
        for (auto it2 = tree.begin(); it2 != tree.end(); ++it2) {
            expect(it2->first > prev).toBeTruthy();
            prev = it2->first;
            ++count;
        }
        expect(count == 4).toBeTruthy();
    });
    it("handles LL rotation", {
        int k[3]; k[0] = 3; k[1] = 2; k[2] = 1;
        rb_rotation_test(k, 3);
    });
    it("handles RR rotation", {
        int k[3]; k[0] = 1; k[1] = 2; k[2] = 3;
        rb_rotation_test(k, 3);
    });
    it("handles LR rotation", {
        int k[3]; k[0] = 3; k[1] = 1; k[2] = 2;
        rb_rotation_test(k, 3);
    });
    it("handles RL rotation", {
        int k[3]; k[0] = 1; k[1] = 3; k[2] = 2;
        rb_rotation_test(k, 3);
    });
    it("handles stress 10000 inserts", {
        RBTreeIntStr tree;
        for (int i = 0; i < 10000; ++i) tree.insert(i, "v");
        expect(tree.size() == 10000).toBeTruthy();
        int prev = -1;
        int count = 0;
        bool sorted = true;
        for (auto it2 = tree.begin(); it2 != tree.end(); ++it2) {
            if (it2->first != prev + 1) sorted = false;
            prev = it2->first;
            ++count;
        }
        expect(sorted).toBeTruthy();
        expect(count == 10000).toBeTruthy();
    });
    it("handles stress 10000 random inserts", {
        RBTreeIntStr tree;
        for (int i = 0; i < 10000; ++i) tree.insert((i * 7 + 13) % 10000, "v");
        expect(tree.size() == 10000).toBeTruthy();
        int prev = -1;
        int count = 0;
        bool sorted = true;
        for (auto it2 = tree.begin(); it2 != tree.end(); ++it2) {
            if (it2->first <= prev) sorted = false;
            prev = it2->first;
            ++count;
        }
        expect(sorted).toBeTruthy();
        expect(count == 10000).toBeTruthy();
    });
    it("supports move semantics", {
        RBTreeIntStr a;
        a.insert(1, "a");
        RBTreeIntStr b(std::move(a));
        expect(b.size() == 1).toBeTruthy();
        expect(b.contains(1)).toBeTruthy();
    });
    it("updates existing key", {
        RBTreeIntStr tree;
        tree.insert(1, "old");
        tree.insert(1, "new");
        expect(tree.size() == 1).toBeTruthy();
        expect(tree.find(1)->second == "new").toBeTruthy();
    });
    it("erases leaf node", {
        RBTreeIntStr tree;
        tree.insert(1, "a");
        tree.insert(2, "b");
        tree.insert(3, "c");
        tree.erase(1);
        expect(tree.size() == 2).toBeTruthy();
        expect(tree.contains(1)).toBeFalsy();
        int prev = 0;
        int cnt = 0;
        for (auto it = tree.begin(); it != tree.end(); ++it) {
            expect(it->first > prev).toBeTruthy();
            prev = it->first;
            ++cnt;
        }
        expect(cnt == 2).toBeTruthy();
    });
    it("erases node with one child", {
        RBTreeIntStr tree;
        tree.insert(1, "a");
        tree.insert(2, "b");
        tree.insert(3, "c");
        tree.erase(2);
        expect(tree.size() == 2).toBeTruthy();
        expect(tree.contains(2)).toBeFalsy();
    });
    it("erases node with two children", {
        RBTreeIntStr tree;
        tree.insert(2, "a");
        tree.insert(1, "b");
        tree.insert(3, "c");
        tree.erase(2);
        expect(tree.size() == 2).toBeTruthy();
        expect(tree.contains(2)).toBeFalsy();
    });
    it("erases root", {
        RBTreeIntStr tree;
        tree.insert(5, "a");
        tree.insert(3, "b");
        tree.insert(7, "c");
        tree.insert(2, "d");
        tree.insert(4, "e");
        tree.erase(5);
        expect(tree.size() == 4).toBeTruthy();
        int prev = 0;
        int cnt = 0;
        for (auto it = tree.begin(); it != tree.end(); ++it) {
            expect(it->first > prev).toBeTruthy();
            prev = it->first;
            ++cnt;
        }
        expect(cnt == 4).toBeTruthy();
    });
    it("erases nonexistent key", {
        RBTreeIntStr tree;
        tree.insert(1, "a");
        tree.erase(99);
        expect(tree.size() == 1).toBeTruthy();
    });

    it("iterator::raw returns underlying node pointer", {
        RBTreeIntStr tree;
        tree.insert(42, "forty-two");
        auto it = tree.find(42);
        expect(it != tree.end()).toBeTruthy();
        auto* raw = it.raw();
        expect(raw != nullptr).toBeTruthy();
        expect(raw->kv.first == 42).toBeTruthy();
    });

    it("is nothrow move constructible", {
        static_assert(std::is_nothrow_move_constructible_v<RBTreeIntStr>);
        static_assert(std::is_nothrow_move_assignable_v<RBTreeIntStr>);
    });
});
}
