#pragma once

#include <cstddef>
#include <functional>
#include <utility>

namespace gameak::core {

template <typename Key, typename Value, typename Compare = std::less<Key>>
class rb_tree {
    enum class color { RED, BLACK };

    struct node {
        std::pair<const Key, Value> kv;
        node* left{nullptr};
        node* right{nullptr};
        node* parent{nullptr};
        color col{color::RED};

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

    node* sibling(node* n) const {
        if (!n->parent) return nullptr;
        return dir_of(n) < 0 ? n->parent->right : n->parent->left;
    }

    node* uncle(node* n) const {
        return sibling(n->parent);
    }

    node* grandparent(node* n) const {
        return n->parent ? n->parent->parent : nullptr;
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
    }

    void fix_insert(node* n) {
        while (n->parent && n->parent->col == color::RED) {
            auto* u = uncle(n);
            auto* g = grandparent(n);
            if (u && u->col == color::RED) {
                n->parent->col = color::BLACK;
                u->col = color::BLACK;
                g->col = color::RED;
                n = g;
            } else {
                int d = dir_of(n);
                int pd = dir_of(n->parent);
                if (d != pd) {
                    rotate(n->parent, d);
                    n = child_ptr(n, -d);
                    pd = dir_of(n->parent);
                }
                rotate(g, pd);
                n->parent->col = color::BLACK;
                g->col = color::RED;
            }
        }
        root_->col = color::BLACK;
    }

    void fix_erase(node* child, node* parent, int dir) {
        while (parent) {
            if (child && child->col == color::RED) {
                child->col = color::BLACK;
                return;
            }

            node* sib = dir < 0 ? parent->right : parent->left;

            if (sib->col == color::RED) {
                parent->col = color::RED;
                sib->col = color::BLACK;
                rotate(parent, dir < 0 ? 1 : -1);
                sib = dir < 0 ? parent->right : parent->left;
            }

            if ((!sib->left || sib->left->col == color::BLACK) &&
                (!sib->right || sib->right->col == color::BLACK)) {
                sib->col = color::RED;
                child = parent;
                parent = parent->parent;
                dir = parent ? dir_of(child) : 0;
            } else {
                if (dir < 0) {
                    if (!sib->right || sib->right->col == color::BLACK) {
                        sib->left->col = color::BLACK;
                        sib->col = color::RED;
                        rotate(sib, -1);
                        sib = parent->right;
                    }
                    sib->col = parent->col;
                    parent->col = color::BLACK;
                    sib->right->col = color::BLACK;
                    rotate(parent, 1);
                } else {
                    if (!sib->left || sib->left->col == color::BLACK) {
                        sib->right->col = color::BLACK;
                        sib->col = color::RED;
                        rotate(sib, 1);
                        sib = parent->left;
                    }
                    sib->col = parent->col;
                    parent->col = color::BLACK;
                    sib->left->col = color::BLACK;
                    rotate(parent, -1);
                }
                return;
            }
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

    rb_tree() = default;
    ~rb_tree() { destroy(root_); }

    rb_tree(const rb_tree&) = delete;
    rb_tree& operator=(const rb_tree&) = delete;
    rb_tree(rb_tree&& other) noexcept
        : root_(other.root_), size_(other.size_) {
        other.root_ = nullptr;
        other.size_ = 0;
    }
    rb_tree& operator=(rb_tree&& other) noexcept {
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
            root_->col = color::BLACK;
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

        fix_insert(nn);
        return iterator(nn);
    }

    void erase(const Key& key) {
        node* target = root_;
        while (target) {
            if (comp_(key, target->key())) {
                target = target->left;
            } else if (comp_(target->key(), key)) {
                target = target->right;
            } else {
                break;
            }
        }
        if (!target) return;

        if (target->left && target->right) {
            node* succ = target->right;
            while (succ->left) succ = succ->left;
            const_cast<Key&>(target->kv.first) = succ->key();
            target->kv.second = std::move(succ->kv.second);
            target = succ;
        }

        node* child = target->left ? target->left : target->right;
        node* parent = target->parent;
        int del_dir = parent ? dir_of(target) : 0;
        bool removed_black = target->col == color::BLACK;

        if (child) child->parent = parent;
        if (!parent) {
            root_ = child;
        } else {
            child_ptr(parent, del_dir) = child;
        }

        size_--;
        delete target;

        if (removed_black) {
            if (child && child->col == color::RED) {
                child->col = color::BLACK;
            } else {
                fix_erase(child, parent, del_dir);
            }
        }

        if (root_) root_->col = color::BLACK;
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

    bool check_invariants() const {
        if (!root_) return true;
        if (root_->col != color::BLACK) return false;
        int expected = -1;
        return check_rb(root_, 0, expected);
    }

private:
    bool check_rb(const node* n, int black_count, int& expected) const {
        if (!n) {
            if (expected == -1) { expected = black_count; return true; }
            return black_count == expected;
        }
        if (n->col == color::BLACK) black_count++;
        if (n->col == color::RED) {
            if (n->left && n->left->col == color::RED) return false;
            if (n->right && n->right->col == color::RED) return false;
        }
        return check_rb(n->left, black_count, expected) &&
               check_rb(n->right, black_count, expected);
    }

public:
};

} // namespace gameak::core
