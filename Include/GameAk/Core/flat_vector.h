#pragma once

#include <algorithm>
#include <cstddef>
#include <cstring>
#include <memory>
#include <type_traits>
#include <utility>

namespace gameak::core {

template <typename T, size_t InlineN = 8>
class flat_vector {
    static_assert(InlineN > 0, "InlineN must be at least 1");

    static constexpr size_t align_ = alignof(T);

    alignas(align_) unsigned char inline_store_[sizeof(T) * InlineN];

    T* begin_;
    T* end_;
    T* capacity_;

    T* inline_ptr() { return reinterpret_cast<T*>(inline_store_); }
    const T* inline_ptr() const { return reinterpret_cast<const T*>(inline_store_); }

    bool is_inline() const { return begin_ == inline_ptr(); }
    size_t cap() const { return static_cast<size_t>(capacity_ - begin_); }
    size_t used() const { return static_cast<size_t>(end_ - begin_); }

    void grow(size_t needed) {
        size_t new_cap = std::max(cap() * 2, needed);
        auto* new_begin = static_cast<T*>(std::malloc(new_cap * sizeof(T)));
        size_t old_count = used();
        if constexpr (std::is_trivially_copyable_v<T>) {
            std::memcpy(new_begin, begin_, old_count * sizeof(T));
        } else {
            for (size_t i = 0; i < old_count; ++i) {
                new (&new_begin[i]) T(std::move(begin_[i]));
                begin_[i].~T();
            }
        }
        if (!is_inline()) {
            std::free(begin_);
        }
        begin_ = new_begin;
        end_ = new_begin + old_count;
        capacity_ = new_begin + new_cap;
    }

public:
    using value_type = T;
    using iterator = T*;
    using const_iterator = const T*;
    using reference = T&;
    using const_reference = const T&;

    flat_vector() noexcept
        : begin_(inline_ptr()), end_(inline_ptr()), capacity_(inline_ptr() + InlineN) {}

    flat_vector(const flat_vector& other) : flat_vector() {
        reserve(other.size());
        for (auto& v : other) {
            push_back(v);
        }
    }

    flat_vector(flat_vector&& other) noexcept
        : flat_vector() {
        if (!other.is_inline()) {
            begin_ = other.begin_;
            end_ = other.end_;
            capacity_ = other.capacity_;
            other.begin_ = other.inline_ptr();
            other.end_ = other.inline_ptr();
            other.capacity_ = other.inline_ptr() + InlineN;
        } else {
            for (auto& v : other) {
                push_back(std::move(v));
                v.~T();
            }
            other.end_ = other.begin_;
        }
    }

    flat_vector& operator=(const flat_vector& other) {
        if (this != &other) {
            clear();
            reserve(other.size());
            for (auto& v : other) {
                push_back(v);
            }
        }
        return *this;
    }

    flat_vector& operator=(flat_vector&& other) noexcept {
        if (this != &other) {
            clear();
            if (!is_inline()) {
                std::free(begin_);
            }
            if (!other.is_inline()) {
                begin_ = other.begin_;
                end_ = other.end_;
                capacity_ = other.capacity_;
                other.begin_ = other.inline_ptr();
                other.end_ = other.inline_ptr();
                other.capacity_ = other.inline_ptr() + InlineN;
            } else {
                begin_ = inline_ptr();
                end_ = begin_;
                capacity_ = inline_ptr() + InlineN;
                for (auto& v : other) {
                    push_back(std::move(v));
                    v.~T();
                }
                other.end_ = other.begin_;
            }
        }
        return *this;
    }

    ~flat_vector() {
        clear();
        if (!is_inline()) {
            std::free(begin_);
        }
    }

    void push_back(const T& value) {
        if (used() == cap()) { grow(cap() + 1); }
        new (end_) T(value);
        ++end_;
    }

    void push_back(T&& value) {
        if (used() == cap()) { grow(cap() + 1); }
        new (end_) T(std::move(value));
        ++end_;
    }

    template <typename... Args>
    T& emplace_back(Args&&... args) {
        if (used() == cap()) { grow(cap() + 1); }
        auto* ptr = new (end_) T(std::forward<Args>(args)...);
        ++end_;
        return *ptr;
    }

    void pop_back() {
        --end_;
        end_->~T();
    }

    void clear() {
        for (auto& v : *this) {
            v.~T();
        }
        end_ = begin_;
    }

    void reserve(size_t n) {
        if (n > cap()) { grow(n); }
    }

    void resize(size_t n) {
        if (n < used()) {
            for (auto it = begin_ + n; it != end_; ++it) { it->~T(); }
            end_ = begin_ + n;
        } else if (n > used()) {
            reserve(n);
            for (auto it = end_; it != begin_ + n; ++it) { new (it) T(); }
            end_ = begin_ + n;
        }
    }

    size_t size() const { return used(); }
    size_t capacity() const { return cap(); }
    bool empty() const { return begin_ == end_; }

    T* data() { return begin_; }
    const T* data() const { return begin_; }

    iterator begin() { return begin_; }
    const_iterator begin() const { return begin_; }
    iterator end() { return end_; }
    const_iterator end() const { return end_; }

    reference operator[](size_t i) { return begin_[i]; }
    const_reference operator[](size_t i) const { return begin_[i]; }

    reference at(size_t i) { return begin_[i]; }
    const_reference at(size_t i) const { return begin_[i]; }

    reference front() { return begin_[0]; }
    const_reference front() const { return begin_[0]; }
    reference back() { return end_[-1]; }
    const_reference back() const { return end_[-1]; }

    iterator erase(const_iterator pos) {
        auto* p = const_cast<T*>(pos);
        p->~T();
        std::move(p + 1, end_, p);
        --end_;
        return p;
    }

    iterator erase(const_iterator first, const_iterator last) {
        auto* f = const_cast<T*>(first);
        auto* l = const_cast<T*>(last);
        size_t n = static_cast<size_t>(l - f);
        for (auto* it = f; it != l; ++it) { it->~T(); }
        std::move(l, end_, f);
        end_ -= n;
        return f;
    }
};

} // namespace gameak::core
