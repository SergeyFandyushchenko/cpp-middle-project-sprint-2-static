#pragma once

#include <algorithm>
#include <string_view>
#include <tuple>

namespace stdx::details {

// Шаблонный класс, хранящий C-style строку фиксированной длины
template <size_t N>
struct fixed_string {
    static_assert(N > 0, "fixed_string cannot be empty");

    char data[N] = {};

    // Конструктор, принимающий массив того же размера
    constexpr fixed_string(const char (&str)[N]) { std::copy_n(str, N, data); }

    // Конструктор, принимающий массив меньшего или равного размера
    template <size_t M>
    constexpr fixed_string(const char (&str)[M]) {
        static_assert(M <= N, "Source string too long");
        std::copy_n(str, M, data);
        if constexpr (N > M) {
            data[M] = '\0';
        }
    }

    // Конструктор от двух указателей
    constexpr fixed_string(const char *begin, const char *end) {
        size_t len = std::min(static_cast<size_t>(end - begin), N - 1);
        std::copy_n(begin, len, data);
        data[len] = '\0';
    }

    constexpr auto operator<=>(const fixed_string &other) const = default;
    constexpr size_t size() const { return N; }
    constexpr const char *c_str() const { return data; }
    constexpr std::string_view view() const { return std::string_view(data, N - 1); }
};

// Класс для хранения ошибки парсинга
struct parse_error : fixed_string<128> {
    using fixed_string<128>::fixed_string;
};

// Шаблонный класс для хранения результатов парсинга
template <typename... Ts>
struct scan_result {
    std::tuple<Ts...> results;

    constexpr scan_result(Ts &&...args) : results(std::forward<Ts>(args)...) {}

    template <size_t I>
    constexpr auto get() const {
        return std::get<I>(results);
    }

    constexpr const std::tuple<Ts...> &values() const { return results; }
};

}  // namespace stdx::details