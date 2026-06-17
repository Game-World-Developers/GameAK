#pragma once

namespace {
namespace event_loop_helpers {
    using namespace gameak::core;
    using namespace gameak::runtime;
    using DefaultRuntime = Runtime<FifoScheduler>;

    void test_event_dispatches_to_handler() {
        EventLoop<int> el;
        bool handled = false;

        el.on(1, [&](const int& data, CommandProducer&, EphemeralProducer&) { handled = true; (void)data; });
        el.enqueue(1, 42);

        DefaultRuntime rt;
        Controller c = el.build();
        expect(rt.register_controller(std::move(c)).has_value()).toBeTruthy();

        auto r = rt.tick();
        expect(r.status == ExecutionStatus::Success).toBeTruthy();
        expect(handled).toBeTruthy();
    }

    void test_event_no_handlers() {
        EventLoop<int> el;
        el.enqueue(99, 100);

        DefaultRuntime rt;
        Controller c = el.build();
        expect(rt.register_controller(std::move(c)).has_value()).toBeTruthy();

        auto r = rt.tick();
        expect(r.status == ExecutionStatus::Success).toBeTruthy();
    }

    void test_multiple_handlers() {
        int h1 = 0;
        int h2 = 0;
        EventLoop<int> el;

        el.on(1, [&](const int&, CommandProducer&, EphemeralProducer&) { h1++; });
        el.on(1, [&](const int&, CommandProducer&, EphemeralProducer&) { h2++; });
        el.enqueue(1, 10);

        DefaultRuntime rt;
        Controller c = el.build();
        expect(rt.register_controller(std::move(c)).has_value()).toBeTruthy();

        auto r = rt.tick();
        expect(r.status == ExecutionStatus::Success).toBeTruthy();
        expect(h1 == 1).toBeTruthy();
        expect(h2 == 1).toBeTruthy();
    }

    void test_fifo_order() {
        std::vector<int> received;
        EventLoop<int> el;

        el.on(1, [&](const int& data, CommandProducer&, EphemeralProducer&) { received.push_back(data); });
        el.enqueue(1, 10);
        el.enqueue(1, 20);
        el.enqueue(1, 30);

        DefaultRuntime rt;
        Controller c = el.build();
        expect(rt.register_controller(std::move(c)).has_value()).toBeTruthy();

        auto r = rt.tick();
        expect(r.status == ExecutionStatus::Success).toBeTruthy();
        expect(received.size() == 3).toBeTruthy();
        expect(received[0] == 10).toBeTruthy();
        expect(received[1] == 20).toBeTruthy();
        expect(received[2] == 30).toBeTruthy();
    }

    void test_handler_receives_data() {
        struct DamageEvent { int amount; uint64_t source; };

        EventLoop<DamageEvent> el;
        int received_amount = 0;
        uint64_t received_source = 0;

        el.on(1, [&](const DamageEvent& data, CommandProducer&, EphemeralProducer&) {
            received_amount = data.amount;
            received_source = data.source;
        });

        el.enqueue(1, DamageEvent{10, 42});

        DefaultRuntime rt;
        Controller c = el.build();
        expect(rt.register_controller(std::move(c)).has_value()).toBeTruthy();

        auto r = rt.tick();
        expect(r.status == ExecutionStatus::Success).toBeTruthy();
        expect(received_amount == 10).toBeTruthy();
        expect(received_source == 42).toBeTruthy();
    }

    void test_event_loop_is_controller() {
        EventLoop<int> el;
        Controller c = el.build();
        expect(static_cast<bool>(c)).toBeTruthy();
    }

    void test_handlers_produce_commands() {
        DefaultRuntime rt;
        BlockTypeDescriptor desc;
        desc.type_id = 1; desc.size = sizeof(int); desc.alignment = alignof(int); desc.name = "test";
        expect(rt.register_block_type(desc).has_value()).toBeTruthy();

        EventLoop<int> el;
        el.on(1, [&](const int&, CommandProducer& producer, EphemeralProducer&) { auto _ = producer.produce(Command{CommandCreateBlock{1}}); (void)_; });
        el.enqueue(1, 0);

        Controller c = el.build();
        expect(rt.register_controller(std::move(c)).has_value()).toBeTruthy();

        auto r = rt.tick();
        expect(r.status == ExecutionStatus::Success).toBeTruthy();
        expect(r.commands_executed == 1).toBeTruthy();
        expect(rt.block_count(1) == 1).toBeTruthy();
    }

    void test_no_state_between_ticks() {
        EventLoop<int> el;
        el.enqueue(1, 100);

        DefaultRuntime rt;
        Controller c = el.build();
        expect(rt.register_controller(std::move(c)).has_value()).toBeTruthy();

        auto r1 = rt.tick();
        expect(r1.status == ExecutionStatus::Success).toBeTruthy();
        expect(el.pending_count() == 0).toBeTruthy();
    }
}
}

inline void run_event_loop_tests() {
describe("EventLoop", {
    it("dispatches to handler",              { event_loop_helpers::test_event_dispatches_to_handler(); });
    it("no handlers silently consumed",      { event_loop_helpers::test_event_no_handlers(); });
    it("multiple handlers execute",          { event_loop_helpers::test_multiple_handlers(); });
    it("FIFO order",                         { event_loop_helpers::test_fifo_order(); });
    it("handler receives data",              { event_loop_helpers::test_handler_receives_data(); });
    it("is a Controller",                    { event_loop_helpers::test_event_loop_is_controller(); });
    it("handlers produce commands",          { event_loop_helpers::test_handlers_produce_commands(); });
    it("no state between ticks",             { event_loop_helpers::test_no_state_between_ticks(); });
});
}
