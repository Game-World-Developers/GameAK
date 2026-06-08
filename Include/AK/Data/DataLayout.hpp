#pragma once

#include <AK/Backend/ExecutionProfile.hpp>
#include <AK/Core/Macros.hpp>
#include <AK/Core/TypeTraits.hpp>
#include <AK/Core/Types.hpp>
#include <AK/Data/ElementRef.hpp>
#include <AK/Data/LayoutPolicy.hpp>

#include <new>

namespace GameAK::Data {

// ---------------------------------------------------------------------------
// Span — lightweight non-owning view
// ---------------------------------------------------------------------------

template <typename T>
struct Span {
    T    *data;
    usize count;

    T       &operator[](usize i) noexcept { return data[i]; }
    const T &operator[](usize i) const noexcept { return data[i]; }

    T       *begin() noexcept { return data; }
    T       *end() noexcept { return data + count; }
    const T *begin() const noexcept { return data; }
    const T *end() const noexcept { return data + count; }

    [[nodiscard]] usize size() const noexcept { return count; }
    [[nodiscard]] bool empty() const noexcept { return count == 0; }
};

// ---------------------------------------------------------------------------
// Internal helpers
// ---------------------------------------------------------------------------

namespace detail {

template <typename T, typename First, typename... Rest>
constexpr usize aos_offset() noexcept {
    if constexpr (std::is_same_v<T, First>) {
        return 0;
    } else {
        return sizeof(First) + aos_offset<T, Rest...>();
    }
}

template <typename... Cs>
constexpr usize aos_stride() noexcept {
    return (sizeof(Cs) + ...);
}

template <typename T, typename... Cs>
inline constexpr usize kAOSOffset = aos_offset<T, Cs...>();

template <typename... Cs>
inline constexpr usize kAOSStride = aos_stride<Cs...>();

constexpr usize align_up(usize val, usize alignment) noexcept {
    return (val + alignment - 1) & ~(alignment - 1);
}

} // namespace detail

// ===========================================================================
// AOS — Array of Structures
// ===========================================================================

template <typename... Components>
class DataLayout<AOS<Components...>> {
    using ComponentList = TypeList<Components...>;
    static constexpr usize kStride = detail::kAOSStride<Components...>;

    u8    *m_buffer;
    usize  m_capacity_bytes;
    usize  m_count;

public:
    DataLayout(void *buffer, usize capacity_bytes) noexcept
        : m_buffer{static_cast<u8 *>(buffer)},
          m_capacity_bytes{capacity_bytes},
          m_count{0} {}

    void reset(void *buffer, usize capacity_bytes) noexcept {
        m_buffer         = static_cast<u8 *>(buffer);
        m_capacity_bytes = capacity_bytes;
        m_count          = 0;
    }

    [[nodiscard]] usize max_elements() const noexcept {
        return m_capacity_bytes / kStride;
    }

    [[nodiscard]] usize size() const noexcept { return m_count; }
    [[nodiscard]] bool empty() const noexcept { return m_count == 0; }

    void resize(usize n) noexcept {
        m_count = (n < max_elements()) ? n : max_elements();
    }

    void clear() noexcept { m_count = 0; }

    [[nodiscard]] ElementRef<Components...> operator[](usize i) noexcept {
        return make_ref(m_buffer + i * kStride);
    }

    [[nodiscard]] const ElementRef<Components...> operator[](usize i) const noexcept {
        return make_ref(const_cast<u8 *>(m_buffer + i * kStride));
    }

    class Iterator {
        DataLayout *m_layout;
        usize       m_index;
    public:
        using value_type = ElementRef<Components...>;
        Iterator(DataLayout *layout, usize index) noexcept : m_layout{layout}, m_index{index} {}
        value_type operator*() noexcept { return (*m_layout)[m_index]; }
        Iterator &operator++() noexcept { ++m_index; return *this; }
        bool operator!=(const Iterator &o) const noexcept { return m_index != o.m_index; }
    };

    Iterator begin() noexcept { return Iterator{this, 0}; }
    Iterator end() noexcept { return Iterator{this, m_count}; }

private:
    template <typename T>
    static T *ptr_in_element(u8 *base) noexcept {
        return reinterpret_cast<T *>(base + detail::kAOSOffset<T, Components...>);
    }

    ElementRef<Components...> make_ref(u8 *base) noexcept {
        return ElementRef<Components...>(ptr_in_element<Components>(base)...);
    }

    const ElementRef<Components...> make_ref(u8 *base) const noexcept {
        return ElementRef<Components...>(ptr_in_element<Components>(base)...);
    }
};

// ===========================================================================
// SOA — Structure of Arrays
// ===========================================================================

template <typename... Components>
class DataLayout<SOA<Components...>> {
    using ComponentList = TypeList<Components...>;
    static constexpr usize kCount  = sizeof...(Components);
    static constexpr usize kStride = detail::kAOSStride<Components...>;

    u8   *m_buffer;
    usize m_capacity_bytes;
    usize m_max_elements;
    usize m_count;
    u8   *m_arrays[kCount];

    void compute_arrays() noexcept {
        usize offset = 0;
        ((m_arrays[IndexOfV<Components, Components...>] = m_buffer + offset,
          offset += detail::align_up(m_max_elements * sizeof(Components), alignof(Components))),
         ...);
    }

public:
    DataLayout(void *buffer, usize capacity_bytes) noexcept
        : m_buffer{static_cast<u8 *>(buffer)},
          m_capacity_bytes{capacity_bytes},
          m_max_elements{capacity_bytes / kStride},
          m_count{0} {
        compute_arrays();
    }

    void reset(void *buffer, usize capacity_bytes) noexcept {
        m_buffer         = static_cast<u8 *>(buffer);
        m_capacity_bytes = capacity_bytes;
        m_max_elements   = capacity_bytes / kStride;
        m_count          = 0;
        compute_arrays();
    }

    [[nodiscard]] usize max_elements() const noexcept { return m_max_elements; }
    [[nodiscard]] usize size() const noexcept { return m_count; }
    [[nodiscard]] bool empty() const noexcept { return m_count == 0; }

    void resize(usize n) noexcept {
        m_count = (n < m_max_elements) ? n : m_max_elements;
    }

    void clear() noexcept { m_count = 0; }

    template <typename T>
    [[nodiscard]] Span<T> slice() noexcept {
        constexpr usize idx = IndexOfV<T, Components...>;
        return Span<T>{reinterpret_cast<T *>(m_arrays[idx]), m_count};
    }

    template <typename T>
    [[nodiscard]] Span<const T> slice() const noexcept {
        constexpr usize idx = IndexOfV<T, Components...>;
        return Span<const T>{reinterpret_cast<const T *>(m_arrays[idx]), m_count};
    }

    [[nodiscard]] ElementRef<Components...> operator[](usize i) noexcept {
        return make_ref(i);
    }

    [[nodiscard]] const ElementRef<Components...> operator[](usize i) const noexcept {
        return make_ref(i);
    }

    class Iterator {
        DataLayout *m_layout;
        usize       m_index;
    public:
        using value_type = ElementRef<Components...>;
        Iterator(DataLayout *layout, usize index) noexcept : m_layout{layout}, m_index{index} {}
        value_type operator*() noexcept { return (*m_layout)[m_index]; }
        Iterator &operator++() noexcept { ++m_index; return *this; }
        bool operator!=(const Iterator &o) const noexcept { return m_index != o.m_index; }
    };

    Iterator begin() noexcept { return Iterator{this, 0}; }
    Iterator end() noexcept { return Iterator{this, m_count}; }

private:
    ElementRef<Components...> make_ref(usize i) noexcept {
        return ElementRef<Components...>(
            reinterpret_cast<Components *>(m_arrays[IndexOfV<Components, Components...>] + i * sizeof(Components))...);
    }

    const ElementRef<Components...> make_ref(usize i) const noexcept {
        return ElementRef<Components...>(
            reinterpret_cast<Components *>(
                const_cast<u8 *>(m_arrays[IndexOfV<Components, Components...>] + i * sizeof(Components)))...);
    }
};

// ===========================================================================
// AOSOA — Array of Structures of Arrays
// ===========================================================================

template <typename... Components>
class DataLayout<AOSOA<Components...>> {
    using ComponentList = TypeList<Components...>;
    static constexpr usize kCount = sizeof...(Components);

    u8    *m_buffer;
    usize  m_capacity_bytes;
    usize  m_chunk_size;
    usize  m_max_elems;
    usize  m_chunk_stride;
    usize  m_chunk_count;
    usize  m_count;

    void init_chunk(usize chunk_size) noexcept {
        m_chunk_size   = (chunk_size > 0) ? chunk_size : 1;
        m_chunk_stride = 0;
        ((m_chunk_stride += detail::align_up(m_chunk_size * sizeof(Components), alignof(Components))), ...);
        m_chunk_count  = (m_chunk_stride > 0) ? (m_capacity_bytes / m_chunk_stride) : 0;
        m_max_elems    = m_chunk_count * m_chunk_size;
        m_count        = 0;
    }

    template <typename T>
    usize chunk_offset() const noexcept {
        usize off = 0;
        bool found = false;
        auto step = [&]<typename U>() {
            if constexpr (std::is_same_v<T, U>) {
                found = true;
            } else if (!found) {
                off += detail::align_up(m_chunk_size * sizeof(U), alignof(U));
            }
        };
        (step.template operator()<Components>(), ...);
        return off;
    }

public:
    DataLayout(void *buffer, usize capacity_bytes, usize chunk_size) noexcept
        : m_buffer{static_cast<u8 *>(buffer)},
          m_capacity_bytes{capacity_bytes} {
        init_chunk(chunk_size);
    }

    DataLayout(void *buffer, usize capacity_bytes,
               const Backend::ExecutionProfile &profile) noexcept
        : DataLayout(buffer, capacity_bytes,
                     (profile.simd_width > 0) ? static_cast<usize>(profile.simd_width) : 8) {}

    void reset(void *buffer, usize capacity_bytes, usize chunk_size) noexcept {
        m_buffer         = static_cast<u8 *>(buffer);
        m_capacity_bytes = capacity_bytes;
        init_chunk(chunk_size);
    }

    [[nodiscard]] usize chunk_size() const noexcept { return m_chunk_size; }
    [[nodiscard]] usize max_elements() const noexcept { return m_max_elems; }
    [[nodiscard]] usize chunk_count() const noexcept { return m_chunk_count; }
    [[nodiscard]] usize size() const noexcept { return m_count; }
    [[nodiscard]] bool empty() const noexcept { return m_count == 0; }

    void resize(usize n) noexcept {
        m_count = (n < m_max_elems) ? n : m_max_elems;
    }

    void clear() noexcept { m_count = 0; }

    [[nodiscard]] ElementRef<Components...> operator[](usize i) noexcept {
        return make_ref(i);
    }

    [[nodiscard]] const ElementRef<Components...> operator[](usize i) const noexcept {
        return make_ref(i);
    }

    template <typename T>
    [[nodiscard]] Span<T> chunk_slice(usize chunk_idx) noexcept {
        u8 *base  = m_buffer + chunk_idx * m_chunk_stride;
        T  *arr   = reinterpret_cast<T *>(base + chunk_offset<T>());
        usize start = chunk_idx * m_chunk_size;
        usize end   = start + m_chunk_size;
        usize n     = (end <= m_count) ? m_chunk_size
                     : (start < m_count) ? (m_count - start)
                                         : 0;
        return Span<T>{arr, n};
    }

    template <typename T>
    [[nodiscard]] Span<const T> chunk_slice(usize chunk_idx) const noexcept {
        const u8 *base = m_buffer + chunk_idx * m_chunk_stride;
        const T  *arr  = reinterpret_cast<const T *>(base + chunk_offset<T>());
        usize start = chunk_idx * m_chunk_size;
        usize end   = start + m_chunk_size;
        usize n     = (end <= m_count) ? m_chunk_size
                     : (start < m_count) ? (m_count - start)
                                         : 0;
        return Span<const T>{arr, n};
    }

    class Iterator {
        DataLayout *m_layout;
        usize       m_index;
    public:
        using value_type = ElementRef<Components...>;
        Iterator(DataLayout *layout, usize index) noexcept : m_layout{layout}, m_index{index} {}
        value_type operator*() noexcept { return (*m_layout)[m_index]; }
        Iterator &operator++() noexcept { ++m_index; return *this; }
        bool operator!=(const Iterator &o) const noexcept { return m_index != o.m_index; }
    };

    Iterator begin() noexcept { return Iterator{this, 0}; }
    Iterator end() noexcept { return Iterator{this, m_count}; }

private:
    template <typename T>
    T *chunk_elem_ptr(u8 *chunk_base, usize local) noexcept {
        return reinterpret_cast<T *>(chunk_base + chunk_offset<T>()) + local;
    }

    ElementRef<Components...> make_ref(usize i) noexcept {
        usize chunk_idx = i / m_chunk_size;
        usize local     = i % m_chunk_size;
        u8 *base        = m_buffer + chunk_idx * m_chunk_stride;
        return ElementRef<Components...>(chunk_elem_ptr<Components>(base, local)...);
    }

    const ElementRef<Components...> make_ref(usize i) const noexcept {
        usize chunk_idx     = i / m_chunk_size;
        usize local         = i % m_chunk_size;
        u8 *base            = const_cast<u8 *>(m_buffer + chunk_idx * m_chunk_stride);
        return ElementRef<Components...>(chunk_elem_ptr<Components>(base, local)...);
    }
};

} // namespace GameAK::Data
