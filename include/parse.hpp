#pragma once

#include <charconv>
#include <concepts>
#include <cstdint>
#include <system_error>

#include "format_string.hpp"
#include "types.hpp"

namespace stdx::details {

// Концепт для проверки поддерживаемых типов
template <typename T>
concept supported_type = std::is_same_v<std::remove_cv_t<T>, std::string_view> ||
                         std::signed_integral<std::remove_cv_t<T>> || std::unsigned_integral<std::remove_cv_t<T>>;

// Шаблонная функция, возвращающая пару позиций в строке с исходными данными, соответствующих I-ому плейсхолдеру
template <int I, format_string fmt, fixed_string source>
consteval auto get_current_source_for_parsing() {
    static_assert(I >= 0 && I < fmt.number_placeholders, "Invalid placeholder index");

    constexpr auto to_sv = [](const auto &fs) { return std::string_view(fs.data, fs.size() - 1); };

    constexpr auto fmt_sv = to_sv(fmt.fmt);
    constexpr auto src_sv = to_sv(source);
    constexpr auto &positions = fmt.placeholder_positions;

    // Получаем границы текущего плейсхолдера в формате
    constexpr auto pos_i = positions[I];
    constexpr size_t fmt_start = pos_i.first, fmt_end = pos_i.second;

    // Находим начало в исходной строке
    constexpr auto src_start = [&] {
        if constexpr (I == 0) {
            return fmt_start;
        } else {
            // Находим конец предыдущего плейсхолдера в исходной строке
            constexpr auto prev_bounds = get_current_source_for_parsing<I - 1, fmt, source>();
            const auto prev_end = prev_bounds.second;

            // Получаем разделитель между текущим и предыдущим плейсхолдерами
            constexpr auto prev_fmt_end = positions[I - 1].second;
            constexpr auto sep = fmt_sv.substr(prev_fmt_end + 1, fmt_start - (prev_fmt_end + 1));

            // Ищем разделитель после предыдущего значения
            auto pos = src_sv.find(sep, prev_end);
            return pos != std::string_view::npos ? pos + sep.size() : src_sv.size();
        }
    }();

    // Находим конец в исходной строке
    constexpr auto src_end = [&] {
        // Получаем разделитель после текущего плейсхолдера
        if constexpr (fmt_end == (fmt_sv.size() - 1)) {
            return src_sv.size();
        }
        constexpr auto sep =
            fmt_sv.substr(fmt_end + 1, (I < fmt.number_placeholders - 1) ? positions[I + 1].first - (fmt_end + 1)
                                                                         : fmt_sv.size() - (fmt_end + 1));
        // Ищем разделитель после текущего значения
        constexpr auto pos = src_sv.find(sep, src_start);
        return pos != std::string_view::npos ? pos : src_sv.size();
    }();
    return std::pair{src_start, src_end};
}

// Функции parse_value для разных типов
template <typename T, char spec>
consteval std::expected<T, parse_error> parse_value(std::string_view sv) {
    if constexpr (std::same_as<T, std::string_view>) {
        static_assert(spec == 's', "String requires 's' specifier");
        return sv;
    } else if constexpr (std::is_signed_v<T>) {
        static_assert(spec == 'd', "Signed integer requires 'd' specifier");
        T value{};
        auto [ptr, ec] = std::from_chars(sv.data(), sv.data() + sv.size(), value);
        if (ec != std::errc() || ptr != sv.data() + sv.size()) {
            return std::unexpected(parse_error{"Failed to parse integer"});
        }
        return value;
    } else if constexpr (std::is_unsigned_v<T>) {
        static_assert(spec == 'u', "Unsigned integer requires 'u' specifier");
        T value{};
        auto [ptr, ec] = std::from_chars(sv.data(), sv.data() + sv.size(), value);
        if (ec != std::errc() || ptr != sv.data() + sv.size()) {
            return std::unexpected(parse_error{"Failed to parse unsigned integer"});
        }
        return value;
    }
    return std::unexpected(parse_error{"Unsupported type"});
}

// Вспомогательная функция для получения спецификатора как constexpr
template <int I, format_string fmt_str>
consteval char get_specifier() {
    constexpr auto &positions = fmt_str.placeholder_positions;
    constexpr auto pos_i = positions[I];
    constexpr auto fmt_sv = fmt_str.view();
    constexpr auto spec_pos = pos_i.first + 1;

    if constexpr (spec_pos < fmt_sv.size() && fmt_sv[spec_pos] == '%') {
        return fmt_sv[spec_pos + 1];
    }
    return 's';
}

// Шаблонная функция, выполняющая преобразования исходных данных в конкретный тип на основе I-го плейсхолдера
template <int I, format_string fmt_str, fixed_string source, typename T>
consteval auto parse_input() {
    static_assert(supported_type<T>, "Unsupported type for parsing");

    constexpr auto bounds = get_current_source_for_parsing<I, fmt_str, source>();
    constexpr auto source_sv = source.view();
    constexpr auto substring = source_sv.substr(bounds.first, bounds.second - bounds.first);

    // Получаем спецификатор как constexpr значение
    constexpr char spec = get_specifier<I, fmt_str>();

    // Вызываем parse_value с шаблонным спецификатором
    constexpr auto result = parse_value<T, spec>(substring);
    static_assert(result.has_value(), "Failed to parse value");
    return result.value();
}

}  // namespace stdx::details
