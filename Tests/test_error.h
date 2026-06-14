#pragma once

inline void run_error_tests() {
    using namespace gameak::core;

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

    it("result move semantics", {
        Result<int> r{42};
        Result<int> s = std::move(r);
        expect(s.has_value()).toBeTruthy();
        expect(s.value()).toEqual(42);
    });
});

describe("Core - Error", {
    it("all error codes are distinct", {
        Error e0{ErrorCode::None};
        Error e1{ErrorCode::BlockNotFound};
        Error e2{ErrorCode::BlockTypeMismatch};
        Error e3{ErrorCode::InvalidIdentity};
        Error e4{ErrorCode::CommandRejected};
        Error e5{ErrorCode::CommandInvalid};
        Error e6{ErrorCode::TypeNotRegistered};
        Error e7{ErrorCode::AllocationFailed};
        Error e8{ErrorCode::InvalidOperation};
        Error e9{ErrorCode::InternalError};
        Error e10{ErrorCode::ControllerFailed};
        Error e11{ErrorCode::DuplicateRegistration};
        Error e12{ErrorCode::CapacityExceeded};
        expect(e0.code() == ErrorCode::None).toBeTruthy();
        expect(e1.code() == ErrorCode::BlockNotFound).toBeTruthy();
        expect(e2.code() == ErrorCode::BlockTypeMismatch).toBeTruthy();
        expect(e3.code() == ErrorCode::InvalidIdentity).toBeTruthy();
        expect(e4.code() == ErrorCode::CommandRejected).toBeTruthy();
        expect(e5.code() == ErrorCode::CommandInvalid).toBeTruthy();
        expect(e6.code() == ErrorCode::TypeNotRegistered).toBeTruthy();
        expect(e7.code() == ErrorCode::AllocationFailed).toBeTruthy();
        expect(e8.code() == ErrorCode::InvalidOperation).toBeTruthy();
        expect(e9.code() == ErrorCode::InternalError).toBeTruthy();
        expect(e10.code() == ErrorCode::ControllerFailed).toBeTruthy();
        expect(e11.code() == ErrorCode::DuplicateRegistration).toBeTruthy();
        expect(e12.code() == ErrorCode::CapacityExceeded).toBeTruthy();
    });

    it("error with diagnostic message", {
        Error e(ErrorCode::BlockNotFound, "block 42 not found");
        expect(e.code() == ErrorCode::BlockNotFound).toBeTruthy();
        expect(e.has_message()).toBeTruthy();
        expect(e.message() == "block 42 not found").toBeTruthy();
    });

    it("error without message has no message", {
        Error e{ErrorCode::InternalError};
        expect(e.has_message()).toBeFalsy();
        expect(e.message().empty()).toBeTruthy();
    });

    it("default error is None", {
        Error e;
        expect(e.code() == ErrorCode::None).toBeTruthy();
        expect(e.has_message()).toBeFalsy();
    });

    it("operator bool returns false for None", {
        Error none{ErrorCode::None};
        Error err{ErrorCode::BlockNotFound};
        expect(static_cast<bool>(none)).toBeFalsy();
        expect(static_cast<bool>(err)).toBeTruthy();
    });
});
}
