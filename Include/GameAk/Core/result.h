#pragma once

#include "error.h"

#include <type_traits>
#include <utility>

namespace gameak::core {

template <typename T>
class [[nodiscard]] Result;

template <typename> struct is_result_impl : std::false_type {};
template <typename U> struct is_result_impl<Result<U>> : std::true_type {};
template <typename T> constexpr bool is_result_v = is_result_impl<std::decay_t<T>>::value;

template <typename F, typename Arg>
using and_then_return_t = std::conditional_t<
    is_result_v<decltype(std::declval<F>()(std::declval<Arg>()))>,
    decltype(std::declval<F>()(std::declval<Arg>())),
    Result<decltype(std::declval<F>()(std::declval<Arg>()))>
>;

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

    // ── Monadic chaining ──────────────────────────────────────────

    template <typename F>
    auto and_then(F&& f) && -> and_then_return_t<F, T> {
        if (!has_value_) return std::move(error_);
        return f(std::move(value_));
    }

    template <typename F>
    auto and_then(F&& f) const& -> and_then_return_t<F, const T&> {
        if (!has_value_) return error_;
        return f(value_);
    }

    template <typename F>
    void or_else(F&& f) const& {
        if (!has_value_) f(error_);
    }

private:
    bool has_value_;
    union {
        T value_;
        Error error_;
    };
};

template <typename F>
using and_then_void_return_t = std::conditional_t<
    is_result_v<decltype(std::declval<F>()())>,
    decltype(std::declval<F>()()),
    Result<decltype(std::declval<F>()())>
>;

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

    // ── Monadic chaining ──────────────────────────────────────────

    template <typename F>
    auto and_then(F&& f) && -> and_then_void_return_t<F> {
        if (error_) return std::move(error_);
        return f();
    }

    template <typename F>
    auto and_then(F&& f) const& -> and_then_void_return_t<F> {
        if (error_) return error_;
        return f();
    }

    template <typename F>
    void or_else(F&& f) const& {
        if (error_) f(error_);
    }

private:
    Error error_;
};

// ── Helper: return a successful Result<void> ─────────────────────

inline Result<void> ok() { return {}; }

} // namespace gameak::core

// ── TRY macros (early-return on error, like Rust's ?) ─────────────

#define GAME_AK_TRY(var, expr) \
    if (auto _game_ak_r_ = (expr); !_game_ak_r_) { return _game_ak_r_.error(); } \
    else { var = std::move(_game_ak_r_.value()); }

#define GAME_AK_TRY_VOID(expr) \
    if (auto _game_ak_r_ = (expr); !_game_ak_r_) { return _game_ak_r_.error(); }

// Short aliases (SPEC-029)
#define TRY(var, expr)          GAME_AK_TRY(var, expr)
#define TRY_VOID(expr)          GAME_AK_TRY_VOID(expr)
