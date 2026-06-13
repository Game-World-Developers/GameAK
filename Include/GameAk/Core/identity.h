#pragma once

#include <cstdint>
#include <functional>
#include <string>

namespace gameak::core {

class Identity {
public:
    static Identity invalid() { return Identity{0}; }

    explicit Identity(uint64_t id = 0)
        : id_{id} {}

    uint64_t value() const { return id_; }
    bool is_valid() const { return id_ != 0; }

    bool operator==(const Identity& other) const { return id_ == other.id_; }
    bool operator!=(const Identity& other) const { return id_ != other.id_; }
    bool operator<(const Identity& other) const { return id_ < other.id_; }
    bool operator>(const Identity& other) const { return id_ > other.id_; }
    bool operator<=(const Identity& other) const { return id_ <= other.id_; }
    bool operator>=(const Identity& other) const { return id_ >= other.id_; }

    std::string to_string() const;

private:
    uint64_t id_;
};

} // namespace gameak::core

namespace std {

template <>
struct hash<gameak::core::Identity> {
    size_t operator()(const gameak::core::Identity& id) const noexcept {
        return hash<uint64_t>{}(id.value());
    }
};

} // namespace std
