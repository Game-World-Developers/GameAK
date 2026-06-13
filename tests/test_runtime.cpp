#include "gameak/core/Identity.h"
#include "gameak/core/Result.h"
#include "gameak/core/Error.h"
#include "gameak/runtime/Runtime.h"
#include "gameak/runtime/Controller.h"
#include "gameak/runtime/Command.h"

#include <cstring>
#include <vector>

#include "cest.h"

using namespace gameak::core;
using namespace gameak::runtime;

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
            Runtime rt;
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
            Runtime rt;
            auto block = rt.create_block(99);
            expect(block.has_value()).toBeFalsy();
            expect(block.error().code() == ErrorCode::TypeNotRegistered).toBeTruthy();
        });

        it("rejects duplicate type registration", {
            Runtime rt;
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
            Runtime rt;
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
            Runtime rt;
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
            Runtime rt;
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
            Runtime rt;
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

        it("rejects command for unregistered type", {
            Runtime rt;
            auto submit = rt.submit_command(Command{CommandCreateBlock{1}});
            expect(submit.has_value()).toBeTruthy();
            auto result = rt.tick();
            expect(result.commands_rejected == 1).toBeTruthy();
            expect(result.commands_executed == 0).toBeTruthy();
        });

        it("cancels a pending command", {
            Runtime rt;
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
            Runtime rt;
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
            Runtime rt;
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
            Runtime rt;
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
            Runtime rt;
            auto destroy = rt.destroy_block(Identity::invalid());
            expect(destroy.has_value()).toBeFalsy();
            expect(destroy.error().code() == ErrorCode::InvalidIdentity).toBeTruthy();
        });

        it("fails to destroy nonexistent block", {
            Runtime rt;
            auto destroy = rt.destroy_block(Identity{999});
            expect(destroy.has_value()).toBeFalsy();
            expect(destroy.error().code() == ErrorCode::BlockNotFound).toBeTruthy();
        });
    });

    return cest_result();
}
