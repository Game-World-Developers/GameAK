#pragma once

#include <AK/Core/Types.hpp>
#include <AK/Data/LayoutPolicy.hpp>

namespace GameAK::Data {

template <typename... Components>
class ElementRef {
    void* m_ptrs[sizeof...(Components)];

public:
    constexpr ElementRef() noexcept
        : m_ptrs{} {}

    constexpr ElementRef(Components*... ptrs) noexcept
        : m_ptrs{ static_cast<void*>(ptrs)... } {}

    template <typename T>
    [[nodiscard]] T& get() noexcept {
        return *static_cast<T*>(m_ptrs[IndexOfV<T, Components...>]);
    }

    template <typename T>
    [[nodiscard]] const T& get() const noexcept {
        return *static_cast<const T*>(m_ptrs[IndexOfV<T, Components...>]);
    }
};

} // namespace GameAK::Data
