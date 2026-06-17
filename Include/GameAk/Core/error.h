#pragma once

#include <cstdint>
#include <string>
#include <string_view>

namespace gameak::core {

enum class ErrorCode : uint32_t {
    None,
    BlockNotFound,
    BlockTypeMismatch,
    InvalidIdentity,
    CommandRejected,
    CommandInvalid,
    TypeNotRegistered,
    AllocationFailed,
    InvalidOperation,
    InternalError,
    ControllerFailed,
    DuplicateRegistration,
    CapacityExceeded,
    LayoutMismatch,
};

class Error {
public:
    Error() = default;

    explicit Error(ErrorCode code)
        : code_{code} {}

    Error(ErrorCode code, std::string message)
        : code_{code}, message_{std::move(message)} {}

    ErrorCode code() const { return code_; }
    bool has_message() const { return !message_.empty(); }
    std::string_view message() const { return message_; }
    explicit operator bool() const { return code_ != ErrorCode::None; }

private:
    ErrorCode code_{ErrorCode::None};
    std::string message_;
};

} // namespace gameak::core
