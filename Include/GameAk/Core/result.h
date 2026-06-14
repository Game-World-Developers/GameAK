#pragma once

#include "error.h"

#include <type_traits>
#include <utility>

namespace gameak::core {

template <typename T>
class [[nodiscard]] Result {
public:
    Result(T value)
        : has_value_{true}, value_{std::move(value)} {}

    Result(Error error)
        : has_value_{false}, error_{std::move(error)} {}

    ~Result() {
        if (has_value_) {
            value_.~T();
        } else {
            error_.~Error();
        }
    }

    Result(const Result&) = delete;
    Result& operator=(const Result&) = delete;

    Result(Result&& other) noexcept(std::is_nothrow_move_constructible_v<T>)
        : has_value_(other.has_value_)
    {
        if (has_value_) {
            new (&value_) T(std::move(other.value_));
        } else {
            new (&error_) Error(std::move(other.error_));
        }
    }

    Result& operator=(Result&& other) noexcept(std::is_nothrow_move_constructible_v<T>) {
        if (this != &other) {
            if (has_value_) {
                value_.~T();
            } else {
                error_.~Error();
            }
            has_value_ = other.has_value_;
            if (has_value_) {
                new (&value_) T(std::move(other.value_));
            } else {
                new (&error_) Error(std::move(other.error_));
            }
        }
        return *this;
    }

    bool has_value() const { return has_value_; }
    explicit operator bool() const { return has_value_; }

    T& value() { return value_; }
    const T& value() const { return value_; }

    Error& error() { return error_; }
    const Error& error() const { return error_; }

private:
    bool has_value_;
    union {
        T value_;
        Error error_;
    };
};

template <>
class [[nodiscard]] Result<void> {
public:
    Result() = default;

    Result(Error error)
        : error_{std::move(error)} {}

    bool has_value() const { return !error_; }
    explicit operator bool() const { return has_value(); }

    Error& error() { return error_; }
    const Error& error() const { return error_; }

private:
    Error error_;
};

} // namespace gameak::core
