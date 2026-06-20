#pragma once

inline void run_result_monadic_tests() {
    using namespace gameak::core;

describe("Result - Monadic", {
    it("result_and_then_chains_on_success", {
        auto make_ok = []() -> Result<int> { return 42; };
        bool called = false;
        auto r = make_ok().and_then([&](int v) -> Result<void> {
            called = true;
            expect(v == 42).toBeTruthy();
            return {};
        });
        expect(called).toBeTruthy();
        expect(r.has_value()).toBeTruthy();
    });

    it("result_and_then_short_circuits_on_error", {
        auto make_err = []() -> Result<int> {
            return Error(ErrorCode::InvalidOperation);
        };
        bool called = false;
        auto r = make_err().and_then([&](int) -> Result<void> {
            called = true;
            return {};
        });
        expect(called).toBeFalsy();
        expect(r.has_value()).toBeFalsy();
        expect(r.error().code() == ErrorCode::InvalidOperation).toBeTruthy();
    });

    it("result_and_then_changes_type", {
        auto make_ok = []() -> Result<int> { return 42; };
        auto r = make_ok().and_then([](int v) -> Result<long> {
            return Result<long>(static_cast<long>(v) * 2L);
        });
        expect(r.has_value()).toBeTruthy();
        expect(r.value() == 84L).toBeTruthy();
    });

    it("result_or_else_handles_error", {
        auto make_err = []() -> Result<int> {
            return Error(ErrorCode::TypeNotRegistered);
        };
        bool handled = false;
        make_err().or_else([&](const Error& e) {
            handled = true;
            expect(e.code() == ErrorCode::TypeNotRegistered).toBeTruthy();
        });
        expect(handled).toBeTruthy();
    });

    it("result_or_else_not_called_on_success", {
        auto make_ok = []() -> Result<int> { return 99; };
        bool handled = false;
        make_ok().or_else([&](const Error&) {
            handled = true;
        });
        expect(handled).toBeFalsy();
    });

    it("result_try_macro_early_return", {
        auto make_val = []() -> Result<int> { return 42; };
        auto make_err = []() -> Result<void> {
            return Error(ErrorCode::AllocationFailed);
        };
        // Must declare v before TRY since the macro uses assignment, not declaration
        auto test_fn = [&]() -> Result<int> {
            int v = 0;
            GAME_AK_TRY(v, make_val());
            GAME_AK_TRY_VOID(make_err());
            return v;
        };
        auto r = test_fn();
        expect(r.has_value()).toBeFalsy();
        expect(r.error().code() == ErrorCode::AllocationFailed).toBeTruthy();
    });
});
}
