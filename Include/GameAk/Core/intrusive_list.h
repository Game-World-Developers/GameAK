#pragma once

#include <cstddef>
#include <type_traits>

namespace gameak::core {

struct intrusive_node {
    intrusive_node* next{nullptr};
    intrusive_node* prev{nullptr};
};

template <typename T>
class intrusive_list {
    static_assert(std::is_base_of_v<intrusive_node, T>);

    intrusive_node head_;
    size_t size_{0};

    void link_after(intrusive_node* pos, T* node) {
        node->next = pos->next;
        node->prev = pos;
        pos->next->prev = node;
        pos->next = node;
        ++size_;
    }

    void unlink(T* node) {
        node->prev->next = node->next;
        node->next->prev = node->prev;
        node->next = nullptr;
        node->prev = nullptr;
        --size_;
    }

public:
    class const_iterator;

    class iterator {
        intrusive_node* node_;
    public:
        using value_type = T;
        using reference = T&;
        using pointer = T*;
        using iterator_category = std::bidirectional_iterator_tag;

        explicit iterator(intrusive_node* n) : node_(n) {}
        T& operator*() const { return static_cast<T&>(*node_); }
        T* operator->() const { return static_cast<T*>(node_); }
        iterator& operator++() { node_ = node_->next; return *this; }
        iterator operator++(int) { auto tmp = *this; node_ = node_->next; return tmp; }
        iterator& operator--() { node_ = node_->prev; return *this; }
        iterator operator--(int) { auto tmp = *this; node_ = node_->prev; return tmp; }
        bool operator==(const iterator& o) const { return node_ == o.node_; }
        bool operator!=(const iterator& o) const { return node_ != o.node_; }
        operator const_iterator() const { return const_iterator(node_); }
    };

    class const_iterator {
        const intrusive_node* node_;
    public:
        using value_type = const T;
        using reference = const T&;
        using pointer = const T*;
        using iterator_category = std::bidirectional_iterator_tag;

        explicit const_iterator(const intrusive_node* n) : node_(n) {}
        const T& operator*() const { return static_cast<const T&>(*node_); }
        const T* operator->() const { return static_cast<const T*>(node_); }
        const_iterator& operator++() { node_ = node_->next; return *this; }
        const_iterator operator++(int) { auto tmp = *this; node_ = node_->next; return tmp; }
        const_iterator& operator--() { node_ = node_->prev; return *this; }
        const_iterator operator--(int) { auto tmp = *this; node_ = node_->prev; return tmp; }
        bool operator==(const const_iterator& o) const { return node_ == o.node_; }
        bool operator!=(const const_iterator& o) const { return node_ != o.node_; }
    };

    intrusive_list() { head_.next = &head_; head_.prev = &head_; }
    ~intrusive_list() { clear(); }

    intrusive_list(const intrusive_list&) = delete;
    intrusive_list& operator=(const intrusive_list&) = delete;
    intrusive_list(intrusive_list&& other) noexcept { move_from(other); }
    intrusive_list& operator=(intrusive_list&& other) noexcept {
        if (this != &other) { clear(); move_from(other); }
        return *this;
    }

    void push_front(T* node) { link_after(&head_, node); }
    void push_back(T* node) { link_after(head_.prev, node); }
    void pop_front() { if (!empty()) { unlink(static_cast<T*>(head_.next)); } }
    void pop_back() { if (!empty()) { unlink(static_cast<T*>(head_.prev)); } }

    iterator insert(const_iterator pos, T* node) {
        auto* p = const_cast<intrusive_node*>(&*pos);
        link_after(p->prev, node);
        return iterator(node);
    }

    iterator erase(const_iterator pos) {
        auto* node = const_cast<T*>(&*pos);
        auto* next = node->next;
        unlink(node);
        return iterator(next);
    }

    void clear() {
        while (!empty()) {
            pop_front();
        }
    }

    T& front() { return static_cast<T&>(*head_.next); }
    const T& front() const { return static_cast<const T&>(*head_.next); }
    T& back() { return static_cast<T&>(*head_.prev); }
    const T& back() const { return static_cast<const T&>(*head_.prev); }

    bool empty() const { return size_ == 0; }
    size_t size() const { return size_; }

    iterator begin() { return iterator(head_.next); }
    const_iterator begin() const { return const_iterator(head_.next); }
    iterator end() { return iterator(&head_); }
    const_iterator end() const { return const_iterator(&head_); }

private:
    void move_from(intrusive_list& other) {
        head_.next = &head_;
        head_.prev = &head_;
        size_ = 0;
        if (!other.empty()) {
            auto* first = other.head_.next;
            auto* last = other.head_.prev;
            head_.next = first;
            head_.prev = last;
            first->prev = &head_;
            last->next = &head_;
            size_ = other.size_;
            other.head_.next = &other.head_;
            other.head_.prev = &other.head_;
            other.size_ = 0;
        }
    }
};

} // namespace gameak::core
