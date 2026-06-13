#pragma once

#include <cstddef>
#include <functional>
#include <utility>

namespace gameak::core {

template <typename Key, typename Value, typename Compare = std::less<Key>>
class avl_tree {
    struct node {
        std::pair<const Key, Value> kv;
        node* left{nullptr};
        node* right{nullptr};
        node* parent{nullptr};
        int balance{0};

        node(Key k, Value v) : kv(std::move(k), std::move(v)) {}
        const Key& key() const { return kv.first; }
        Value& value() { return kv.second; }
    };

    node* root_{nullptr};
    size_t size_{0};
    Compare comp_;

    node*& child_ptr(node* n, int dir) {
        return dir < 0 ? n->left : n->right;
    }

    int dir_of(node* n) const {
        return n->parent == nullptr ? 0
             : n->parent->left == n ? -1 : 1;
    }

    void rotate(node* n, int dir) {
        auto* child = child_ptr(n, dir);
        if (!child) return;
        auto* grandchild = child_ptr(child, -dir);

        child_ptr(n, dir) = grandchild;
        if (grandchild) grandchild->parent = n;

        child->parent = n->parent;
        if (!n->parent) {
            root_ = child;
        } else {
            child_ptr(n->parent, dir_of(n)) = child;
        }

        child_ptr(child, -dir) = n;
        n->parent = child;

        n->balance = n->balance - dir - (child->balance == dir ? 1 : 0);
        child->balance = child->balance - dir + (n->balance == -dir ? 1 : 0);
    }

    void rebalance(node* n, int d) {
        while (n) {
            n->balance += d;
            if (n->balance == 0) break;

            if (n->balance == -2) {
                if (n->left->balance == -1) {
                    rotate(n, -1);
                } else {
                    rotate(n->left, 1);
                    rotate(n, -1);
                }
                break;
            }
            if (n->balance == 2) {
                if (n->right->balance == 1) {
                    rotate(n, 1);
                } else {
                    rotate(n->right, -1);
                    rotate(n, 1);
                }
                break;
            }

            node* par = n->parent;
            if (!par) break;
            d = dir_of(n);
            n = par;
        }
    }

    void destroy(node* n) {
        if (n) {
            destroy(n->left);
            destroy(n->right);
            delete n;
        }
    }

public:
    class iterator {
        node* n_;
    public:
        using value_type = std::pair<const Key, Value>;
        using reference = value_type&;
        using pointer = value_type*;
        using iterator_category = std::bidirectional_iterator_tag;

        explicit iterator(node* n) : n_(n) {}
        reference operator*() const { return n_->kv; }
        pointer operator->() const { return &n_->kv; }
        iterator& operator++() { n_ = next(); return *this; }
        iterator operator++(int) { auto t = *this; n_ = next(); return t; }
        iterator& operator--() { n_ = prev(); return *this; }
        iterator operator--(int) { auto t = *this; n_ = prev(); return t; }
        bool operator==(const iterator& o) const { return n_ == o.n_; }
        bool operator!=(const iterator& o) const { return n_ != o.n_; }
        node* raw() const { return n_; }

    private:
        node* next() const {
            if (n_->right) {
                auto* r = n_->right;
                while (r->left) r = r->left;
                return r;
            }
            auto* p = n_->parent;
            auto* c = n_;
            while (p && c == p->right) {
                c = p;
                p = p->parent;
            }
            return p;
        }
        node* prev() const {
            if (n_->left) {
                auto* l = n_->left;
                while (l->right) l = l->right;
                return l;
            }
            auto* p = n_->parent;
            auto* c = n_;
            while (p && c == p->left) {
                c = p;
                p = p->parent;
            }
            return p;
        }
    };

    avl_tree() = default;
    ~avl_tree() { destroy(root_); }

    avl_tree(const avl_tree&) = delete;
    avl_tree& operator=(const avl_tree&) = delete;
    avl_tree(avl_tree&& other) noexcept
        : root_(other.root_), size_(other.size_) {
        other.root_ = nullptr;
        other.size_ = 0;
    }
    avl_tree& operator=(avl_tree&& other) noexcept {
        if (this != &other) {
            destroy(root_);
            root_ = other.root_;
            size_ = other.size_;
            other.root_ = nullptr;
            other.size_ = 0;
        }
        return *this;
    }

    iterator insert(Key key, Value value) {
        if (!root_) {
            root_ = new node(std::move(key), std::move(value));
            size_ = 1;
            return iterator(root_);
        }

        node* cur = root_;
        node* par = nullptr;
        int last_dir = 0;

        while (cur) {
            par = cur;
            if (comp_(key, cur->key())) {
                cur = cur->left;
                last_dir = -1;
            } else if (comp_(cur->key(), key)) {
                cur = cur->right;
                last_dir = 1;
            } else {
                cur->value() = std::move(value);
                return iterator(cur);
            }
        }

        auto* nn = new node(std::move(key), std::move(value));
        nn->parent = par;
        child_ptr(par, last_dir) = nn;
        size_++;

        rebalance(par, last_dir);

        while (root_->parent) root_ = root_->parent;
        return iterator(nn);
    }

    iterator find(const Key& key) const {
        node* cur = root_;
        while (cur) {
            if (comp_(key, cur->key())) {
                cur = cur->left;
            } else if (comp_(cur->key(), key)) {
                cur = cur->right;
            } else {
                return iterator(cur);
            }
        }
        return end();
    }

    bool contains(const Key& key) const { return find(key) != end(); }

    size_t size() const { return size_; }
    bool empty() const { return size_ == 0; }

    iterator begin() const {
        if (!root_) return end();
        node* cur = root_;
        while (cur->left) cur = cur->left;
        return iterator(cur);
    }

    iterator end() const { return iterator(nullptr); }
};

} // namespace gameak::core
