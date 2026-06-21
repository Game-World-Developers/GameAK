#pragma once

#include "layout_strategy.h"
#include "GameAk/Core/semantic.h"

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace gameak::runtime {

struct FieldDescriptor {
    std::string name;
    size_t offset;
    size_t size;
    size_t alignment;

    FieldDescriptor() = default;
    FieldDescriptor(const char* n, size_t o, size_t s, size_t a)
        : name{n}, offset{o}, size{s}, alignment{a} {}
    FieldDescriptor(std::string n, size_t o, size_t s, size_t a)
        : name{std::move(n)}, offset{o}, size{s}, alignment{a} {}
};

struct BlockTypeDescriptor {
    uint32_t type_id;
    size_t size;
    size_t alignment;
    std::string name;
    LayoutStrategy layout{LayoutStrategy::AoS};
    AoSoAConfig aosoa_config{};
    std::vector<FieldDescriptor> fields;
    std::unordered_map<std::string, size_t> field_index;  // field name → index in fields[]
    std::unordered_map<size_t, size_t> offset_index;     // field offset → index in fields[]
    bool ephemeral{false};
    const core::SemanticConstraint* semantic{nullptr};
};

} // namespace gameak::runtime
