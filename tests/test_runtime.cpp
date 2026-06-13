#include "gameak/core/Identity.h"
#include "gameak/core/Result.h"
#include "gameak/core/Error.h"
#include "gameak/core/flat_vector.h"
#include "gameak/core/intrusive_list.h"
#include "gameak/core/avl_tree.h"
#include "gameak/core/rb_tree.h"
#include "gameak/runtime/Runtime.h"
#include "gameak/runtime/Controller.h"
#include "gameak/runtime/Command.h"

#include <cstring>
#include <vector>

#include "cest.h"

using namespace gameak::core;
using namespace gameak::runtime;

using FlatVec4 = gameak::core::flat_vector<int, 4>;
using FlatVec2 = gameak::core::flat_vector<int, 2>;
using FlatVec3 = gameak::core::flat_vector<int, 3>;

struct TestNode : gameak::core::intrusive_node {
    int value;
    explicit TestNode(int v) : value(v) {}
};

using TestList = gameak::core::intrusive_list<TestNode>;
using AVLTreeIntStr = gameak::core::avl_tree<int, std::string>;
using RBTreeIntStr = gameak::core::rb_tree<int, std::string>;
using DefaultRuntime = gameak::runtime::Runtime<gameak::runtime::FifoScheduler>;

static void avl_rotation_test(const int* keys, int n) {
    AVLTreeIntStr tree;
    for (int i = 0; i < n; ++i) tree.insert(keys[i], "x");
    expect(tree.size() == (size_t)n).toBeTruthy();
    int prev = -1;
    int cnt = 0;
    for (auto iter = tree.begin(); iter != tree.end(); ++iter) {
        expect(iter->first > prev).toBeTruthy();
        prev = iter->first;
        ++cnt;
    }
    expect(cnt == n).toBeTruthy();
}

static void rb_rotation_test(const int* keys, int n) {
    RBTreeIntStr tree;
    for (int i = 0; i < n; ++i) tree.insert(keys[i], "x");
    expect(tree.size() == (size_t)n).toBeTruthy();
    int prev = -1;
    int cnt = 0;
    for (auto iter = tree.begin(); iter != tree.end(); ++iter) {
        expect(iter->first > prev).toBeTruthy();
        prev = iter->first;
        ++cnt;
    }
    expect(cnt == n).toBeTruthy();
}

int main(int argc, char* argv[]) {
    cest_init(argc, argv);

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
    });

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
    });

    describe("Core - intrusive_list", {
        it("supports push erase clear and iteration", {
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

            list.erase(list.begin());
            expect(list.size() == 2).toBeTruthy();

            list.clear();
            expect(list.empty()).toBeTruthy();
        });
    });

    describe("Core - Result", {
        it("holds a value on success", {
            Result<int> r{42};
            expect(r.has_value()).toBeTruthy();
            expect(r.value()).toEqual(42);
        });

        it("holds an error on failure", {
            Result<int> r{Error{ErrorCode::BlockNotFound}};
            expect(r.has_value()).toBeFalsy();
            expect(r.error().code() == ErrorCode::BlockNotFound).toBeTruthy();
        });

        it("void result succeeds by default", {
            Result<void> r;
            expect(r.has_value()).toBeTruthy();
        });

        it("void result holds error", {
            Result<void> r{Error{ErrorCode::AllocationFailed}};
            expect(r.has_value()).toBeFalsy();
            expect(r.error().code() == ErrorCode::AllocationFailed).toBeTruthy();
        });
    });

    describe("Runtime", {
        it("creates and destroys a block", {
            DefaultRuntime rt;
            BlockTypeDescriptor desc;
            desc.type_id = 1;
            desc.size = sizeof(int);
            desc.alignment = alignof(int);
            desc.name = "test";
            auto reg = rt.register_block_type(desc);
            expect(reg.has_value()).toBeTruthy();

            auto block = rt.create_block(1);
            expect(block.has_value()).toBeTruthy();
            expect(rt.has_block(block.value())).toBeTruthy();

            auto destroy = rt.destroy_block(block.value());
            expect(destroy.has_value()).toBeTruthy();
            expect(rt.has_block(block.value())).toBeFalsy();
        });

        it("fails to create block with unregistered type", {
            DefaultRuntime rt;
            auto block = rt.create_block(99);
            expect(block.has_value()).toBeFalsy();
            expect(block.error().code() == ErrorCode::TypeNotRegistered).toBeTruthy();
        });

        it("rejects duplicate type registration", {
            DefaultRuntime rt;
            BlockTypeDescriptor desc;
            desc.type_id = 1;
            auto r1 = rt.register_block_type(desc);
            expect(r1.has_value()).toBeTruthy();
            auto r2 = rt.register_block_type(desc);
            expect(r2.has_value()).toBeFalsy();
            expect(r2.error().code() == ErrorCode::DuplicateRegistration).toBeTruthy();
        });
    });

    describe("Commands", {
        it("creates a block via command", {
            DefaultRuntime rt;
            BlockTypeDescriptor desc;
            desc.type_id = 1;
            desc.size = sizeof(int);
            desc.alignment = alignof(int);
            desc.name = "test";
            auto r1 = rt.register_block_type(desc);
            expect(r1.has_value()).toBeTruthy();

            auto submit = rt.submit_command(Command{CommandCreateBlock{1}});
            expect(submit.has_value()).toBeTruthy();

            auto result = rt.tick();
            expect(result.status == ExecutionStatus::Success).toBeTruthy();
            expect(rt.block_count(1) == 1).toBeTruthy();
        });

        it("destroys a block via command", {
            DefaultRuntime rt;
            BlockTypeDescriptor desc;
            desc.type_id = 1;
            desc.size = sizeof(int);
            desc.alignment = alignof(int);
            desc.name = "test";
            auto r1 = rt.register_block_type(desc);
            expect(r1.has_value()).toBeTruthy();

            auto block = rt.create_block(1);
            expect(block.has_value()).toBeTruthy();

            auto submit = rt.submit_command(Command{CommandDestroyBlock{{block.value()}}});
            expect(submit.has_value()).toBeTruthy();

            auto result = rt.tick();
            expect(result.status == ExecutionStatus::Success).toBeTruthy();
            expect(rt.block_count(1) == 0).toBeTruthy();
        });

        it("writes field data via command", {
            DefaultRuntime rt;
            BlockTypeDescriptor desc;
            desc.type_id = 1;
            desc.size = sizeof(int);
            desc.alignment = alignof(int);
            desc.name = "test";
            auto r1 = rt.register_block_type(desc);
            expect(r1.has_value()).toBeTruthy();

            auto block = rt.create_block(1);
            expect(block.has_value()).toBeTruthy();

            int value = 42;
            auto bytes = std::vector<std::byte>(
                reinterpret_cast<std::byte*>(&value),
                reinterpret_cast<std::byte*>(&value) + sizeof(int));

            auto submit = rt.submit_command(Command{CommandSetField{{block.value()}, 0, bytes}});
            expect(submit.has_value()).toBeTruthy();

            auto result = rt.tick();
            expect(result.status == ExecutionStatus::Success).toBeTruthy();

            auto& blocks = rt.blocks();
            auto it = blocks.find(block.value());
            expect(it != blocks.end()).toBeTruthy();

            int stored;
            std::memcpy(&stored, it->second.data(), sizeof(int));
            expect(stored).toEqual(42);
        });

        it("destroys multiple blocks with one command", {
            DefaultRuntime rt;
            BlockTypeDescriptor desc;
            desc.type_id = 1;
            desc.size = sizeof(int);
            desc.alignment = alignof(int);
            desc.name = "test";
            expect(rt.register_block_type(desc).has_value()).toBeTruthy();

            auto b1 = rt.create_block(1);
            auto b2 = rt.create_block(1);
            auto b3 = rt.create_block(1);
            expect(b1.has_value() && b2.has_value() && b3.has_value()).toBeTruthy();

            auto submit = rt.submit_command(
                Command{CommandDestroyBlock{{b1.value(), b3.value()}}});
            expect(submit.has_value()).toBeTruthy();

            auto result = rt.tick();
            expect(result.status == ExecutionStatus::Success).toBeTruthy();
            expect(rt.block_count(1) == 1).toBeTruthy();
            expect(rt.has_block(b2.value())).toBeTruthy();
        });

        it("resizes a block and writes beyond original size", {
            DefaultRuntime rt;
            BlockTypeDescriptor desc;
            desc.type_id = 1;
            desc.size = sizeof(int);
            desc.alignment = alignof(int);
            desc.name = "test";
            expect(rt.register_block_type(desc).has_value()).toBeTruthy();

            auto block = rt.create_block(1);
            expect(block.has_value()).toBeTruthy();

            auto resize = rt.submit_command(
                Command{CommandResizeBlock{{block.value()}, sizeof(int) * 2}});
            expect(resize.has_value()).toBeTruthy();
            rt.tick();
            expect(rt.blocks().at(block.value()).size() == sizeof(int) * 2).toBeTruthy();

            int value = 99;
            auto bytes = std::vector<std::byte>(
                reinterpret_cast<std::byte*>(&value),
                reinterpret_cast<std::byte*>(&value) + sizeof(int));

            auto set = rt.submit_command(
                Command{CommandSetField{{block.value()}, sizeof(int), bytes}});
            expect(set.has_value()).toBeTruthy();

            auto result = rt.tick();
            expect(result.status == ExecutionStatus::Success).toBeTruthy();

            int stored;
            std::memcpy(&stored,
                        static_cast<const std::byte*>(rt.blocks().at(block.value()).data()) + sizeof(int),
                        sizeof(int));
            expect(stored).toEqual(99);
        });

        it("rejects command for unregistered type", {
            DefaultRuntime rt;
            auto submit = rt.submit_command(Command{CommandCreateBlock{1}});
            expect(submit.has_value()).toBeTruthy();
            auto result = rt.tick();
            expect(result.commands_rejected == 1).toBeTruthy();
            expect(result.commands_executed == 0).toBeTruthy();
            expect(result.rejected_commands.size() == 1).toBeTruthy();
            expect(result.rejected_commands[0].error.code() == ErrorCode::TypeNotRegistered).toBeTruthy();
        });

        it("cancels a pending command", {
            DefaultRuntime rt;
            BlockTypeDescriptor desc;
            desc.type_id = 1;
            desc.size = sizeof(int);
            desc.alignment = alignof(int);
            desc.name = "test";
            expect(rt.register_block_type(desc).has_value()).toBeTruthy();

            auto id = rt.submit_command(Command{CommandCreateBlock{1}});
            expect(id.has_value()).toBeTruthy();
            rt.cancel_command(id.value());

            auto result = rt.tick();
            expect(result.commands_executed == 0).toBeTruthy();
            expect(result.commands_rejected == 0).toBeTruthy();
            expect(rt.block_count(1) == 0).toBeTruthy();
        });

        it("cancels one command, executes another", {
            DefaultRuntime rt;
            BlockTypeDescriptor desc;
            desc.type_id = 1;
            desc.size = sizeof(int);
            desc.alignment = alignof(int);
            desc.name = "test";
            expect(rt.register_block_type(desc).has_value()).toBeTruthy();

            auto id1 = rt.submit_command(Command{CommandCreateBlock{1}});
            auto id2 = rt.submit_command(Command{CommandCreateBlock{1}});
            expect(id2.has_value()).toBeTruthy();
            rt.cancel_command(id1.value());

            auto result = rt.tick();
            expect(result.commands_executed == 1).toBeTruthy();
            expect(rt.block_count(1) == 1).toBeTruthy();
        });
    });

    describe("Controllers", {
        it("creates a block via controller", {
            DefaultRuntime rt;
            BlockTypeDescriptor desc;
            desc.type_id = 1;
            desc.size = sizeof(int);
            desc.alignment = alignof(int);
            desc.name = "counter";
            auto r1 = rt.register_block_type(desc);
            expect(r1.has_value()).toBeTruthy();

            auto controller = [](StateView&, CommandProducer& producer) -> Result<void> {
                auto result = producer.produce(Command{CommandCreateBlock{1}});
                if (!result) return result.error();
                return {};
            };

            auto reg = rt.register_controller(std::move(controller));
            expect(reg.has_value()).toBeTruthy();

            auto result = rt.tick();
            expect(result.status == ExecutionStatus::Success).toBeTruthy();
            expect(result.controllers_executed == 1).toBeTruthy();
            expect(rt.block_count(1) == 1).toBeTruthy();
        });

        it("produces multiple commands in one tick", {
            DefaultRuntime rt;
            BlockTypeDescriptor desc;
            desc.type_id = 1;
            desc.size = sizeof(int);
            desc.alignment = alignof(int);
            desc.name = "test";
            auto r1 = rt.register_block_type(desc);
            expect(r1.has_value()).toBeTruthy();

            auto controller = [](StateView&, CommandProducer& producer) -> Result<void> {
                auto r1 = producer.produce(Command{CommandCreateBlock{1}});
                if (!r1) return r1.error();
                auto r2 = producer.produce(Command{CommandCreateBlock{1}});
                if (!r2) return r2.error();
                return {};
            };

            auto reg = rt.register_controller(std::move(controller));
            expect(reg.has_value()).toBeTruthy();

            auto result = rt.tick();
            expect(result.status == ExecutionStatus::Success).toBeTruthy();
            expect(result.commands_executed == 2).toBeTruthy();
            expect(rt.block_count(1) == 2).toBeTruthy();
        });
    });

    describe("Error Handling", {
        it("fails to destroy invalid identity", {
            DefaultRuntime rt;
            auto destroy = rt.destroy_block(Identity::invalid());
            expect(destroy.has_value()).toBeFalsy();
            expect(destroy.error().code() == ErrorCode::InvalidIdentity).toBeTruthy();
        });

        it("fails to destroy nonexistent block", {
            DefaultRuntime rt;
            auto destroy = rt.destroy_block(Identity{999});
            expect(destroy.has_value()).toBeFalsy();
            expect(destroy.error().code() == ErrorCode::BlockNotFound).toBeTruthy();
        });
    });

    describe("Core - avl_tree", {
        it("inserts finds and iterates", {
            AVLTreeIntStr tree;
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
            avl_rotation_test(k, 3);
        });
        it("handles RR rotation", {
            int k[3]; k[0] = 1; k[1] = 2; k[2] = 3;
            avl_rotation_test(k, 3);
        });
        it("handles LR rotation", {
            int k[3]; k[0] = 3; k[1] = 1; k[2] = 2;
            avl_rotation_test(k, 3);
        });
        it("handles RL rotation", {
            int k[3]; k[0] = 1; k[1] = 3; k[2] = 2;
            avl_rotation_test(k, 3);
        });
        it("handles stress 10000 inserts", {
            AVLTreeIntStr tree;
            for (int i = 0; i < 10000; ++i) tree.insert(i, "v");
            expect(tree.size() == 10000).toBeTruthy();
            int prev = -1;
            int count = 0;
            for (auto it2 = tree.begin(); it2 != tree.end(); ++it2) {
                expect(it2->first == prev + 1).toBeTruthy();
                prev = it2->first;
                ++count;
            }
            expect(count == 10000).toBeTruthy();
        });
        it("handles stress 10000 random inserts", {
            AVLTreeIntStr tree;
            for (int i = 0; i < 10000; ++i) tree.insert((i * 7 + 13) % 10000, "v");
            expect(tree.size() == 10000).toBeTruthy();
            int prev = -1;
            int count = 0;
            for (auto it2 = tree.begin(); it2 != tree.end(); ++it2) {
                expect(it2->first > prev).toBeTruthy();
                prev = it2->first;
                ++count;
            }
            expect(count == 10000).toBeTruthy();
        });
        it("supports move semantics", {
            AVLTreeIntStr a;
            a.insert(1, "a");
            AVLTreeIntStr b(std::move(a));
            expect(b.size() == 1).toBeTruthy();
            expect(b.contains(1)).toBeTruthy();
        });
        it("updates existing key", {
            AVLTreeIntStr tree;
            tree.insert(1, "old");
            tree.insert(1, "new");
            expect(tree.size() == 1).toBeTruthy();
            expect(tree.find(1)->second == "new").toBeTruthy();
        });
    });

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
            for (auto it2 = tree.begin(); it2 != tree.end(); ++it2) {
                expect(it2->first == prev + 1).toBeTruthy();
                prev = it2->first;
                ++count;
            }
            expect(count == 10000).toBeTruthy();
        });
        it("handles stress 10000 random inserts", {
            RBTreeIntStr tree;
            for (int i = 0; i < 10000; ++i) tree.insert((i * 7 + 13) % 10000, "v");
            expect(tree.size() == 10000).toBeTruthy();
            int prev = -1;
            int count = 0;
            for (auto it2 = tree.begin(); it2 != tree.end(); ++it2) {
                expect(it2->first > prev).toBeTruthy();
                prev = it2->first;
                ++count;
            }
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
    });

    return cest_result();
}
