#pragma once

#include <array>
#include <expected>
#include <utility>

#include "types.hpp"

namespace stdx::details {

// Шаблонный класс для хранения форматирующей строчки и ее особенностей
template <fixed_string fmt_str>
struct format_string {
    static constexpr auto fmt = fmt_str;

    // Функция для получения количества плейсхолдеров
    static consteval std::expected<size_t, parse_error> get_number_placeholders();
    // Функция для получения позиций плейсхолдеров
    static consteval auto get_placeholder_positions();

    static constexpr size_t number_placeholders = []() consteval {
        constexpr auto result = get_number_placeholders();
        static_assert(result.has_value(), "Failed to parse format string at compile time");
        return result.value();
    }();

    static constexpr auto placeholder_positions = get_placeholder_positions();
    constexpr std::string_view view() const { return fmt.view(); }
};

template <fixed_string fmt_str>
consteval std::expected<size_t, parse_error> format_string<fmt_str>::get_number_placeholders() {
    constexpr size_t N = fmt.size();
    if (N <= 1)
        return 0;

    size_t placeholder_count = 0;
    size_t pos = 0;
    const size_t size = N - 1;  // -1 для игнорирования нуль-терминатора

    while (pos < size) {
        // Пропускаем все символы до '{'
        if (fmt.data[pos] != '{') {
            ++pos;
            continue;
        }

        // Проверяем незакрытый плейсхолдер
        if (pos + 1 >= size) {
            return std::unexpected(parse_error{"Unclosed last placeholder"});
        }

        // Начало плейсхолдера
        ++placeholder_count;
        ++pos;

        // Проверка спецификатора формата
        if (fmt.data[pos] == '%') {
            ++pos;
            if (pos >= size) {
                return std::unexpected(parse_error{"Unclosed last placeholder"});
            }

            // Проверяем допустимые спецификаторы
            const char spec = fmt.data[pos];
            constexpr char valid_specs[] = {'d', 'u', 's'};
            bool valid = false;

            for (const char s : valid_specs) {
                if (spec == s) {
                    valid = true;
                    break;
                }
            }

            if (!valid) {
                return std::unexpected(parse_error{"Invalid specifier."});
            }
            ++pos;
        }

        // Проверяем закрывающую скобку
        if (pos >= size || fmt.data[pos] != '}') {
            return std::unexpected(parse_error{"'}' hasn't been found in appropriate place"});
        }
        ++pos;
    }

    return placeholder_count;
}

template <fixed_string fmt_str>
consteval auto format_string<fmt_str>::get_placeholder_positions() {
    constexpr size_t N = fmt.size() - 1;
    constexpr size_t num = number_placeholders;
    std::array<std::pair<size_t, size_t>, num> positions{};

    size_t placeholder_idx = 0;
    size_t pos = 0;

    while (pos < N && placeholder_idx < num) {
        if (fmt.data[pos] != '{') {
            ++pos;
            continue;
        }

        size_t start = pos;
        ++pos;

        if (fmt.data[pos] == '%') {
            ++pos;  // пропускаем %
            ++pos;  // пропускаем спецификатор
        }

        // Находим закрывающую скобку
        while (pos < N && fmt.data[pos] != '}') {
            ++pos;
        }

        positions[placeholder_idx++] = {start, pos};
        ++pos;
    }

    return positions;
}

}  // namespace stdx::details

// Пользовательский литерал
template <stdx::details::fixed_string str>
consteval auto operator""_fs() {
    return stdx::details::format_string<str>{};
}
