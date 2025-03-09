//
// Created by ruixuezhao on 25-3-9.
//

#ifndef ITOA_HPP
#define ITOA_HPP
#include "type_traits.hpp"

namespace StringFlow
{
    enum class IotaCase : char
    {
        Upper = 'A',
        Lower = 'a',
    };

    template <typename integral>
    size_t itoa(integral value, char *string, size_t radix = 10, IotaCase type = IotaCase::Lower);
} // namespace fmt

template <typename integral>
size_t StringFlow::itoa(integral value, char *string, size_t radix, IotaCase type)
{
    char tmp[33];
    char *tp = tmp;
    integral i;
    integral v;
    char sign;
    char *sp;

    static_assert(std::is_integral<integral>::value, "value is not signed integral");

    if (string == nullptr)
    {
        return 0;
    }

    if (radix > 36 || radix <= 1)
    {
        return 0;
    }

    sign = (value >= 0) ? '+' : '-';
    v = (value >= 0) ? value : -value;

    do
    {
        i = v % radix;
        v = v / radix;
        *tp++ = (i < 10) ? (i + '0') : (i + (char)type - 10);
    } while (v != 0 && (size_t)(tp - tmp) < sizeof(tmp));

    sp = string;

    if (sign == '-')
    {
        *sp++ = '-';
    }
    while (tp > tmp)
    {
        *sp++ = *--tp;
    }
    *sp = 0;

    return (size_t)(sp - string);
}

#endif //ITOA_HPP
