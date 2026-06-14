#pragma once

inline void run_intrusive_list_tests() {
    using namespace gameak::core;

    struct TestNode : gameak::core::intrusive_node {
        int value;
        explicit TestNode(int v) : value(v) {}
    };

    using TestList = gameak::core::intrusive_list<TestNode>;

describe("Core - intrusive_list", {
    it("push_back and iteration", {
        TestList list;
        TestNode a{10};
        TestNode b{20};
        TestNode c{30};
        list.push_back(&a);
        list.push_back(&b);
        list.push_back(&c);
        int sum = 0;
        for (auto& node : list) { sum += node.value; }
        expect(sum == 60).toBeTruthy();
    });

    it("push_front inserts in reverse order", {
        TestList list;
        TestNode a{1};
        TestNode b{2};
        list.push_front(&a);
        list.push_front(&b);
        expect(list.front().value == 2).toBeTruthy();
        expect(list.back().value == 1).toBeTruthy();
    });

    it("front and back access", {
        TestList list;
        TestNode a{10};
        TestNode b{20};
        TestNode c{30};
        list.push_back(&a);
        list.push_back(&b);
        list.push_back(&c);
        expect(list.front().value == 10).toBeTruthy();
        expect(list.back().value == 30).toBeTruthy();
    });

    it("pop_front and pop_back", {
        TestList list;
        TestNode a{10};
        TestNode b{20};
        TestNode c{30};
        list.push_back(&a);
        list.push_back(&b);
        list.push_back(&c);
        list.pop_front();
        expect(list.size() == 2).toBeTruthy();
        expect(list.front().value == 20).toBeTruthy();
        list.pop_back();
        expect(list.size() == 1).toBeTruthy();
        expect(list.back().value == 20).toBeTruthy();
    });

    it("pop_front and pop_back on empty are no-ops", {
        TestList list;
        list.pop_front();
        list.pop_back();
        expect(list.empty()).toBeTruthy();
    });

    it("insert before position", {
        TestList list;
        TestNode a{1};
        TestNode b{3};
        list.push_back(&a);
        list.push_back(&b);
        TestNode c{2};
        list.insert(++list.begin(), &c);
        expect(list.size() == 3).toBeTruthy();
        expect(list.front().value == 1).toBeTruthy();
        expect(list.back().value == 3).toBeTruthy();
    });

    it("erase at different positions", {
        TestList list;
        TestNode a{1};
        TestNode b{2};
        TestNode c{3};
        list.push_back(&a);
        list.push_back(&b);
        list.push_back(&c);

        list.erase(list.begin());
        expect(list.size() == 2).toBeTruthy();
        expect(list.front().value == 2).toBeTruthy();

        list.erase(list.begin());
        expect(list.size() == 1).toBeTruthy();

        list.erase(list.begin());
        expect(list.empty()).toBeTruthy();
    });

    it("size is tracked correctly", {
        TestList list;
        expect(list.size() == 0).toBeTruthy();
        TestNode a{1};
        TestNode b{2};
        TestNode c{3};
        list.push_back(&a);
        expect(list.size() == 1).toBeTruthy();
        list.push_back(&b);
        expect(list.size() == 2).toBeTruthy();
        list.push_back(&c);
        expect(list.size() == 3).toBeTruthy();
        list.clear();
        expect(list.size() == 0).toBeTruthy();
    });

    it("move semantics leaves source empty", {
        TestList list;
        TestNode a{1};
        TestNode b{2};
        list.push_back(&a);
        list.push_back(&b);
        TestList other = std::move(list);
        expect(other.size() == 2).toBeTruthy();
        expect(list.empty()).toBeTruthy();
    });

    it("destructor unlinks nodes", {
        TestNode a{1};
        TestNode b{2};
        {
            TestList list;
            list.push_back(&a);
            list.push_back(&b);
        }
        // After list destruction, nodes should have null next/prev
        expect(a.next == nullptr).toBeTruthy();
        expect(a.prev == nullptr).toBeTruthy();
        expect(b.next == nullptr).toBeTruthy();
        expect(b.prev == nullptr).toBeTruthy();
    });
});
}
