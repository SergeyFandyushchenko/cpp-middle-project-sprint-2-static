#pragma once

#include <type_traits>

#include "format_string.hpp"
#include "parse.hpp"
#include "types.hpp"

namespace stdx {

template <details::format_string fmt_str, details::fixed_string source, typename... Ts>
consteval details::scan_result<Ts...> scan() {
    static_assert(fmt_str.number_placeholders == sizeof...(Ts), "Number of placeholders doesn't match number of types");
    static_assert(((details::supported_type<Ts>) && ...), "Unsupported type in parameter pack");

    // Вспомогательная лямбда внутри функции
    constexpr auto helper = []<size_t... Is>(std::index_sequence<Is...>) {
        return details::scan_result<Ts...>{details::parse_input<Is, fmt_str, source, Ts>()...};
    };

    return helper(std::index_sequence_for<Ts...>{});
}

}  // namespace stdx
