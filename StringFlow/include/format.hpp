//
// Created by ruixuezhao on 25-3-7.
//

#ifndef FORMAT_HPP
#define FORMAT_HPP
#include <cmath>
#include <cstring>
#include <include/utils.hpp>
#include <include/type_traits.hpp>
#include <include/itoa.hpp>
#include "result/result.h"

#include <algorithm>
#include <cfloat>

using namespace result;
namespace StringFlow {
    using OutputFunc =int(*)(const char *);

    enum class format_error {
        success = 0,
        // 格式字符串错误
        unmatched_brace,       // 花括号不匹配
        invalid_format_spec,  // 无效格式说明符
        argument_index_out_of_range,
        // 类型错误
        unsupported_type,
        type_mismatch,
        // 数值错误
        number_overflow,
        nan_format_error,
        inf_format_error,
        // 缓冲区错误
        buffer_full,
        // 对齐错误
        invalid_alignment
    };
    inline std::string format_error_to_string(format_error code){
        switch (code) {
            // 成功状态
            case format_error::success:
                return "Success";

            // 格式字符串错误
            case format_error::unmatched_brace:
                return "Unmatched braces in format string";
            case format_error::invalid_format_spec:
                return "Invalid format specifier";
            case format_error::argument_index_out_of_range:
                return "Argument index out of range";

            // 类型错误
            case format_error::unsupported_type:
                return "Unsupported data type";
            case format_error::type_mismatch:
                return "Type mismatch between format specifier and argument";

            // 数值错误
            case format_error::number_overflow:
                return "Number exceeds allowable range";
            case format_error::nan_format_error:
                return "Invalid NaN (Not-a-Number) formatting attempt";
            case format_error::inf_format_error:
                return "Invalid INF (Infinity) formatting attempt";

            // 缓冲区错误
            case format_error::buffer_full:
                return "Output buffer full";

            // 对齐错误
            case format_error::invalid_alignment:
                return "Invalid alignment specification";

            // 未知错误处理
            default:
                return "Unknown error code: " + std::to_string(static_cast<int>(code));
        }
    }

    //声明所需要的全部函数
    template <class output_str_function_wrap,typename ... Args>
    Result<size_t,format_error> format_to(output_str_function_wrap && output_str_function_wrap_,const char * format,Args&&...args);

    template <size_t Index, class output_str_function_wrap, typename Arg, typename... Args>
     Result<bool,format_error> formatter_to(size_t index, output_str_function_wrap &&out_fct_wrap, const Context &context, Arg &&arg, Args &&...args);
    template <size_t Index, class output_str_function_wrap>
     Result<bool,format_error> formatter_to(size_t index, output_str_function_wrap out_fct_wrap, const Context &context) { return Ok(false); }

    template <class output_str_function_wrap, typename Arg>
     Result<bool,format_error> handle_integral(output_str_function_wrap out_fct_wrap, const FormatterOption &option, Arg &&arg);
    template <class output_str_function_wrap, typename Arg>
     Result<bool,format_error> handle_float(output_str_function_wrap out_fct_wrap, const FormatterOption &option, Arg &&arg);
    template <class output_str_function_wrap, typename Arg>
     Result<bool,format_error> ftoa_to(output_str_function_wrap out_fct_wrap, const FormatterOption &option, Arg &&arg);
    template <class output_str_function_wrap, typename Arg>
     Result<bool,format_error> etoa_to(output_str_function_wrap out_fct_wrap, const FormatterOption &option, Arg &&arg);
    template <class output_str_function_wrap>
     Result<bool,format_error> handle_point(output_str_function_wrap out_fct_wrap, const FormatterOption &option, const void *arg);
    template <class output_str_function_wrap>
     Result<bool,format_error> handle_cstring(output_str_function_wrap out_fct_wrap, const FormatterOption &option, const char *arg);
    template <class output_str_function_wrap, typename Arg>
     Result<bool,format_error> handle_class(output_str_function_wrap out_fct_wrap, const Context &context, Arg arg);

    template <class output_str_function_wrap>
     Result<bool,format_error> handle_rev(output_str_function_wrap out_fct_wrap, const FormatterOption &option, const char *buffer, size_t length);

    // 默认版本，使用 putchar
    template<typename ...Args>
    Result<size_t, format_error> print(const char* format, Args&&... args) {
        return format_to(putchar, format, std::forward<Args>(args)...);
    }

    // 自定义输出函数的版本
    template<typename ...Args>
    Result<size_t, format_error> print( const char* format, Args&&... args,OutputFunc out) {
        return format_to(out, format, std::forward<Args>(args)...);
    }

    // 默认版本，使用 putchar
    template<typename ...Args>
    Result<size_t, format_error> println(const char* format, Args&&... args) {
        auto retval = format_to(putchar, format, std::forward<Args>(args)...);
        putchar('\n');
        return  retval;
    }

    // 自定义输出函数的版本
    template< typename ...Args>
    Result<size_t, format_error> println( const char* format, Args&&... args,OutputFunc out) {
        auto retval = format_to(out, format, std::forward<Args>(args)...);
        out("\n");
        return  retval;
    }

    template <typename... Args>
   size_t format_to_buffer(void *buffer, size_t size, const char *format, Args &&...args)
    {
        auto begin = static_cast<char *>(buffer);
        auto iter = static_cast<char *>(buffer);
        auto out_to_buffer_wrap = [begin, &iter, size](char ch) {
            if (static_cast<size_t>(iter - begin) < size - 1)
            {
                *iter++ = ch;
                *iter = '\0';
            }
        };
        return format_to(out_to_buffer_wrap, format, args...);
    }

    template <class output_str_function_wrap,typename ... Args>
    Result<size_t,format_error> format_to(output_str_function_wrap && output_str_function_wrap_,const char * format,Args&&...args) {
        size_t count = 0;
        size_t auto_index = 0;

        if (!format) return Err(format_error::invalid_alignment);

        for (; *format; ++format) {
            if (*format != '{' && *format != '}') {
                output_str_function_wrap_(*format);
                continue;
            }

            if (*format == '{') {
                const char* spec_begin = format++;
                const char* colon = nullptr;
                bool is_closed = false;

                // Parse format specifier
                while (*format && *format != '{' && *format != '}') {
                    if (*format == ':' && !colon) colon = format;
                    ++format;
                }

                if (*format == '}') {
                    is_closed = true;
                    const char* spec_end = format;//此处由++改为不加这样就能支持{0}{1}二者都输出{0} {1}
                    const char* num_start = spec_begin + 1;

                    // Parse argument index
                    size_t arg_index = auto_index;
                    if (num_start < spec_end && *num_start >= '0' && *num_start <= '9') {
                        arg_index = 0;
                        while (num_start < spec_end && *num_start >= '0' && *num_start <= '9') {
                            arg_index = arg_index * 10 + (*num_start++ - '0');
                        }
                    } else {
                        arg_index = auto_index++;
                    }

                    count += formatter_to<0>(
                        arg_index,
                        output_str_function_wrap_,
                        {spec_begin, colon, spec_end},
                        std::forward<Args>(args)...
                    );
                } else {
                    // Handle unclosed brace or escape
                    output_str_function_wrap_('{');
                    if (*format == '{') output_str_function_wrap_(*format++);
                    // if (format_to(output_str_function_wrap_,colon+1,args...).is_err())
                    // {
                    //     output_str_function_wrap_('}');
                    // }
                }
            } else { // Handle '}'
                // Find closing brace
                const char* start = format++;
                while (*format && *format != '}') ++format;

                if (*format == '}') {
                    output_str_function_wrap_('}');
                    ++format;
                }
            }
        }

        return Ok(count);
    }

    template <size_t Index, class output_str_function_wrap, typename Arg, typename... Args>
    Result<bool,format_error> formatter_to(size_t index, output_str_function_wrap &&out_fct_wrap, const Context &context, Arg &&arg, Args &&...args) {
        FormatterOption option;

        if (Index != index)
            return formatter_to<Index + 1>(index, out_fct_wrap, context, args...);

        // 类类型处理
        if constexpr (type_check<Arg>::is_class_v) {
            constexpr bool has_custom = has_out_class_function<output_str_function_wrap, Arg>::value;
            return has_custom ? out_fct_wrap(context, arg) : out_class_to(out_fct_wrap, context, arg);
        }

        context.unpack_to(option);

        // 统一指针处理 (包含 C 字符串)
        constexpr bool is_ptr = type_check<Arg>::is_pointer_v;
        constexpr bool is_cstr = type_check<Arg>::is_cstring_v;
        if constexpr (is_ptr || is_cstr) {
            if (option.type == Type::Pointer || option.type == Type::pointer) {
                const void* ptr = is_cstr ? static_cast<const void*>(arg) :
                                          static_cast<const void*>(&arg);
                return handle_point(out_fct_wrap, option, ptr);
            }
        }

        // 基础类型分发器
         auto handle_scalar = [&](auto type_tag) {
            using T = decltype(type_tag);
            if constexpr (std::is_same_v<T, char>) {
                option.type = (option.type == Type::None) ? Type::Chr : option.type;
            } else if constexpr (std::is_same_v<T, bool>) {
                option.type = (option.type == Type::None) ? Type::Bol : option.type;
            } else if constexpr (std::is_integral_v<T>) {
                option.type = (option.type == Type::None) ? Type::Dec : option.type;
            }
            return handle_integral(out_fct_wrap, option, arg);
        };

        if constexpr (type_check<Arg>::is_character_v) {
            return handle_scalar(char{});
        } else if constexpr (type_check<Arg>::is_bool_v) {
            return handle_scalar(bool{});
        } else if constexpr (type_check<Arg>::is_signed_int_v ||
                            type_check<Arg>::is_unsigned_int_v) {
            return handle_scalar(int{});
        }

        // 浮点类型处理
        if constexpr (type_check<Arg>::is_floating_point_v) {
            constexpr auto is_in_float_range = [](auto val) {
                return (val < max_float && val >= min_float) ||
                      (-val < max_float && -val >= min_float);
            };
            option.type = (option.type == Type::None) ?
                         (is_in_float_range(arg) ? Type::Float : Type::Exp) :
                         option.type;
            return handle_float(out_fct_wrap, option, arg);
        }

        // 兜底处理
        if constexpr (is_cstr) return handle_cstring(out_fct_wrap, option, arg);
        if constexpr (is_ptr)  return handle_point(out_fct_wrap, option,
                                                   static_cast<const void*>(arg));
    }


    template <class output_str_function_wrap, typename Arg>
    Result<bool,format_error> handle_integral(output_str_function_wrap out_fct_wrap, const FormatterOption &option, Arg &&arg) {
        char temp[33]={0};
        switch (option.type) {
            case Type::Chr:
                return handle_rev(out_fct_wrap, option, (const char*)&arg, 1);//只能使用c风格的强转换，不然报错

            case Type::Bol:
                return handle_rev(out_fct_wrap, option, arg ? "true" : "false", arg ? 4 : 5);

            default: {
                // 统一处理数值类型
                const auto [radix, itoa_case] = [option]{
                    switch (option.type) {
                        case Type::Bin:  return std::make_pair(2,  IotaCase::Lower);
                        case Type::Oct:  return std::make_pair(8,  IotaCase::Lower);
                        case Type::Dec:  return std::make_pair(10, IotaCase::Lower);
                        case Type::hex:  return std::make_pair(16, IotaCase::Lower);
                        case Type::Hex:  return std::make_pair(16, IotaCase::Upper);
                        default:         return std::make_pair(0,  IotaCase::Lower);
                    }
                }();

                if (radix == 0) return Err(format_error::type_mismatch);

                // 符号预处理
                const bool needs_sign = (arg < 0) || (option.sign == Sign::Minus);
                const char sign_char = needs_sign ? '-' :
                                      (option.sign == Sign::Plus) ? '+' : '\0';

                char* buffer_start = temp;
                if (sign_char) {
                    *buffer_start++ = sign_char;
                }

                // 统一数值转换
                size_t length = itoa(needs_sign ? std::abs(arg) : arg, buffer_start, radix, itoa_case);
                length += (buffer_start - temp);  // 包含符号长度

                return handle_rev(out_fct_wrap, option, temp, length);
            }
        }
    }

    template <class output_str_function_wrap, typename Arg>
     Result<bool,format_error> handle_float(output_str_function_wrap out_fct_wrap, const FormatterOption &option, Arg &&arg) {
        static_assert(type_check<Arg>::is_floating_point_v);

        // 处理特殊值
        if (arg != arg)  // NaN检测
            return handle_rev(out_fct_wrap, option, "nan", 3);

        if (arg < -DBL_MAX)  // 负无穷
            return handle_rev(out_fct_wrap, option, "-inf", 4);

        // 处理正无穷
        if (arg > DBL_MAX) {
            const auto [inf_str, inf_len] = [&]() -> std::pair<const char*, size_t> {
                switch (option.sign) {
                    case Sign::Minus: return {"inf",   3};
                    case Sign::Plus:  return {"+inf",  4};
                    case Sign::Space: return {" inf",  4};
                    default:          return {nullptr, 0};
                }
            }();
            return inf_str ? handle_rev(out_fct_wrap, option, inf_str, inf_len) : Err(format_error::number_overflow);
        }

        // 常规数值格式化
        switch (option.type) {
            case Type::Float: return ftoa_to(out_fct_wrap, option, arg);
            case Type::exp:   // 允许小写形式
            case Type::Exp:   return etoa_to(out_fct_wrap, option, arg);
            default:          return  Err(format_error::type_mismatch);
        }
    }

    template <class output_str_function_wrap, typename Arg>
     Result<bool,format_error> ftoa_to(output_str_function_wrap out_fct_wrap, const FormatterOption &option, Arg &&arg) {
        char temp[66]={0};
        char* pos = temp;
        auto value = arg;

        // 符号处理
        if (value < 0) {
            *pos++ = '-';
            value = -value;
        } else if (option.sign != Sign::Minus) {
            *pos++ = static_cast<char>(option.sign);
        }

        // 整数部分处理
        const int integer = static_cast<int>(value);
        const size_t int_len = (integer != 0) ? itoa(integer, pos) : (pos += (*pos = '0', 1), 0);
        pos += int_len;
        *pos++ = '.';  // 小数点

        // 小数部分参数
        const size_t max_decimals = option.precision;
        size_t buffer_remaining = sizeof(temp) - (pos - temp);

        // 小数位生成
        for (size_t i = 0; buffer_remaining > 1 && i < max_decimals; ++i, --buffer_remaining) {

            const int digit = static_cast<int>(value * 10) % 10;
            *pos++ = '0' + digit;
            value *= 10;

            if (option.auto_precision && value < 1e-9) break;  // 浮点精度容差
        }

        return handle_rev(out_fct_wrap, option, temp, pos - temp);
    }

    template <class output_str_function_wrap, typename Arg>
     Result<bool,format_error> etoa_to(output_str_function_wrap out_fct_wrap, const FormatterOption &option, Arg &&arg) {
        char temp[66]={0};
        char* iter = temp;
        double value = arg;
        int expval;

        // 安全类型转换替代联合体
        auto safe_bit_cast = [](auto& dest, const auto& src) {
            static_assert(sizeof(dest) == sizeof(src), "Size mismatch");
            memcpy(&dest, &src, sizeof(dest));
        };

        // 提取浮点位表示
        uint64_t bit_representation;
        safe_bit_cast(bit_representation, value < 0 ? -value : value);

        // 计算 log2 指数
        const int exp2 = static_cast<int>((bit_representation >> 52U) & 0x07FFU) - 1023;

        // 构造标准化尾数 [1,2)
        bit_representation = (bit_representation & ((1ULL << 52U) - 1U)) | (0x3FF0000000000000ULL);
        double mantissa;
        safe_bit_cast(mantissa, bit_representation);

        // 近似 log10 计算
        expval = static_cast<int>(0.1760912590558 + exp2 * 0.301029995663981 + (mantissa - 1.5) * 0.289529654602168);

        // 指数修正计算
        int exp_correction = static_cast<int>(expval * 3.321928094887362 + 0.5);
        const double z = expval * 2.302585092994046 - exp_correction * 0.6931471805599453;
        const double z_sq = z * z;

        // 连分数近似
        const double exp_factor = 1.0 + (2.0 * z) / (2.0 - z + z_sq / (6.0 + z_sq / (10.0 + z_sq / 14.0)));

        // 构造缩放因子
        bit_representation = static_cast<uint64_t>(exp_correction + 1023) << 52U;
        double scaling_factor;
        safe_bit_cast(scaling_factor, bit_representation);
        scaling_factor *= mantissa * exp_factor;

        // 边界修正
        if (std::abs(value) < scaling_factor) {
            scaling_factor /= 10.0;
            --expval;
        }

        // 格式化输出
         const bool success = ftoa_to(out_fct_wrap,
                                FormatterOption{
                                    .sign = option.sign,
                                    .width = 0,
                                    .auto_precision = option.auto_precision,
                                    .precision = option.precision,
                                    .type = Type::Float
                                },
                                value / scaling_factor
         ).unwrap();

        if (!success) return Err(format_error::inf_format_error);

        out_fct_wrap(static_cast<char>(option.type));
        return handle_integral(out_fct_wrap, {.type = Type::Dec}, expval);
    }

    template <class output_str_function_wrap>
     Result<bool,format_error> handle_point(output_str_function_wrap out_fct_wrap, const FormatterOption &option, const void *arg) {
        char temp[33];
        const auto ptr = reinterpret_cast<uintptr_t>(arg);
        size_t length = itoa(ptr, temp, 16, (option.type == Type::Pointer) ? IotaCase::Upper : IotaCase::Lower);

        if (length >= option.width)
        {
            for (size_t i = 0; i < length; i++)
                out_fct_wrap(temp[i]);
            return Ok(true);
        }

        return handle_rev(out_fct_wrap, option, temp, length);
    }

    template <class output_str_function_wrap>
     Result<bool,format_error> handle_cstring(output_str_function_wrap out_fct_wrap, const FormatterOption &option, const char *arg) {
        size_t length = strlen(arg);

        if (length >= option.width)
        {
            for (size_t i = 0; i < length; i++)
                out_fct_wrap(arg[i]);
            return Ok(true);
        }

        return handle_rev(out_fct_wrap, option, arg, length);
    }

    template <class output_str_function_wrap, typename Arg>
     Result<bool,format_error> handle_class(output_str_function_wrap out_fct_wrap, const Context &context, Arg arg) {
        FormatterOption option;
        const char *temp = typeid(Arg).name();
        context.unpack_to(option);
        return handle_rev(out_fct_wrap, option, temp, strlen(temp));
    }

    template <class output_str_function_wrap>
     Result<bool,format_error> handle_rev(output_str_function_wrap out_fct_wrap, const FormatterOption &option, const char *buffer, size_t length) {
        auto write_buffer = [&](const char* data, size_t len) {
            for (size_t i = 0; i < len; ++i)
                out_fct_wrap(data[i]);
        };

        auto write_fill = [&](size_t count) {
            for (size_t i = 0; i < count; ++i)
                out_fct_wrap(option.fill);
        };

        if (option.width <= length) {
            write_buffer(buffer, length);
            return Ok(true);
        }

        const size_t padding = option.width - length;

        switch (option.align) {
            case Align::Left:
                write_buffer(buffer, length);
            write_fill(padding);
            break;

            case Align::Center: {
                const size_t left_pad = padding / 2;
                const size_t right_pad = padding - left_pad;
                write_fill(left_pad);
                write_buffer(buffer, length);
                write_fill(right_pad);
                break;
            }

            case Align::Right:
                write_fill(padding);
            write_buffer(buffer, length);
            break;

            default:
                return Err(format_error::invalid_alignment);
        }

        return Ok(true);
    }


}
#endif //FORMAT_HPP
