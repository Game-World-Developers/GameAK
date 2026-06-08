#pragma once

#include <AK/Core/TypeTraits.hpp>
#include <AK/Core/Types.hpp>

namespace GameAK::Data {

template <typename Policy>
class DataLayout;

} // namespace GameAK::Data

namespace GameAK::Data {

template <typename... Components>
struct AOS {};

template <typename... Components>
struct SOA {};

template <typename... Components>
struct AOSOA {};

template <typename... Ts>
struct TypeList {
    static constexpr usize size = sizeof...(Ts);
};

template <typename T, typename List>
struct IndexOf;

template <typename T, typename First, typename... Rest>
struct IndexOf<T, TypeList<First, Rest...>>
    : std::integral_constant<usize, std::is_same_v<T, First>
                                        ? 0
                                        : 1 + IndexOf<T, TypeList<Rest...>>::value> {};

template <typename T>
struct IndexOf<T, TypeList<>>
    : std::integral_constant<usize, static_cast<usize>(-1)> {};

template <typename T, typename... Ts>
inline constexpr usize IndexOfV = IndexOf<T, TypeList<Ts...>>::value;

template <typename Policy>
struct LayoutTraits;

template <typename... Cs>
struct LayoutTraits<AOS<Cs...>> {
    using ComponentList = TypeList<Cs...>;
    static constexpr bool kIsAOS   = true;
    static constexpr bool kIsSOA   = false;
    static constexpr bool kIsAOSOA = false;
};

template <typename... Cs>
struct LayoutTraits<SOA<Cs...>> {
    using ComponentList = TypeList<Cs...>;
    static constexpr bool kIsAOS   = false;
    static constexpr bool kIsSOA   = true;
    static constexpr bool kIsAOSOA = false;
};

template <typename... Cs>
struct LayoutTraits<AOSOA<Cs...>> {
    using ComponentList = TypeList<Cs...>;
    static constexpr bool kIsAOS   = false;
    static constexpr bool kIsSOA   = false;
    static constexpr bool kIsAOSOA = true;
};

} // namespace GameAK::Data
