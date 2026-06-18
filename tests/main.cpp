#include "scan.hpp"

#include <cassert>

// Тест 0: Ошибки.

// error: Signed integer requires 'd' specifier
// static_assert(stdx::scan<"{%u}"_fs, "1", int8_t>().get<0>() == 1);

// error: Unsigned integer requires 'u' specifier
// static_assert(stdx::scan<"{%d}"_fs, "2", uint8_t>().get<0>() == 2);

// error, overflow
// static_assert(stdx::scan<"{%d}"_fs, "128", int8_t>().get<0>() == 128);
// static_assert(stdx::scan<"{%u}"_fs, "256", uint8_t>().get<0>() == 256);

// error, Number of placeholders doesn't match number of types
// static_assert(stdx::scan<"{} {}"_fs, "1 2", std::string_view>().values() == std::tuple("1", "2"));
// ok
static_assert(stdx::scan<"{} {}"_fs, "1 2", std::string_view, std::string_view>().values() == std::tuple("1", "2"));

// error, Unsupported type in parameter pack
// static_assert(stdx::scan<"{%d}"_fs, "1", float>().get<0>() == 1);
// static_assert(stdx::scan<"{%d}"_fs, "1", bool>().get<0>() == 1);

// Тест 1: Целые числа со спецификатором d
static_assert(stdx::scan<"{%d} {%d}"_fs, "42 100", int, int>().get<0>() == 42);
static_assert(stdx::scan<"{%d} {%d}"_fs, "42 100", int, int>().get<1>() == 100);
static_assert(stdx::scan<"{%d} {%d}"_fs, "42 100", int, int>().values() == std::tuple(42, 100));

static_assert(stdx::scan<"{%d}"_fs, "42", int>().get<0>() == 42);
static_assert(stdx::scan<"value: {%d}"_fs, "value: 123", int>().get<0>() == 123);

// Тест 2: Беззнаковые числа со спецификатором u
static_assert(stdx::scan<"{%u}"_fs, "42", unsigned int>().get<0>() == 42u);
static_assert(stdx::scan<"{%u} {%u}"_fs, "100 200", unsigned int, unsigned int>().get<1>() == 200u);

// Тест 3: Строки со спецификатором s
static_assert(stdx::scan<"{%s}"_fs, "hello", std::string_view>().get<0>() == "hello");
static_assert(stdx::scan<"{} {}"_fs, "hello world", std::string_view, std::string_view>().get<1>() == "world");

// Тест 4: Смешанные типы
static_assert(stdx::scan<"{} {%u} {}"_fs, "answer 42 text", std::string_view, unsigned int, std::string_view>()
                  .get<0>() == "answer");

// Тест 5: Разные целочисленные типы
static_assert(stdx::scan<"{%d}"_fs, "127", int8_t>().get<0>() == 127);
static_assert(stdx::scan<"{%d}"_fs, "32767", int16_t>().get<0>() == 32767);
static_assert(stdx::scan<"{%u}"_fs, "255", uint8_t>().get<0>() == 255);

// Тест 6: "cv-qualifiers"
static_assert(stdx::scan<"{%d}"_fs, "127", const int8_t>().get<0>() == 127);
static_assert(stdx::scan<"{%u}"_fs, "127", const uint8_t>().get<0>() == 127);
static_assert(stdx::scan<"{%d}"_fs, "127", volatile int16_t>().get<0>() == 127);
static_assert(stdx::scan<"{%u}"_fs, "127", volatile uint16_t>().get<0>() == 127);
static_assert(stdx::scan<"{%u}"_fs, "127", const volatile uint16_t>().get<0>() == 127);
static_assert(stdx::scan<"{%u} {%d} {%u}"_fs, "1 2 3", const uint8_t, volatile int8_t, const volatile uint8_t>()
                  .values() == std::tuple(1, 2, 3));
// Грехи отцов.
// 1)
// const и volatile не сохраняются в возвращаемом значении, однако (а в примере в main - сохраняется const) ) Нужно ли
// справлять? static_assert(std::is_same_v<decltype(stdx::scan<"{%u}"_fs, "127", const volatile uint16_t>().get<0>()),
// const volatile uint16_t>);
//  2) Слитные {}{}... не работают )) Нужно переколбашивать парасеры ) Исправлять ?
// static_assert(stdx::scan<"{%u}{%d}{%u}"_fs, "1 2 3", uint8_t, int8_t, uint8_t>().values() ==
//               std::tuple<uint8_t, int8_t, uint8_t>(1, 2, 3));

int main() {
    constexpr const volatile uint16_t v0 = stdx::scan<"{%u}"_fs, "127", const volatile uint16_t>().get<0>();

    // const сохраняется, volatile нет. Нужно ли исправлять? (Если да, то как?)
    constexpr decltype(auto) v1 = stdx::scan<"{%u}"_fs, "127", const volatile uint16_t>().get<0>();
    static_assert(std::is_same_v<decltype(v1), const uint16_t>);
    // static_assert(std::is_same_v<decltype(v1), const volatile uint16_t>);
    return 0;
}