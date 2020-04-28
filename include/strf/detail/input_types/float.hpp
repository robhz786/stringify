#ifndef STRF_DETAIL_INPUT_TYPES_FLOAT_HPP
#define STRF_DETAIL_INPUT_TYPES_FLOAT_HPP

//  Distributed under the Boost Software License, Version 1.0.
//  (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

#include <strf/printer.hpp>
#include <strf/facets_pack.hpp>
#include <strf/detail/facets/numpunct.hpp>
#include <strf/detail/ryu/double.hpp>
#include <strf/detail/ryu/float.hpp>
#include <algorithm>
#include <cstring>
#include <type_traits>

namespace strf {
namespace detail {

struct double_dec
{
    std::uint64_t m10;
    std::int32_t e10;
    bool negative;
    bool infinity;
    bool nan;
};

struct double_dec_base
{
    std::uint64_t m10;
    std::int32_t e10;
};

#if ! defined(STRF_OMIT_IMPL)

STRF_INLINE STRF_HD double_dec_base trivial_float_dec(
    std::uint32_t ieee_mantissa,
    std::int32_t biased_exponent,
    std::uint32_t k )
{
    constexpr int m_size = 23;

    STRF_ASSERT(-10 <= biased_exponent && biased_exponent <= m_size);
    STRF_ASSERT((std::int32_t)k == (biased_exponent * 179 + 1850) >> 8);
    STRF_ASSERT(0 == (ieee_mantissa & (0x7FFFFF >> k)));

    STRF_ASSERT(k <= m_size);
    STRF_ASSERT(biased_exponent <= (int)k);

    std::int32_t e10 = biased_exponent - k;
    std::uint32_t m = (1ul << k) | (ieee_mantissa >> (m_size - k));
    int p5 = k - biased_exponent;
    STRF_ASSERT(p5 <= 10);
    // when p5 >= 8 , k <= 5; then (m & 0xFF) != 0
    // if (p5 >= 8 && (0 == (m & 0xFF))) {
    //     p5 -= 8;
    //     e10 += 8;
    //     m = m >> 8;
    // }
    if (p5 >= 4 && (0 == (m & 0xF))) {
        p5 -= 4;
        e10 += 4;
        m = m >> 4;
    }
    if (p5 >= 2 && (0 == (m & 0x3))) {
        p5 -= 2;
        e10 += 2;
        m = m >> 2;
    }
    if (p5 != 0 && (0 == (m & 1))) {
        -- p5;
        ++ e10;
        m = m >> 1;
    }
    while ((m % 10) == 0){
        m /= 10;
        ++e10;
    }
    if (p5 >= 8) {
        m *= 390625ul; // (m << 18) + (m << 16) + (m << 15) + (m << 14) + ...
        p5 -= 8;
    }
    if (p5 >= 4) {
        m *= 625;  // = (m << 9) + (m << 6) + (m << 5) + (m << 4) + m
        p5 -= 4;
    }
    if (p5 >= 2) {
        m *= 25; // = (m << 4) + (m << 3) + m
        p5 -= 2;
    }
    if (p5 >= 1) {
        m = (m << 2) + m; // m *= 5
    }
    STRF_ASSERT((m % 10) != 0);
    return {m, e10};
}

STRF_INLINE STRF_HD double_dec_base trivial_double_dec(
    std::uint64_t ieee_mantissa,
    std::int32_t biased_exponent,
    std::uint32_t k )
{
    STRF_ASSERT(-22 <= biased_exponent && biased_exponent <= 52);
    STRF_ASSERT((std::int32_t)k == (biased_exponent * 179 + 4084) >> 8);
    STRF_ASSERT(0 == (ieee_mantissa & (0xFFFFFFFFFFFFFull >> k)));

    STRF_ASSERT(biased_exponent <= (int)k);
    STRF_ASSERT(k <= 52);

    std::int32_t e10 = biased_exponent - k;
    std::uint64_t m = (1ull << k) | (ieee_mantissa >> (52 - k));
    int p5 = k - biased_exponent;
    STRF_ASSERT(p5 <= 22);
    // when p5 >= 16 , k <= 15; then (m & 0xFFFF) != 0
    // if (p5 >= 16 && (0 == (m & 0xFFFF))) {
    //     p5 -= 16;
    //     e10 += 16;
    //     m = m >> 16;
    // }
    if (p5 >= 8 && (0 == (m & 0xFF))) {
        p5 -= 8;
        e10 += 8;
        m = m >> 8;
    }
    if (p5 >= 4 && (0 == (m & 0xF))) {
        p5 -= 4;
        e10 += 4;
        m = m >> 4;
    }
    if (p5 >= 2 && (0 == (m & 0x3))) {
        p5 -= 2;
        e10 += 2;
        m = m >> 2;
    }
    if (p5 != 0 && (0 == (m & 1))) {
        -- p5;
        ++ e10;
        m = m >> 1;
    }
    while ((m % 10) == 0){
        m /= 10;
        ++e10;
    }
    if (p5 >= 16) {
        m *= 152587890625ull;
        p5 -= 16;
    }
    if (p5 >= 8) {
        m *= 390625ull; // (m << 18) + (m << 16) + (m << 15) + (m << 14) + ...
        p5 -= 8;
    }
    if (p5 >= 4) {
        m *= 625;  // = (m << 9) + (m << 6) + (m << 5) + (m << 4) + m
        p5 -= 4;
    }
    if (p5 >= 2) {
        m *= 25; // = (m << 4) + (m << 3) + m
        p5 -= 2;
    }
    if (p5 >= 1) {
        m = (m << 2) + m; // m *= 5
    }
    STRF_ASSERT((m % 10) != 0);
    return {m, e10};
}
STRF_INLINE STRF_HD detail::double_dec decode(float f)
{
    constexpr int bias = 127;
    constexpr int e_size = 8;
    constexpr int m_size = 23;

    std::uint32_t bits;
    std::memcpy(&bits, &f, 4);
    const std::uint32_t exponent
        = static_cast<std::uint32_t>((bits << 1) >> (m_size + 1));
    const bool sign = (bits >> (m_size + e_size));
    const std::uint32_t mantissa = bits & 0x7FFFFF;

    if (exponent == 0 && mantissa == 0) {
        return {0, 0, sign, false, false};
    } else if (bias - 10 <= exponent && exponent <= bias + m_size) {
        const int e = exponent - bias;
        const unsigned k = (179 * e + 1850) >> 8;
        if (0 == (mantissa & (0x7FFFFF >> k))) {
            auto res = trivial_float_dec(mantissa, e, k);
            return {res.m10, res.e10, sign, false, false};
        }
    } else if (exponent == 0xFF) {
        if (mantissa == 0) {
            return {0, 0, sign, true, false};
        } else {
            return {0, 0, sign, false, true};
        }
    }

    auto fdec = detail::ryu::f2d(mantissa, exponent);
    return {fdec.mantissa, fdec.exponent, sign, false, false};
}


STRF_INLINE STRF_HD detail::double_dec decode(double d)
{
    constexpr int bias = 1023;
    constexpr int e_size = 11; // bits in exponent
    constexpr int m_size = 52; // bits in matissa

    std::uint64_t bits;
    std::memcpy(&bits, &d, 8);
    const std::uint32_t exponent
        = static_cast<std::uint32_t>((bits << 1) >> (m_size + 1));
    const bool sign = (bits >> (m_size + e_size));
    const std::uint64_t mantissa = bits & 0xFFFFFFFFFFFFFull;

    if (exponent == 0 && mantissa == 0) {
        return {0, 0, sign, false, false};
    } else if (bias - 22 <= exponent && exponent <= bias + 52) {
        const int e = exponent - bias;
        const unsigned k = (e * 179 + 4084) >> 8;
        if (0 == (mantissa & (0xFFFFFFFFFFFFFull >> k))) {
            auto res = trivial_double_dec(mantissa, e, k);
            return {res.m10, res.e10, sign, false, false};
        }
     } else if (exponent == 0x7FF) {
        if (mantissa == 0) {
            return {0, 0, sign, true, false};
        } else {
            return {0, 0, sign, false, true};
        }
    }
    auto ddec = detail::ryu::d2d(mantissa, exponent);
    return {ddec.mantissa, ddec.exponent, sign, false, false};
}

#else  // ! defined(STRF_OMIT_IMPL)

detail::double_dec decode(double d);
detail::double_dec decode(float f);

#endif // ! defined(STRF_OMIT_IMPL)

} // namespace detail

enum class float_notation{fixed, scientific, general, hex};

struct float_format_data
{
    unsigned precision = (unsigned)-1;
    bool showpoint = false;
    bool showpos = false;
};

constexpr STRF_HD bool operator==( strf::float_format_data lhs
                                 , strf::float_format_data rhs ) noexcept
{
    return lhs.precision == rhs.precision
        && lhs.showpoint == rhs.showpoint
        && lhs.showpos == rhs.showpos ;
}

constexpr STRF_HD bool operator!=( strf::float_format_data lhs
                                 , strf::float_format_data rhs ) noexcept
{
    return ! (lhs == rhs);
}

template <strf::float_notation Notation>
struct float_format;

template <typename T, strf::float_notation Notation = strf::float_notation::general>
class float_format_fn
{
    template <strf::float_notation OtherNotation>
    using adapted_derived_type_
        = strf::fmt_replace<T, float_format<Notation>, float_format<OtherNotation> >;

public:

    constexpr float_format_fn() noexcept = default;

    constexpr STRF_HD explicit float_format_fn(const strf::float_format_data& data) noexcept
        : data_(data)
    {
    }

    template <typename U, strf::float_notation N>
    constexpr STRF_HD explicit float_format_fn(const float_format_fn<U, N>& other) noexcept
        : data_(other.get_float_format_data())
    {
    }
    constexpr STRF_HD T&& operator+() && noexcept
    {
        data_.showpos = true;
        return static_cast<T&&>(*this);
    }
    constexpr STRF_HD T&& operator*() && noexcept
    {
        data_.showpoint = true;
        return static_cast<T&&>(*this);
    }
    [[deprecated]] // use instead operator*
    constexpr STRF_HD T&& operator~() && noexcept
    {
        data_.showpoint = true;
        return static_cast<T&&>(*this);
    }
    constexpr STRF_HD T&& p(unsigned _) && noexcept
    {
        data_.precision = _;
        return static_cast<T&&>(*this);
    }

    template <strf::float_notation N = strf::float_notation::scientific>
    constexpr STRF_HD
    std::enable_if_t<N == Notation && N == strf::float_notation::scientific, T&&>
    sci() && noexcept
    {
        return static_cast<T&&>(*this);
    }
    template <strf::float_notation N = strf::float_notation::scientific>
    constexpr STRF_HD
    std::enable_if_t< N != Notation && N == strf::float_notation::scientific
                    , adapted_derived_type_<N> >
    sci() const & noexcept
    {
        return adapted_derived_type_<N>{ static_cast<const T&>(*this) };
    }

    template <strf::float_notation N = strf::float_notation::fixed>
    constexpr STRF_HD
    std::enable_if_t<N == Notation && N == strf::float_notation::fixed, T&&>
    fixed() && noexcept
    {
        return static_cast<T&&>(*this);
    }
    template <strf::float_notation N = strf::float_notation::fixed>
    constexpr STRF_HD
    std::enable_if_t< N != Notation && N == strf::float_notation::fixed
                    , adapted_derived_type_<N> >
    fixed() const & noexcept
    {
        return adapted_derived_type_<N>{ static_cast<const T&>(*this) };
    }

    template <strf::float_notation N = strf::float_notation::general>
    constexpr STRF_HD
    std::enable_if_t<N == Notation && N == strf::float_notation::general, T&&>
    gen() && noexcept
    {
        return static_cast<T&&>(*this);
    }
    template <strf::float_notation N = strf::float_notation::general>
    constexpr STRF_HD
    std::enable_if_t< N != Notation && N == strf::float_notation::general
                    , adapted_derived_type_<N> >
    gen() const & noexcept
    {
        return adapted_derived_type_<N>{ static_cast<const T&>(*this) };
    }

    template <strf::float_notation N = strf::float_notation::hex>
    constexpr STRF_HD
    std::enable_if_t<N == Notation && N == strf::float_notation::hex, T&&>
    hex() && noexcept
    {
        return static_cast<T&&>(*this);
    }
    template <strf::float_notation N = strf::float_notation::hex>
    constexpr STRF_HD
    std::enable_if_t< N != Notation && N == strf::float_notation::hex
                    , adapted_derived_type_<N> >
    hex() const & noexcept
    {
        return adapted_derived_type_<N>{ static_cast<const T&>(*this) };
    }

    constexpr strf::float_format_data get_float_format_data() const noexcept
    {
        return data_;
    }

private:

    strf::float_format_data data_;
};

template <strf::float_notation Notation>
struct float_format
{
    template <typename T>
    using fn = float_format_fn<T, Notation>;
};

template<typename FloatT, strf::float_notation Notation, bool Align = false>
using float_with_format = value_with_format
    < FloatT
    , strf::float_format<Notation>
    , strf::alignment_format_q<Align> >;

inline STRF_HD auto make_fmt(strf::rank<1>, float x)
{
    return strf::float_with_format<float, strf::float_notation::general, false>{x};
}

inline STRF_HD auto make_fmt(strf::rank<1>, double x)
{
    return strf::float_with_format<double, strf::float_notation::general, false>{x};
}

inline STRF_HD void make_fmt(strf::rank<1>, long double) = delete;

namespace detail {

template <std::size_t> class fast_double_printer;
template <std::size_t> class fast_punct_double_printer;
template <std::size_t> class double_printer;
template <std::size_t> class punct_double_printer;
template <std::size_t> class hex_double_printer;

template < typename CharT, typename Preview, typename FloatT>
struct fast_double_printer_input
{
    using printer_type = strf::detail::fast_double_printer<sizeof(CharT)>;

    template <typename FPack>
    STRF_HD fast_double_printer_input(const FPack& fp, Preview& preview_, FloatT arg_)
        : preview(preview_)
        , value(arg_)
        , lcase(strf::get_facet<strf::lettercase_c, float>(fp))
    {
    }

    fast_double_printer_input(const fast_double_printer_input&) = default;
    fast_double_printer_input(fast_double_printer_input&&) = default;

    Preview& preview;
    FloatT value;
    strf::lettercase lcase;
};


template <typename CharT, typename FPack, typename Preview, typename FloatT>
struct fast_punct_double_printer_input
{
    using printer_type = strf::detail::fast_punct_double_printer<sizeof(CharT)>;

    FPack fp;
    Preview& preview;
    FloatT value;
};

template < typename CharT, typename FPack, typename Preview, typename FloatT >
struct fast_double_printable_traits
{
    using printer_input_type = std::conditional_t
        < strf::detail::has_punct<CharT, FPack, FloatT, 10>
        , strf::detail::fast_punct_double_printer_input<CharT, FPack, Preview, FloatT>
        , strf::detail::fast_double_printer_input<CharT, Preview, FloatT> >;

    constexpr static STRF_HD printer_input_type
    make_input(const FPack& fp, Preview& preview, FloatT arg)
    {
        return {fp, preview, arg};
    }
};

template < typename CharT, typename FPack, typename Preview, typename FloatT
         , strf::float_notation Notation, bool HasAlignment >
struct fmt_double_printer_input
{
    using printer_type = std::conditional_t
        < Notation == float_notation::hex
        , strf::detail::hex_double_printer<sizeof(CharT)>
        , std::conditional_t
            < strf::detail::has_punct<CharT, FPack, FloatT, 10>
            , strf::detail::punct_double_printer<sizeof(CharT)>
            , strf::detail::double_printer<sizeof(CharT)> > >;

    FPack fp;
    Preview& preview;
    strf::float_with_format<FloatT, Notation, HasAlignment> vwf;
};

} // namespace detail

template <typename CharT, typename FPack, typename Preview>
constexpr STRF_HD strf::detail::fast_double_printable_traits
    < CharT, FPack, Preview, float >
get_printable_traits(Preview&, float)
{ return {}; }

template <typename CharT, typename FPack, typename Preview>
constexpr STRF_HD strf::detail::fast_double_printable_traits
    < CharT, FPack, Preview, double >
get_printable_traits(Preview&, double)
{ return {}; }

template <typename CharT, typename FPack, typename Preview>
constexpr STRF_HD void get_printable_traits(Preview&, long double) = delete;


template < typename CharT, typename FPack, typename Preview
         , typename FloatT, strf::float_notation Notation, bool HasAlignment >
struct printable_traits
    < CharT, FPack, Preview
    , strf::float_with_format<FloatT, Notation, HasAlignment> >
{
    template <typename Arg>
    constexpr static STRF_HD strf::detail::fmt_double_printer_input
        < CharT, FPack, Preview, FloatT, Notation, HasAlignment >
    make_input(const FPack fp, Preview& preview, const Arg& arg)
    {
        return {fp, preview, arg};
    }
};

namespace detail {

struct double_printer_data
{
    unsigned m10_digcount;
    unsigned extra_zeros;
    std::uint64_t m10;
    std::int32_t e10;
    bool negative;
    bool infinity;
    bool nan;
    bool showpoint;
    bool showsign;
    bool sci_notation;
};


template <strf::float_notation Notation>
STRF_HD double_printer_data init_double_printer_data
    ( detail::double_dec d, float_format_data fmt );

template <strf::float_notation Notation>
inline STRF_HD double_printer_data init_double_printer_data
    ( float f, float_format_data fmt )
{
    return init_double_printer_data<Notation>(detail::decode(f), fmt);
}

template <strf::float_notation Notation>
inline STRF_HD double_printer_data init_double_printer_data
    ( double d, float_format_data fmt )
{
    return init_double_printer_data<Notation>(detail::decode(d), fmt);
}

//#if !defined(STRF_OMIT_IMPL)

template <strf::float_notation Notation>
STRF_HD double_printer_data init_double_printer_data
    ( detail::double_dec dd, float_format_data fmt )
{
    static_assert(Notation != strf::float_notation::hex, "");
    double_printer_data data;
    std::memcpy(&data.m10, &dd, sizeof(dd));
    data.showsign = fmt.showpos || data.negative;

    if (data.nan || data.infinity) {
        data.showpoint = false;
        data.sci_notation = false;
        data.m10_digcount = 0;
        data.extra_zeros = 0;
    } else if (fmt.precision == (unsigned)-1) {
        data.m10_digcount = strf::detail::count_digits<10>(data.m10);
        data.extra_zeros = 0;
        STRF_IF_CONSTEXPR (Notation == float_notation::general) {
            data.sci_notation
                = (data.e10 > 4 + (!fmt.showpoint && data.m10_digcount != 1))
               || (data.e10 < ( -(int)data.m10_digcount - 2
                               - (fmt.showpoint || data.m10_digcount != 1) ));
            data.showpoint = fmt.showpoint
                    || (data.sci_notation && data.m10_digcount != 1)
                    || (!data.sci_notation && data.e10 < 0);
        }
        STRF_IF_CONSTEXPR (Notation == float_notation::fixed) {
            data.sci_notation = false;
            data.showpoint = fmt.showpoint || (data.e10 < 0);
        }
        STRF_IF_CONSTEXPR (Notation == float_notation::scientific) {
           data.sci_notation = true;
           data.showpoint = fmt.showpoint || (data.m10_digcount != 1);
        }
    } else {
        data.m10_digcount = strf::detail::count_digits<10>(data.m10);
        int xz; // number of zeros to be added or ( if negative ) digits to be removed
        STRF_IF_CONSTEXPR (Notation == float_notation::general) {
            int p = fmt.precision + (fmt.precision == 0);
            int sci_notation_exp = data.e10 + (int)data.m10_digcount - 1;
            data.sci_notation = (sci_notation_exp < -4 || sci_notation_exp >= p);
            data.showpoint = fmt.showpoint
                || (data.sci_notation && data.m10_digcount != 1)
                || (!data.sci_notation && data.e10 < 0);
            xz = ((unsigned)p < data.m10_digcount || fmt.showpoint)
               * (p - (int)data.m10_digcount);
         }
        STRF_IF_CONSTEXPR (Notation == float_notation::fixed) {
            const int frac_digits = (data.e10 < 0) * -data.e10;
            xz = (fmt.precision - frac_digits);
            data.sci_notation = false;
            data.showpoint = fmt.showpoint || (fmt.precision != 0);
        }
        STRF_IF_CONSTEXPR (Notation == float_notation::scientific) {
            const unsigned frac_digits = data.m10_digcount - 1;
            xz = (fmt.precision - frac_digits);
            data.sci_notation = true;
            data.showpoint = fmt.showpoint || (fmt.precision != 0);
        }
        if (xz < 0) {
            data.extra_zeros = 0;
            unsigned dp = -xz;
            data.m10_digcount -= dp;
            data.e10 += dp;
            auto p10 = strf::detail::pow10(dp);
            auto remainer = data.m10 % p10;
            data.m10 = data.m10 / p10;
            auto middle = p10 >> 1;
            data.m10 += (remainer > middle || (remainer == middle && (data.m10 & 1) == 1));
            STRF_IF_CONSTEXPR (Notation == float_notation::general) {
                while (data.m10 % 10 == 0) {
                    data.m10 /= 10;
                    -- data.m10_digcount;
                    ++ data.e10;
                }
                int frac_digits = data.sci_notation * (data.m10_digcount - 1)
                                - !data.sci_notation * (data.e10 < 0) * data.e10;
                data.showpoint = fmt.showpoint || (frac_digits != 0);
            }
         } else {
            data.extra_zeros = xz;
        }
    }
    return data;
}

//#endif // !defined(STRF_OMIT_IMPL)

template <int Base, std::size_t CharSize, typename IntT>
inline STRF_HD void write_int_with_leading_zeros
    ( strf::underlying_outbuf<CharSize>& ob
    , IntT value
    , unsigned digcount
    , strf::lettercase lc )
{
    ob.ensure(digcount);
    auto p = ob.pointer();
    auto end = p + digcount;
    using writer = detail::intdigits_backwards_writer<Base>;
    auto p2 = writer::write_txtdigits_backwards(value, end, lc);
    if (p != p2) {
        strf::detail::str_fill_n(p, p2 - p, '0');
    }
    ob.advance_to(end);
}

template <std::size_t CharSize>
STRF_HD void print_amplified_integer_small_separator
    ( strf::underlying_outbuf<CharSize>& ob
    , const strf::numpunct_base& punct
    , unsigned long long value
    , unsigned num_digits
    , unsigned num_trailing_zeros
    , strf::underlying_char_type<CharSize> separator )
{
    STRF_ASSERT( ! punct.no_group_separation(num_trailing_zeros + num_digits));

    constexpr std::size_t size_after_recycle = strf::min_size_after_recycle<CharSize>();
    (void) size_after_recycle;

    constexpr auto max_digits = detail::max_num_digits<unsigned long long, 10>();
    char digits_buff[max_digits];
    auto digits = strf::detail::write_int_dec_txtdigits_backwards
        (value, digits_buff + max_digits);

    std::uint8_t groups[std::numeric_limits<double>::max_exponent10 + 1];
    auto num_groups = punct.groups(num_trailing_zeros + num_digits, groups);

    auto grp_it = groups + num_groups - 1;
    unsigned grp_size = *grp_it;
    while (num_digits > grp_size) {
        STRF_ASSERT(grp_size + 1 <= size_after_recycle);
        ob.ensure(grp_size + 1);
        auto it = ob.pointer();
        strf::detail::copy_n(digits, grp_size, it);
        it[grp_size] = separator;
        digits += grp_size;
        ob.advance(grp_size + 1);
        num_digits -= grp_size;
        STRF_ASSERT(grp_it != groups);
        grp_size = *--grp_it;
    }
    if (num_digits != 0) {
        STRF_ASSERT(num_digits <= size_after_recycle);
        ob.ensure(num_digits);
        strf::detail::copy_n(digits, num_digits, ob.pointer());
        ob.advance(num_digits);
    }
    if (grp_size > num_digits) {
        STRF_ASSERT(num_digits <= size_after_recycle);
        grp_size -= num_digits;
        ob.ensure(grp_size);
        strf::detail::str_fill_n(ob.pointer(), grp_size, '0');
        ob.advance(grp_size);
    }
    while (grp_it != groups) {
        grp_size = *--grp_it;
        STRF_ASSERT(grp_size + 1 <= size_after_recycle);
        ob.ensure(grp_size + 1);
        auto it = ob.pointer();
        *it = separator;
        strf::detail::str_fill_n(it + 1, grp_size, '0');
        ob.advance(grp_size + 1);
    }
}

template <std::size_t CharSize>
STRF_HD void print_amplified_integer_big_separator
    ( strf::underlying_outbuf<CharSize>& ob
    , strf::encode_char_f<CharSize> encode_char
    , const strf::numpunct_base& punct
    , unsigned long long value
    , unsigned num_digits
    , unsigned num_trailing_zeros
    , unsigned separator_size )
{
    STRF_ASSERT( ! punct.no_group_separation(num_trailing_zeros + num_digits));
    STRF_ASSERT(separator_size > 1);

    constexpr std::size_t size_after_recycle = strf::min_size_after_recycle<CharSize>();
    (void) size_after_recycle;

    constexpr auto max_digits = detail::max_num_digits<unsigned long long, 10>();
    char digits_buff[max_digits];
    auto digits = strf::detail::write_int_dec_txtdigits_backwards
        (value, digits_buff + max_digits);

    std::uint8_t groups[std::numeric_limits<double>::max_exponent10 + 1];
    auto num_groups = punct.groups(num_trailing_zeros + num_digits, groups);

    auto grp_it = groups + num_groups - 1;
    unsigned grp_size = *grp_it;
    char32_t separator = punct.thousands_sep();
    while (num_digits > grp_size) {
        STRF_ASSERT(grp_size + separator_size <= size_after_recycle);
        ob.ensure(grp_size + separator_size);
        auto it = ob.pointer();
        strf::detail::copy_n(digits, grp_size, it);
        digits += grp_size;
        ob.advance_to(encode_char(it + grp_size, separator));
        num_digits -= grp_size;
        STRF_ASSERT(grp_it != groups);
        grp_size = *--grp_it;
    }
    if (num_digits != 0) {
        STRF_ASSERT(num_digits <= size_after_recycle);
        ob.ensure(num_digits);
        strf::detail::copy_n(digits, num_digits, ob.pointer());
        ob.advance(num_digits);
    }
    if (grp_size > num_digits) {
        STRF_ASSERT(num_digits <= size_after_recycle);
        grp_size -= num_digits;
        ob.ensure(grp_size);
        strf::detail::str_fill_n(ob.pointer(), grp_size, '0');
        ob.advance(grp_size);
    }
    while (grp_it != groups) {
        grp_size = *--grp_it;
        STRF_ASSERT(grp_size + separator_size <= size_after_recycle);
        ob.ensure(grp_size + separator_size);
        auto it = encode_char(ob.pointer(), separator);
        strf::detail::str_fill_n(it, grp_size, '0');
        ob.advance_to(it + grp_size);
    }
}

template <std::size_t CharSize>
STRF_HD void print_scientific_notation
    ( strf::underlying_outbuf<CharSize>& ob
    , strf::encode_char_f<CharSize> encode_char
    , unsigned long long digits
    , unsigned num_digits
    , char32_t decimal_point
    , unsigned decimal_point_size
    , int exponent
    , bool print_point
    , unsigned trailing_zeros
    , strf::lettercase lc )
{
    // digits
    using char_type = strf::underlying_char_type<CharSize>;

    print_point |= num_digits != 1;
    ob.ensure(num_digits + print_point * decimal_point_size);
    if (num_digits == 1) {
        auto it = ob.pointer();
        *it = static_cast<char_type>('0' + digits);
        ++it;
        if (print_point) {
            if (decimal_point_size == 1) {
                *it++ = static_cast<char_type>(decimal_point);
            } else {
                it = encode_char(it, decimal_point);
            }
        }
        ob.advance_to(it);
    } else {
       auto it = ob.pointer();
       auto end = it + num_digits + decimal_point_size;
       *it = *write_int_dec_txtdigits_backwards(digits, end);
       ++it;
       if (decimal_point_size == 1) {
           *it++ = static_cast<char_type>(decimal_point);
       } else {
           encode_char(it, decimal_point);
       }
       ob.advance_to(end);
    }

    // extra trailing zeros

    if (trailing_zeros != 0) {
        strf::detail::write_fill(ob, trailing_zeros, char_type('0'));
    }

    // exponent

    unsigned adv = 4;
    char_type* it;
    unsigned e10u = std::abs(exponent);
    STRF_ASSERT(e10u < 1000);

    if (e10u >= 100) {
        ob.ensure(5);
        it = ob.pointer();
        it[4] = static_cast<char_type>('0' + e10u % 10);
        e10u /= 10;
        it[3] = static_cast<char_type>('0' + e10u % 10);
        it[2] = static_cast<char_type>('0' + e10u / 10);
        adv = 5;
    } else if (e10u >= 10) {
        ob.ensure(4);
        it = ob.pointer();
        it[3] = static_cast<char_type>('0' + e10u % 10);
        it[2] = static_cast<char_type>('0' + e10u / 10);
    } else {
        ob.ensure(4);
        it = ob.pointer();
        it[3] = static_cast<char_type>('0' + e10u);
        it[2] = '0';
    }
    it[0] = 'E' | ((lc != strf::uppercase) << 5);
    it[1] = static_cast<char_type>('+' + ((exponent < 0) << 1));
    ob.advance(adv);
}

template <std::size_t CharSize>
STRF_HD void print_nan(strf::underlying_outbuf<CharSize>& ob, strf::lettercase lc)
{
    ob.ensure(3);
    auto p = ob.pointer();
    switch (lc) {
        case strf::mixedcase:
            p[0] = 'N';
            p[1] = 'a';
            p[2] = 'N';
            break;
        case strf::uppercase:
            p[0] = 'N';
            p[1] = 'A';
            p[2] = 'N';
            break;
        default:
            p[0] = 'n';
            p[1] = 'a';
            p[2] = 'n';
    }
    ob.advance(3);

}
template <std::size_t CharSize>
STRF_HD void print_nan(strf::underlying_outbuf<CharSize>& ob, strf::lettercase lc
                      , bool negative )
{
    ob.ensure(3 + negative);
    auto p = ob.pointer();
    if (negative) {
        *p ++ = '-';
    }
    switch (lc) {
        case strf::mixedcase:
            *p++ = 'N';
            *p++ = 'a';
            *p++ = 'N';
            break;
        case strf::uppercase:
            *p++ = 'N';
            *p++ = 'A';
            *p++ = 'N';
            break;
        default:
            *p++ = 'n';
            *p++ = 'a';
            *p++ = 'n';
    }
    ob.advance_to(p);
}

template <std::size_t CharSize>
STRF_HD void print_inf(strf::underlying_outbuf<CharSize>& ob, strf::lettercase lc)
{
    ob.ensure(3);
    auto p = ob.pointer();
    switch (lc) {
        case strf::mixedcase:
            p[0] = 'I';
            p[1] = 'n';
            p[2] = 'f';
            break;
        case strf::uppercase:
            p[0] = 'I';
            p[1] = 'N';
            p[2] = 'F';
            break;
        default:
            p[0] = 'i';
            p[1] = 'n';
            p[2] = 'f';
    }
    ob.advance(3);
}

template <std::size_t CharSize>
STRF_HD void print_inf( strf::underlying_outbuf<CharSize>& ob
                      , strf::lettercase lc
                      , bool negative )
{
    ob.ensure(3 + negative);
    auto p = ob.pointer();
    if (negative) {
        *p ++ = '-';
    }
    switch (lc) {
        case strf::mixedcase:
            *p++ = 'I';
            *p++ = 'n';
            *p++ = 'f';
            break;
        case strf::uppercase:
            *p++ = 'I';
            *p++ = 'N';
            *p++ = 'F';
            break;
        default:
            *p++ = 'i';
            *p++ = 'n';
            *p++ = 'f';
    }
    ob.advance_to(p);
}

template <std::size_t CharSize>
class punct_double_printer: public strf::printer<CharSize>
{
public:

    using char_type = strf::underlying_char_type<CharSize>;

    template < typename FPack, typename Preview, typename FloatT
             , strf::float_notation Notation, typename CharT>
    STRF_HD punct_double_printer
        ( const strf::detail::fmt_double_printer_input
            < CharT, FPack, Preview, FloatT, Notation, false >& input )
        : punct_(strf::get_facet<strf::numpunct_c<10>, FloatT>(input.fp))
        , inv_seq_poli_(strf::get_facet<strf::invalid_seq_policy_c, FloatT>(input.fp))
        , surr_poli_(strf::get_facet<strf::surrogate_policy_c, FloatT>(input.fp))
        , lettercase_(strf::get_facet<strf::lettercase_c, FloatT>(input.fp))
    {
        static_assert(Notation != strf::float_notation::hex, "");

        const auto fmt = input.vwf.get_float_format_data();
        data_ = strf::detail::init_double_printer_data<Notation>(input.vwf.value(), fmt);
        decltype(auto) cs = get_facet<strf::charset_c<CharT>, FloatT>(input.fp);
        init_(cs, Notation == float_notation::general, fmt.showpoint);
        STRF_IF_CONSTEXPR (Preview::width_required) {
            input.preview.subtract_width(content_width_());
        }
        STRF_IF_CONSTEXPR (Preview::size_required) {
            input.preview.add_size(content_size_());
        }
    }

    template < typename FPack, typename Preview, typename FloatT
             , strf::float_notation Notation, typename CharT >
    STRF_HD punct_double_printer
        ( const strf::detail::fmt_double_printer_input
            < CharT, FPack, Preview, FloatT, Notation, true >& input )
        : punct_(strf::get_facet<strf::numpunct_c<10>, FloatT>(input.fp))
        , fillchar_(input.vwf.fill())
        , inv_seq_poli_(strf::get_facet<strf::invalid_seq_policy_c, FloatT>(input.fp))
        , surr_poli_(strf::get_facet<strf::surrogate_policy_c, FloatT>(input.fp))
        , lettercase_(strf::get_facet<strf::lettercase_c, FloatT>(input.fp))
    {
        static_assert(Notation != strf::float_notation::hex, "");

        const auto fmt = input.vwf.get_float_format_data();
        data_ = strf::detail::init_double_printer_data<Notation>(input.vwf.value(), fmt);
        decltype(auto) cs = get_facet<strf::charset_c<CharT>, FloatT>(input.fp);
        init_(cs, Notation == float_notation::general, fmt.showpoint);
        init_(input.preview, input.vwf.width(), input.vwf.alignment(), cs);
    }


    STRF_HD void print_to(strf::underlying_outbuf<CharSize>&) const override;

private:

    template <typename Charset>
    STRF_HD void init_
        (const Charset& cs, bool fmt_general_format, bool fmt_showpoint);

    template <typename Preview, typename Charset>
    STRF_HD void init_
        ( Preview& preview, std::int16_t w, strf::text_alignment a
        , const Charset& cs );

    STRF_HD std::int16_t content_width_() const;
    STRF_HD std::size_t content_size_() const;

    const strf::numpunct_base& punct_;
    strf::encode_char_f<CharSize> encode_char_;
    strf::encode_fill_f<CharSize> encode_fill_;
    char32_t fillchar_ = U' ';
    unsigned left_fillcount_ = 0;
    unsigned split_fillcount_ = 0;
    unsigned right_fillcount_ = 0;
    unsigned sep_count_ = 0;
    unsigned sep_size_ = 0;
    unsigned decimal_point_size_ = 0;
    char32_t decimal_point_;
    char_type little_sep_;
    strf::invalid_seq_policy inv_seq_poli_;
    strf::surrogate_policy surr_poli_ = surrogate_policy::strict;
    strf::lettercase lettercase_;
    strf::detail::double_printer_data data_;
};

template <std::size_t CharSize>
template <typename Charset>
STRF_HD void punct_double_printer<CharSize>::init_
    ( const Charset& cs, bool general_format, bool fmt_showpoint)
{
    encode_char_ = cs.encode_char_func();
    encode_fill_ = cs.encode_fill_func();
    if (!data_.sci_notation) {
        auto int_dig_count = (int)data_.m10_digcount + data_.e10;
        if (! punct_.no_group_separation(int_dig_count)) {
            auto sep_validation = cs.validate(punct_.thousands_sep());
            if (sep_validation != strf::invalid_char_len) {
                sep_size_ = static_cast<unsigned>(sep_validation);
                sep_count_ = punct_.thousands_sep_count(int_dig_count);
                if (general_format) {
                    bool e10neg = data_.e10 < 0;
                    int fixed_width = data_.e10 * !e10neg
                        + (fmt_showpoint || e10neg) + (int)sep_count_;
                    int scientific_width = 5 + (data_.e10 > 99); // assuming dec point
                    if (scientific_width < fixed_width) {
                        data_.sci_notation = true;
                        data_.showpoint |= data_.m10_digcount != 1;
                        sep_count_ = 0;
                        sep_size_ = 0;
                        goto init_decimal_point;
                    }
                }
                if (sep_size_ == 1) {
                    cs.encode_char(&little_sep_, punct_.thousands_sep());
                }
            }
        }
    }
    init_decimal_point:
    if (data_.showpoint) {

        decimal_point_ = punct_.decimal_point();
        auto validation = cs.validate(decimal_point_);
        if (validation == 1) {
            decimal_point_size_ = 1;
            char_type ch;
            cs.encode_char(&ch, decimal_point_);
            decimal_point_ = ch;
        } else if (validation != strf::invalid_char_len) {
            decimal_point_size_ = static_cast<unsigned>(validation);
        } else {
            decimal_point_size_ = static_cast<unsigned>(cs.replacement_char_size());
            decimal_point_ = cs.replacement_char();
        }
    }
}

template <std::size_t CharSize>
template <typename Preview, typename Charset>
STRF_HD void punct_double_printer<CharSize>::init_
    ( Preview& preview, std::int16_t fmt_width, strf::text_alignment a
    , const Charset& cs )
{
    (void) cs;
    auto content_width = content_width_();
    if (content_width >= fmt_width) {
        preview.subtract_width(content_width);
        STRF_IF_CONSTEXPR (Preview::size_required) {
            preview.add_size(content_size_());
        }
    } else {
        auto fillcount = fmt_width - content_width;
        preview.subtract_width(fmt_width);
        STRF_IF_CONSTEXPR (Preview::size_required) {
            std::size_t fillsize = cs.validate(fillchar_);
            if (fillsize == (size_t)-1) {
                fillsize = cs.replacement_char_size();
            }
            preview.add_size(content_size_() + fillsize * fillcount);
        }
        switch (a) {
            case strf::text_alignment::right:
                left_fillcount_ = fillcount;
                break;
            case strf::text_alignment::left:
                right_fillcount_ = fillcount;
                break;
            case strf::text_alignment::split:
                split_fillcount_ = fillcount;
                break;
            default:
                STRF_ASSERT(a == strf::text_alignment::center);
                left_fillcount_ = fillcount / 2;
                right_fillcount_ = fillcount - left_fillcount_;
        }
    }
}

template <std::size_t CharSize>
STRF_HD std::int16_t punct_double_printer<CharSize>::content_width_() const
{
    int decpoint_width = data_.showpoint;
    unsigned w = 0;
    if (data_.infinity || data_.nan) {
        w = 3 + data_.showsign;
    } else if (data_.sci_notation) {
        unsigned e10u = std::abs(data_.e10 + (int)data_.m10_digcount - 1);
        w = data_.m10_digcount + data_.extra_zeros
            + data_.showsign
            + (e10u < 10) + 2
            + detail::count_digits<10>(e10u)
            + decpoint_width;
    } else {
        if (data_.e10 <= -(int)data_.m10_digcount) {
            w = data_.showsign + 1 + decpoint_width
                    - data_.e10 + data_.extra_zeros;
        } else {
            auto idigcount = (int)data_.m10_digcount + data_.e10;
            if (data_.e10 < 0) {
                    w = data_.showsign
                        + (int)data_.m10_digcount
                        + data_.extra_zeros
                        + 1 // decpoint_width
                        + sep_count_;
            } else {
                w = data_.showsign
                    + idigcount
                    + data_.extra_zeros
                    + data_.showpoint
                    + sep_count_;
            }
        }
    }
    return static_cast<std::int16_t>(w);
}

template <std::size_t CharSize>
STRF_HD std::size_t punct_double_printer<CharSize>::content_size_() const
{
    if (data_.infinity || data_.nan) {
        return 3 + data_.showsign;
    }
    if (data_.sci_notation) {
        unsigned e10u = std::abs(data_.e10 + (int)data_.m10_digcount - 1);
        return data_.m10_digcount + data_.extra_zeros
            + data_.showsign
            + (e10u < 10) + 2
            + detail::count_digits<10>(e10u)
            + decimal_point_size_;
    }
    if (data_.e10 <= -(int)data_.m10_digcount) {
        return 1 + data_.showsign + decimal_point_size_
            + (-data_.e10) +data_.extra_zeros;
    }
    return data_.showsign + sep_count_ * sep_size_ + decimal_point_size_
        + data_.m10_digcount + data_.extra_zeros + (data_.e10 > 0) * data_.e10;
}

template <std::size_t CharSize>
STRF_HD void punct_double_printer<CharSize>::print_to
    (strf::underlying_outbuf<CharSize>& ob) const
{
    if (left_fillcount_ != 0) {
        encode_fill_(ob, left_fillcount_, fillchar_, inv_seq_poli_, surr_poli_);
    }
    if (data_.showsign) {
        put(ob, static_cast<char_type>('+' + (data_.negative << 1)));
    }
    if (split_fillcount_ != 0) {
        encode_fill_(ob, split_fillcount_, fillchar_, inv_seq_poli_, surr_poli_);
    }
    if (data_.nan) {
        strf::detail::print_nan(ob, lettercase_);
    } else if (data_.infinity) {
        strf::detail::print_inf(ob, lettercase_);
    } else if (data_.sci_notation) {
        strf::detail::print_scientific_notation
            ( ob, encode_char_, data_.m10, data_.m10_digcount
            , decimal_point_, decimal_point_size_
            , data_.e10 + data_.m10_digcount - 1
            , data_.showpoint, data_.extra_zeros, lettercase_ );
    } else if (data_.e10 >= 0) {
        if (sep_count_ == 0) {
            strf::detail::write_int<10>( ob, data_.m10, data_.m10_digcount
                                       , strf::lowercase );
            strf::detail::write_fill(ob, data_.e10, (char_type)'0');
        } else if (sep_size_ == 1) {
            strf::detail::print_amplified_integer_small_separator
                ( ob, punct_, data_.m10, data_.m10_digcount, data_.e10
                , little_sep_ );
        } else {
            strf::detail::print_amplified_integer_big_separator
                ( ob, encode_char_, punct_, data_.m10
                , data_.m10_digcount, data_.e10, sep_size_ );
        }
        if (decimal_point_size_ == 1) {
            strf::put(ob, static_cast<char_type>(decimal_point_));
        } else if (decimal_point_size_ != 0) {
            ob.ensure(decimal_point_size_);
            ob.advance_to(encode_char_(ob.pointer(), decimal_point_));
        }
        if (data_.extra_zeros) {
            detail::write_fill(ob, data_.extra_zeros,  (char_type)'0');
        }
    } else {
        STRF_ASSERT(data_.e10 < 0);

        unsigned e10u = - data_.e10;
        if (e10u >= data_.m10_digcount) {
            ob.ensure(1 + decimal_point_size_);
            auto it = ob.pointer();
            *it++ = static_cast<char_type>('0');
            if (decimal_point_size_ == 1) {
                *it++ = static_cast<char_type>(decimal_point_);
            } else {
                STRF_ASSERT(decimal_point_size_ != 0);
                it = encode_char_(it, decimal_point_);
            }
            ob.advance_to(it);

            if (e10u > data_.m10_digcount) {
                strf::detail::write_fill(ob, e10u - data_.m10_digcount, (char_type)'0');
            }
            strf::detail::write_int<10>( ob, data_.m10, data_.m10_digcount
                                       , strf::lowercase);
            if (data_.extra_zeros != 0) {
                strf::detail::write_fill(ob, data_.extra_zeros,  (char_type)'0');
            }
        } else {
            //auto v = std::lldiv(data_.m10, detail::pow10(e10u)); // todo test this
            auto p10 = strf::detail::pow10(e10u);
            auto integral_part = data_.m10 / p10;
            auto fractional_part = data_.m10 % p10;
            auto idigcount = data_.m10_digcount - e10u;

            STRF_ASSERT(idigcount == detail::count_digits<10>(integral_part));

            if (sep_count_ == 0) {
                strf::detail::write_int<10>(ob, integral_part, idigcount, strf::lowercase);
            } else if (sep_size_ == 1) {
                strf::detail::write_int_little_sep<10>
                    ( ob, punct_, integral_part, idigcount, little_sep_);
            } else {
                strf::detail::write_int_big_sep<10>
                    (  ob, punct_, encode_char_, integral_part
                    , sep_size_, idigcount );
            }

            ob.ensure(decimal_point_size_);
            auto it = ob.pointer();
            if (decimal_point_size_ == 1) {
                *it++ = static_cast<char_type>(decimal_point_);
            } else {
                STRF_ASSERT(decimal_point_size_ != 0);
                it = encode_char_(it, decimal_point_);
            }
            ob.advance_to(it);

            strf::detail::write_int_with_leading_zeros<10>
                (ob, fractional_part, e10u, strf::lowercase);
            if (data_.extra_zeros) {
                detail::write_fill(ob, data_.extra_zeros,  (char_type)'0');
            }
        }
    }
    if (right_fillcount_ != 0) {
        encode_fill_(ob, right_fillcount_, fillchar_, inv_seq_poli_, surr_poli_);
    }
}

template <std::size_t CharSize>
class double_printer final: public strf::printer<CharSize>
{
public:

    using char_type = strf::underlying_char_type<CharSize>;

    template < typename FPack, typename Preview, typename FloatT
             , strf::float_notation Notation, typename CharT >
    STRF_HD double_printer
        ( const strf::detail::fmt_double_printer_input
            < CharT, FPack, Preview, FloatT, Notation, false >& input )
        : data_( strf::detail::init_double_printer_data<Notation>
                    ( input.vwf.value(), input.vwf.get_float_format_data() ) )
        , lettercase_(strf::get_facet<strf::lettercase_c, FloatT>(input.fp))
    {
        static_assert(Notation != strf::float_notation::hex, "");

        auto content_width = content_width_();
        input.preview.subtract_width(content_width);
        input.preview.add_size(content_width);
    }

    template < typename FPack, typename Preview, typename FloatT
             , strf::float_notation Notation, typename CharT >
    STRF_HD double_printer
        ( const strf::detail::fmt_double_printer_input
            < CharT, FPack, Preview, FloatT, Notation, true >& input )
        : data_( strf::detail::init_double_printer_data<Notation>
                    ( input.vwf.value(), input.vwf.get_float_format_data() ) )
        , fillchar_(input.vwf.fill())
        , inv_seq_poli_(strf::get_facet<strf::invalid_seq_policy_c, FloatT>(input.fp))
        , surr_poli_(strf::get_facet<strf::surrogate_policy_c, FloatT>(input.fp))
        , lettercase_(strf::get_facet<strf::lettercase_c, FloatT>(input.fp))
    {
        static_assert(Notation != strf::float_notation::hex, "");

        decltype(auto) cs = strf::get_facet<strf::charset_c<CharT>, FloatT>(input.fp);
        init_(input.preview, input.vwf.width(), input.vwf.alignment(), cs);
    }

    STRF_HD void print_to(strf::underlying_outbuf<CharSize>&) const override;

private:

    template <typename Preview, typename Charset>
    STRF_HD void init_( Preview& preview, std::int16_t w, strf::text_alignment a
                      , const Charset& cs );

    STRF_HD std::int16_t content_width_() const
    {
        return static_cast<std::int16_t>
               ( data_.nan * 3
               + data_.infinity * 3
               + data_.showsign
               + !(data_.infinity | data_.nan)
               * ( data_.extra_zeros
                 + data_.showpoint
                 + data_.m10_digcount
                 + ( data_.sci_notation
                   * ( 4 + ((data_.e10 > 99) || (data_.e10 < -99))) )
                 + ( !data_.sci_notation
                   * ( (0 <= data_.e10)
                     * data_.e10
                     + (data_.e10 <= -(int)data_.m10_digcount)
                       * (-data_.e10 + 1 -(int)data_.m10_digcount) ))));
    }

    strf::detail::double_printer_data data_;
    strf::encode_fill_f<CharSize> encode_fill_;
    char32_t fillchar_ = U' ';
    unsigned left_fillcount_ = 0;
    unsigned split_fillcount_ = 0;
    unsigned right_fillcount_ = 0;
    strf::invalid_seq_policy inv_seq_poli_ = invalid_seq_policy::replace;
    strf::surrogate_policy surr_poli_ = surrogate_policy::strict;
    strf::lettercase lettercase_;
};

template <std::size_t CharSize>
template <typename Preview, typename Charset>
STRF_HD void double_printer<CharSize>::init_
    ( Preview& preview, std::int16_t w, strf::text_alignment a
    , const Charset& cs )
{
    encode_fill_ = cs.encode_fill_func();
    auto content_width = content_width_();
    if (content_width >= w) {
        preview.checked_subtract_width(content_width);
        preview.add_size(content_width);
    } else {
        auto fillcount = (w - static_cast<std::int16_t>(content_width));
        preview.subtract_width(w);
        STRF_IF_CONSTEXPR(Preview::size_required) {
            std::size_t fillchar_size = cs.validate(fillchar_);
            if (fillchar_size == (size_t)-1) {
                fillchar_size = cs.replacement_char_size();
            }
            preview.add_size(content_width + fillchar_size * fillcount);
        }
        switch (a) {
            case strf::text_alignment::right:
                left_fillcount_ = fillcount;
                break;
            case strf::text_alignment::left:
                right_fillcount_ = fillcount;
                break;
            case strf::text_alignment::split:
                split_fillcount_ = fillcount;
                break;
            default:
                STRF_ASSERT(a == strf::text_alignment::center);
                left_fillcount_ = fillcount / 2;
                right_fillcount_ = fillcount - left_fillcount_;
        }
    }
}

template <std::size_t CharSize>
STRF_HD void double_printer<CharSize>::print_to
    ( strf::underlying_outbuf<CharSize>& ob ) const
{
    if (left_fillcount_ != 0) {
        encode_fill_(ob, left_fillcount_, fillchar_, inv_seq_poli_, surr_poli_);
    }
    if (data_.showsign) {
        put<CharSize>(ob, '+' + (data_.negative << 1));
    }
    if (split_fillcount_ != 0) {
        encode_fill_(ob, split_fillcount_, fillchar_, inv_seq_poli_, surr_poli_);
    }
    if (data_.nan) {
        strf::detail::print_nan(ob, lettercase_);
    } else if (data_.infinity) {
        strf::detail::print_inf(ob, lettercase_);
    } else if (data_.sci_notation) {
        ob.ensure( data_.m10_digcount
                 + data_.showpoint
                 + 4 + (data_.e10 > 99 || data_.e10 < -99) );
        char_type* it = ob.pointer();
        if (data_.m10_digcount == 1) {
            * it = static_cast<char_type>('0' + data_.m10);
            ++it;
            if (data_.showpoint) {
                *it = '.';
                ++it;
            }
            if (data_.extra_zeros > 0) {
                ob.advance_to(it);
                strf::detail::write_fill<CharSize>(ob, data_.extra_zeros, '0');
                it = ob.pointer();
            }
        } else {
            auto itz = it + data_.m10_digcount + 1;
            write_int_dec_txtdigits_backwards(data_.m10, itz);
            it[0] = it[1];
            it[1] = '.';
            it = itz;
            if (data_.extra_zeros > 0) {
                ob.advance_to(itz);
                strf::detail::write_fill<CharSize>(ob, data_.extra_zeros, '0');
                it = ob.pointer();
            }
        }
        auto e10 = data_.e10 - 1 + (int)data_.m10_digcount;
        it[0] = 'E' | ((lettercase_ != strf::uppercase) << 5);
        it[1] = static_cast<char_type>('+' + ((e10 < 0) << 1));
        unsigned e10u = std::abs(e10);
        if (e10u >= 100) {
            it[4] = static_cast<char_type>('0' + e10u % 10);
            e10u /= 10;
            it[3] = static_cast<char_type>('0' + e10u % 10);
            it[2] = static_cast<char_type>('0' + e10u / 10);
            it += 5;
        } else if (e10u >= 10) {
            it[3] = static_cast<char_type>('0' + e10u % 10);
            it[2] = static_cast<char_type>('0' + e10u / 10);
            it += 4;
        } else {
            it[3] = static_cast<char_type>('0' + e10u);
            it[2] = '0';
            it += 4;
        }
        ob.advance_to(it);
    } else {
        ob.ensure( data_.showpoint + data_.m10_digcount
                 + (data_.e10 < -(int)data_.m10_digcount) );
        auto it = ob.pointer();
        if (data_.e10 >= 0) {
            it += data_.m10_digcount;
            write_int_dec_txtdigits_backwards(data_.m10, it);
            ob.advance_to(it);
            detail::write_fill(ob, data_.e10, (char_type)'0');
            if (data_.showpoint) {
                ob.ensure(1);
                *ob.pointer() = '.';
                ob.advance();
            }
            detail::write_fill(ob, data_.extra_zeros, (char_type)'0');
        } else {
            unsigned e10u = - data_.e10;
            if (e10u >= data_.m10_digcount) {
                it[0] = '0';
                it[1] = '.';
                ob.advance_to(it + 2);
                detail::write_fill(ob, e10u - data_.m10_digcount, (char_type)'0');

                ob.ensure(data_.m10_digcount);
                auto end = ob.pointer() + data_.m10_digcount;
                write_int_dec_txtdigits_backwards(data_.m10, end);
                ob.advance_to(end);
                detail::write_fill(ob, data_.extra_zeros, (char_type)'0');
            } else {
                const char* const arr = strf::detail::chars_00_to_99();
                auto m = data_.m10;
                char_type* const end = it + data_.m10_digcount + 1;
                it = end;
                while(e10u >= 2) {
                    auto index = (m % 100) << 1;
                    it[-2] = arr[index];
                    it[-1] = arr[index + 1];
                    it -= 2;
                    m /= 100;
                    e10u -= 2;
                }
                if (e10u != 0) {
                    *--it = static_cast<char_type>('0' + (m % 10));
                    m /= 10;
                }
                * --it = '.';
                while(m > 99) {
                    auto index = (m % 100) << 1;
                    it[-2] = arr[index];
                    it[-1] = arr[index + 1];
                    it -= 2;
                    m /= 100;
                }
                if (m > 9) {
                    auto index = m << 1;
                    it[-2] = arr[index];
                    it[-1] = arr[index + 1];
                } else {
                    *--it = static_cast<char_type>('0' + m);
                }
                ob.advance_to(end);
                detail::write_fill(ob, data_.extra_zeros, (char_type)'0');
            }
        }
    }
    if (right_fillcount_ != 0) {
        encode_fill_(ob, right_fillcount_, fillchar_, inv_seq_poli_, surr_poli_);
    }
}

template <std::size_t CharSize>
class fast_double_printer: public strf::printer<CharSize>
{
public:

    using char_type = strf::underlying_char_type<CharSize>;

    template <typename CharT, typename Preview, typename FloatT>
    STRF_HD fast_double_printer
        ( strf::detail::fast_double_printer_input<CharT, Preview, FloatT> input) noexcept
        : fast_double_printer(input.value, input.lcase)
    {
        std::size_t s = 0;
        STRF_IF_CONSTEXPR (Preview::width_required || Preview::size_required) {
            s = size();
        }
        input.preview.checked_subtract_width(s);
        input.preview.add_size(s);
    }

    STRF_HD fast_double_printer(float f, strf::lettercase lc) noexcept
        : value_(decode(f))
        , m10_digcount_(strf::detail::count_digits<10>(value_.m10))
        , lettercase_(lc)

    {
        STRF_ASSERT(!value_.nan || !value_.infinity);
        sci_notation_ = (value_.e10 > 4 + (m10_digcount_ != 1))
            || (value_.e10 < -(int)m10_digcount_ - 2 - (m10_digcount_ != 1));
    }

    STRF_HD fast_double_printer(double d, strf::lettercase lc) noexcept
        : value_(decode(d))
        , m10_digcount_(strf::detail::count_digits<10>(value_.m10))
        , lettercase_(lc)

    {
        STRF_ASSERT(!value_.nan || !value_.infinity);
        sci_notation_ = (value_.e10 > 4 + (m10_digcount_ != 1))
            || (value_.e10 < -(int)m10_digcount_ - 2 - (m10_digcount_ != 1));
    }

    STRF_HD void print_to(strf::underlying_outbuf<CharSize>&) const override;

    STRF_HD std::size_t size() const;

private:

    const detail::double_dec value_;
    bool sci_notation_ ;
    const unsigned m10_digcount_;
    strf::lettercase lettercase_;
};

template <std::size_t CharSize>
STRF_HD std::size_t fast_double_printer<CharSize>::size() const
{
    return ( value_.nan * 3
           + (value_.infinity * 3)
           + value_.negative
           + !(value_.infinity | value_.nan)
           * ( ( sci_notation_
               * ( 4 // e+xx
                 + (m10_digcount_ != 1) // decimal point
                 + m10_digcount_
                 + ((value_.e10 > 99) || (value_.e10 < -99))) )
             + ( !sci_notation_
               * ( (int)m10_digcount_
                 + (value_.e10 > 0) * value_.e10 // trailing zeros
                 + (value_.e10 <= -(int)m10_digcount_) * (2 -value_.e10 - (int)m10_digcount_) // leading zeros and point
                 + (-(int)m10_digcount_ < value_.e10 && value_.e10 < 0) ))));
}

template <std::size_t CharSize>
STRF_HD void fast_double_printer<CharSize>::print_to
    ( strf::underlying_outbuf<CharSize>& ob ) const
{
    if (value_.nan) {
        strf::detail::print_nan(ob, lettercase_, value_.negative);
    } else if (value_.infinity) {
        strf::detail::print_inf(ob, lettercase_, value_.negative);
    } else if (sci_notation_) {
        ob.ensure( value_.negative + m10_digcount_ + (m10_digcount_ != 1) + 4
                 + (value_.e10 > 99 || value_.e10 < -99) );
        char_type* it = ob.pointer();
        if (value_.negative) {
            * it = '-';
            ++it;
        }
        if (m10_digcount_ == 1) {
            * it = static_cast<char_type>('0' + value_.m10);
            ++ it;
        } else {
            auto next = it + m10_digcount_ + 1;
            write_int_dec_txtdigits_backwards(value_.m10, next);
            it[0] = it[1];
            it[1] = '.';
            it = next;
        }
        auto e10 = value_.e10 - 1 + (int)m10_digcount_;
        it[0] = 'E' | ((lettercase_ != strf::uppercase) << 5);
        it[1] = static_cast<char_type>('+' + ((e10 < 0) << 1));
        unsigned e10u = std::abs(e10);
        if (e10u >= 100) {
            it[4] = static_cast<char_type>('0' + e10u % 10);
            e10u /= 10;
            it[3] = static_cast<char_type>('0' + e10u % 10);
            it[2] = static_cast<char_type>('0' + e10u / 10);
            it += 5;
        } else if (e10u >= 10) {
            it[3] = static_cast<char_type>('0' + e10u % 10);
            it[2] = static_cast<char_type>('0' + e10u / 10);
            it += 4;
        } else {
            it[3] = static_cast<char_type>('0' + e10u);
            it[2] = '0';
            it += 4;
        }
        ob.advance_to(it);
    } else {
        ob.ensure( value_.negative
                 + m10_digcount_ * (value_.e10 > - (int)m10_digcount_)
                 + (value_.e10 < - (int)m10_digcount_)
                 + (value_.e10 < 0) );
        auto it = ob.pointer();
        if (value_.negative) {
            *it = '-';
            ++it;
        }
        if (value_.e10 >= 0) {
            it += m10_digcount_;
            write_int_dec_txtdigits_backwards(value_.m10, it);
            ob.advance_to(it);
            if (value_.e10 != 0) {
                detail::write_fill(ob, value_.e10, (char_type)'0');
            }
        } else {
            unsigned e10u = - value_.e10;
            if (e10u >= m10_digcount_) {
                it[0] = '0';
                it[1] = '.';
                ob.advance_to(it + 2);
                detail::write_fill(ob, e10u - m10_digcount_, (char_type)'0');

                ob.ensure(m10_digcount_);
                auto end = ob.pointer() + m10_digcount_;
                write_int_dec_txtdigits_backwards(value_.m10, end);
                ob.advance_to(end);
            } else {
                const char* const arr = strf::detail::chars_00_to_99();
                auto m = value_.m10;
                it += m10_digcount_ + 1;
                char_type* const end = it;
                while(e10u >= 2) {
                    auto index = (m % 100) << 1;
                    it[-2] = arr[index];
                    it[-1] = arr[index + 1];
                    it -= 2;
                    m /= 100;
                    e10u -= 2;
                }
                if (e10u != 0) {
                    *--it = static_cast<char_type>('0' + (m % 10));
                    m /= 10;
                }
                * --it = '.';
                while(m > 99) {
                    auto index = (m % 100) << 1;
                    it[-2] = arr[index];
                    it[-1] = arr[index + 1];
                    it -= 2;
                    m /= 100;
                }
                if (m > 9) {
                    auto index = m << 1;
                    it[-2] = arr[index];
                    it[-1] = arr[index + 1];
                } else {
                    *--it = static_cast<char_type>('0' + m);
                }
                ob.advance_to(end);
            }
        }
    }
}


template <std::size_t CharSize>
class fast_punct_double_printer: public strf::printer<CharSize>
{
public:

    using char_type = strf::underlying_char_type<CharSize>;

    template <typename CharT, typename FPack, typename Preview, typename FloatT>
    STRF_HD fast_punct_double_printer
        ( const strf::detail::fast_punct_double_printer_input
              < CharT, FPack, Preview, FloatT >& input )
        : punct_(strf::get_facet<strf::numpunct_c<10>, FloatT>(input.fp))
        , value_(decode(input.value))
        , m10_digcount_(strf::detail::count_digits<10>(value_.m10))
        , sep_count_(0)
        , lettercase_(strf::get_facet<strf::lettercase_c, FloatT>(input.fp))
    {
        init_(strf::get_facet<strf::charset_c<CharT>, FloatT>(input.fp));
        STRF_IF_CONSTEXPR (Preview::width_required) {
            input.preview.subtract_width(width_());
        }
        STRF_IF_CONSTEXPR (Preview::size_required) {
            input.preview.add_size(size_());
        }
    }


    STRF_HD void print_to(strf::underlying_outbuf<CharSize>&) const override;

private:

    template <typename Charset>
    STRF_HD void init_(const Charset& cs);

    STRF_HD strf::width_t width_() const;
    STRF_HD std::size_t size_() const;

    const strf::numpunct_base& punct_;
    strf::encode_char_f<CharSize> encode_char_;
    const detail::double_dec value_;
    const unsigned m10_digcount_;
    unsigned sep_count_ = 0;
    unsigned sep_size_ = 0;
    unsigned decimal_point_size_ = 0;
    char32_t decimal_point_ = '.';
    char_type little_sep_;
    strf::lettercase lettercase_;
    bool sci_notation_ ;

};

template <std::size_t CharSize>
template <typename Charset>
STRF_HD void fast_punct_double_printer<CharSize>::init_(const Charset& cs)
{
    encode_char_ = cs.encode_char_func();
    bool showpoint;
    if (value_.e10 > -(int)m10_digcount_) {
        bool e10neg = value_.e10 < 0;
        int fixed_width = value_.e10 * !e10neg  + e10neg + (int)sep_count_;
        int scientific_width = 4 + (value_.e10 > 99) + (m10_digcount_ != 1);
        if (scientific_width < fixed_width) {
            sci_notation_ = true;
            showpoint = m10_digcount_ != 1;
        } else {
            auto int_dig_count = (int)m10_digcount_ + value_.e10;
            if (! punct_.no_group_separation(int_dig_count)) {
                auto sep_validation = cs.validate(punct_.thousands_sep());
                if (sep_validation != strf::invalid_char_len) {
                    sep_count_ = punct_.thousands_sep_count(int_dig_count);
                    if (scientific_width < fixed_width + (int)sep_count_) {
                        sep_count_ = 0;
                        sci_notation_ = true;
                        showpoint = m10_digcount_ != 1;
                        goto init_decimal_point;
                    }
                    sep_size_ = static_cast<unsigned>(sep_validation);
                    if (sep_size_ == 1) {
                        encode_char_(&little_sep_, punct_.thousands_sep());
                    }
                }
            }
            showpoint = value_.e10 < 0;
            sci_notation_ = false;
        }
    } else {
        sep_count_ = 0;
        int tmp = m10_digcount_ + 2 + (value_.e10 < -99)
            + (m10_digcount_ != 1);
        sci_notation_ = -value_.e10 > tmp;
        showpoint = m10_digcount_ != 1 || !sci_notation_;
    }
    init_decimal_point:
    if (showpoint) {
        decimal_point_ = punct_.decimal_point();
        auto validation = cs.validate(decimal_point_);
        if (validation == 1) {
            decimal_point_size_ = 1;
            char_type ch;
            cs.encode_char(&ch, decimal_point_);
            decimal_point_ = ch;
        } else if (validation != strf::invalid_char_len) {
            decimal_point_size_ = static_cast<unsigned>(validation);
        } else {
            decimal_point_size_ = static_cast<unsigned>(cs.replacement_char_size());
            decimal_point_ = cs.replacement_char();
        }
    }
}


template <std::size_t CharSize>
STRF_HD std::size_t fast_punct_double_printer<CharSize>::size_() const
{
    if (value_.infinity || value_.nan) {
        return 3 + value_.negative;
    }
    if (sci_notation_) {
        unsigned e10u = std::abs(value_.e10 + (int)m10_digcount_ - 1);
        return m10_digcount_
            + value_.negative
            + (e10u < 10) + 2
            + detail::count_digits<10>(e10u)
            + decimal_point_size_;
    }
    if (value_.e10 <= -(int)m10_digcount_) {
        return 1 + decimal_point_size_ + (-value_.e10);
    }
    return sep_count_ * sep_size_ + decimal_point_size_ + m10_digcount_
        + value_.negative + (value_.e10 > 0) * value_.e10;
}

template <std::size_t CharSize>
STRF_HD strf::width_t fast_punct_double_printer<CharSize>::width_() const
{
    if (value_.infinity || value_.nan) {
        return static_cast<std::int16_t>(3 + value_.negative);
    }
    constexpr unsigned decpoint_width = 1;
    if (sci_notation_) {
        unsigned e10u = std::abs(value_.e10 + (int)m10_digcount_ - 1);
        auto w = m10_digcount_
            + value_.negative
            + (e10u < 10) + 2
            + detail::count_digits<10>(e10u)
            + decpoint_width * (m10_digcount_ != 1);
        return static_cast<std::int16_t>(w);
    }
    if (value_.e10 <= -(int)m10_digcount_) {
        return static_cast<std::int16_t>
            (value_.negative + 1 - value_.e10 +  decpoint_width);
    }
    auto sep_w = sep_size_ * sep_count_;
    auto idigcount = (int)m10_digcount_ + value_.e10;
    if (value_.e10 < 0) {
        auto w = value_.negative + m10_digcount_ + decpoint_width + sep_w;
        return static_cast<std::int16_t>(w);
    }
    return static_cast<std::int16_t>(value_.negative + idigcount + sep_w);
}

template <std::size_t CharSize>
STRF_HD void fast_punct_double_printer<CharSize>::print_to
    ( strf::underlying_outbuf<CharSize>& ob ) const
{
    if (value_.negative) {
        put(ob, static_cast<char_type>('-'));
    }
    if (value_.nan) {
        strf::detail::print_nan(ob, lettercase_);
    } else if (value_.infinity) {
        strf::detail::print_inf(ob, lettercase_);
    } else if (sci_notation_) {
        strf::detail::print_scientific_notation
            ( ob, encode_char_, value_.m10, m10_digcount_
            , decimal_point_, decimal_point_size_
            , value_.e10 + m10_digcount_ - 1
            , false, 0, lettercase_ );
    } else {
        if (value_.e10 >= 0) {
            if (sep_count_ == 0) {
                strf::detail::write_int<10>( ob, value_.m10, m10_digcount_
                                           , strf::lowercase);
                strf::detail::write_fill(ob, value_.e10, (char_type)'0');
            } else if (sep_size_ == 1) {
                strf::detail::print_amplified_integer_small_separator
                    ( ob, punct_, value_.m10, m10_digcount_, value_.e10
                    , little_sep_ );
            } else {
                strf::detail::print_amplified_integer_big_separator
                    ( ob, encode_char_, punct_, value_.m10
                    , m10_digcount_, value_.e10, sep_size_ );
            }
        } else {
            unsigned e10u = - value_.e10;
            if (e10u >= m10_digcount_) {
                ob.ensure(1 + decimal_point_size_);
                auto it = ob.pointer();
                *it = static_cast<char_type>('0');
                if (decimal_point_size_ == 1) {
                    it[1] = static_cast<char_type>(decimal_point_);
                    ob.advance_to(it + 2);
                } else {
                    ob.advance_to(encode_char_(it + 1, decimal_point_));
                }
                if (e10u > m10_digcount_) {
                    strf::detail::write_fill(ob, e10u - m10_digcount_, (char_type)'0');
                }
                strf::detail::write_int<10>( ob, value_.m10, m10_digcount_
                                           , strf::lowercase );
            } else {
                //auto v = std::lldiv(value_.m10, detail::pow10(e10u)); // todo test this
                auto p10 = strf::detail::pow10(e10u);
                auto integral_part = value_.m10 / p10;
                auto fractional_part = value_.m10 % p10;
                auto idigcount = m10_digcount_ - e10u;
                STRF_ASSERT(idigcount == detail::count_digits<10>(integral_part));
                if (sep_count_ == 0) {
                    strf::detail::write_int<10>( ob, integral_part, idigcount
                                               , strf::lowercase );
                } else if (sep_size_ == 1) {
                    strf::detail::write_int_little_sep<10>
                        ( ob, punct_, integral_part, idigcount
                        , little_sep_, strf::lowercase);
                } else {
                    strf::detail::write_int_big_sep<10>
                        ( ob, punct_, encode_char_, integral_part
                        , sep_size_, idigcount, strf::lowercase );
                }
                ob.ensure(decimal_point_size_);
                if (decimal_point_size_ == 1) {
                    *ob.pointer() = static_cast<char_type>(decimal_point_);
                } else {
                    encode_char_(ob.pointer(), decimal_point_);
                }
                ob.advance(decimal_point_size_);

                strf::detail::write_int_with_leading_zeros<10>
                    (ob, fractional_part, e10u, strf::lowercase);
            }
        }
    }
}

inline STRF_HD unsigned exponent_hex_digcount(std::uint32_t abs_exponent)
{
    return 1 + (abs_exponent >= 1000) + (abs_exponent >= 100) + (abs_exponent >= 10);
}

inline STRF_HD unsigned mantissa_hex_digcount(std::uint64_t mantissa)
{
    STRF_ASSERT(mantissa == (mantissa & 0xFFFFFFFFFFFFFull));
    if (mantissa == 0) {
        return 0;
    }
#if defined(__cpp_lib_bitops)
    unsigned lz = std::countl_zero(mantissa) >> 3
#else
    unsigned lz = 0;
    if ((mantissa & 0xFFFFFFFFull) == 0) {
        lz += 8;
        mantissa = mantissa >> 32;
    }
    if ((mantissa & 0xFFFFull) == 0) {
        lz += 4;
        mantissa = mantissa >> 16;
    }
    if ((mantissa & 0xFFull) == 0) {
        lz += 2;
        mantissa = mantissa >> 8;
    }
    if ((mantissa & 0xFull) == 0) {
        lz += 1;
    }
#endif
    return 13 - lz;
}

struct hex_double_printer_data
{
    std::uint64_t mantissa;
    std::int32_t exponent;
    unsigned exponent_digcount = 0;
    unsigned mantissa_digcount = 0;
    unsigned extra_zeros = 0;
    bool showpoint = false;
    bool negative;
    bool showsign;
};

#if ! defined(STRF_OMIT_IMPL)

STRF_INLINE STRF_HD strf::detail::hex_double_printer_data init_hex_double_printer_data
    ( float_format_data fmt, double x ) noexcept
{
    strf::detail::hex_double_printer_data data;

    std::uint64_t bits;
    std::memcpy(&bits, &x, 8);

    data.mantissa = bits & 0xFFFFFFFFFFFFFull;
    data.exponent = static_cast<std::int32_t>((bits << 1) >> 53) - 1023;
    data.negative = bits & (1ull << 63);
    data.showsign = data.negative || fmt.showpos;
    if (data.exponent != 1024) {
        if ((bits & 0x7FFFFFFFFFFFFFFFull) == 0) {
            data.exponent_digcount = 1;
            data.mantissa_digcount = 0;
            data.extra_zeros = (fmt.precision != (unsigned)-1) * fmt.precision;
            data.showpoint = data.extra_zeros || fmt.showpoint;
        } else {
            data.exponent_digcount = strf::detail::exponent_hex_digcount(std::abs(data.exponent));
            if (data.mantissa == 0){
                data.mantissa_digcount = 0;
                data.extra_zeros = (fmt.precision != (unsigned)-1) * fmt.precision;
                data.showpoint = data.extra_zeros || fmt.showpoint;
            } else if (fmt.precision == (unsigned)-1) {
                data.mantissa_digcount = strf::detail::mantissa_hex_digcount(data.mantissa);
                data.extra_zeros = 0;
                data.showpoint = true;
            } else {
                data.mantissa_digcount = strf::detail::mantissa_hex_digcount(data.mantissa);
                if (fmt.precision >= data.mantissa_digcount) {
                    data.extra_zeros = fmt.precision - data.mantissa_digcount;
                    data.showpoint = true;
                } else {
                    // round mantissa if necessary
                    unsigned s = (13 - fmt.precision) << 2;
                    auto d = 1ull << s;
                    auto mask = d - 1;
                    auto mantissa_low = data.mantissa & mask;
                    if ( mantissa_low > (d >> 1)) {
                        data.mantissa += d;
                    } else if (mantissa_low == (d >> 1) && (data.mantissa & d)) {
                        data.mantissa += d;
                    }
                    data.mantissa_digcount = fmt.precision;
                    data.showpoint = fmt.precision || fmt.showpoint;
                }
            }
        }
    }
    return data;
}

#else // ! defined(STRF_OMIT_IMPL)

STRF_HD strf::detail::hex_double_printer_data init_hex_double_printer_data
    ( float_format_data fmt, double d ) noexcept;

#endif // ! defined(STRF_OMIT_IMPL)

template <std::size_t CharSize>
class hex_double_printer: public strf::printer<CharSize>
{
public:
    using char_type = strf::underlying_char_type<CharSize>;

    template <typename CharT, typename FPack, typename Preview, typename FloatT>
    hex_double_printer
        ( const strf::detail::fmt_double_printer_input
            < CharT, FPack, Preview, FloatT, strf::float_notation::hex, false >&
            input )
        : data_( strf::detail::init_hex_double_printer_data
                   ( input.vwf.get_float_format_data(), input.vwf.value() ) )
        , lettercase_(strf::get_facet<strf::lettercase_c, FloatT>(input.fp))
    {
        if (data_.exponent != 1024) {
            init_( strf::get_facet<strf::numpunct_c<10>, FloatT>(input.fp)
                 , strf::get_facet<strf::charset_c<CharT>, FloatT>(input.fp) );

            STRF_IF_CONSTEXPR ( ! Preview::nothing_required) {
                unsigned s = data_.showsign + 5 + data_.mantissa_digcount
                    + data_.extra_zeros + data_.exponent_digcount;
                STRF_IF_CONSTEXPR (Preview::width_required) {
                    input.preview.subtract_width(static_cast<std::int16_t>(s));
                    input.preview.subtract_width(data_.showpoint);
                }
                STRF_IF_CONSTEXPR (Preview::size_required) {
                    input.preview.add_size(s);
                    input.preview.add_size(pointsize_);
                }
            }
        } else {
            input.preview.subtract_width(3 + data_.showsign);
            input.preview.add_size(3 + data_.showsign);
        }
    }

    template <typename CharT, typename FPack, typename Preview, typename FloatT>
    hex_double_printer
        ( const strf::detail::fmt_double_printer_input
            < CharT, FPack, Preview, FloatT, strf::float_notation::hex, true >&
            input )
        : data_( strf::detail::init_hex_double_printer_data
                   ( input.vwf.get_float_format_data(), input.vwf.value() ) )
        , inv_seq_poli_(strf::get_facet<strf::invalid_seq_policy_c, FloatT>(input.fp))
        , surr_poli_(strf::get_facet<strf::surrogate_policy_c, FloatT>(input.fp))
        , lettercase_(strf::get_facet<strf::lettercase_c, FloatT>(input.fp))
    {
        int content_width_without_point = 0;
        decltype(auto) charset = strf::get_facet<strf::charset_c<CharT>, FloatT>(input.fp);
        encode_fill_ = charset.encode_fill_func();
        if (data_.exponent != 1024) {
            init_(strf::get_facet<strf::numpunct_c<10>, FloatT>(input.fp), charset);
            content_width_without_point = data_.showsign + 5
                + data_.mantissa_digcount
                + data_.extra_zeros + data_.exponent_digcount;
        } else {
            content_width_without_point = 3 + data_.showsign;
        }
        int content_width = content_width_without_point + data_.showpoint;
        auto fillcount = init_fills_(content_width, input.vwf.get_alignment_format_data());
        input.preview.checked_subtract_width(content_width + fillcount);
        STRF_IF_CONSTEXPR (Preview::size_required) {
            input.preview.add_size(content_width_without_point);
            input.preview.add_size(pointsize_);
            input.preview.add_size(fillcount * charset.encoded_char_size(input.vwf.fill()));
        }
    }

    STRF_HD void print_to(strf::underlying_outbuf<CharSize>&) const override;

private:

    template <typename NumPunct, typename Charset>
    STRF_HD void init_(const NumPunct& punct, Charset charset) noexcept
    {
        if (data_.showpoint) {
            decimal_point_ = punct.decimal_point();
            pointsize_ = 1;
            if (decimal_point_ >= 0x80) {
                encode_char_ = charset.encode_char_func();
                pointsize_ = static_cast<unsigned>(charset.encoded_char_size(decimal_point_));
                if (pointsize_ == 1) {
                    char_type ch;
                    charset.encode_char(&ch, decimal_point_);
                    decimal_point_ = ch;
                }
            }
        }
    }

    STRF_HD std::uint16_t init_fills_(int content_width, strf::alignment_format_data afmt)
    {
        if (content_width < afmt.width) {
            fillchar_ = afmt.fill;
            std::uint16_t fillcount = afmt.width - (std::uint16_t)content_width;
            switch(afmt.alignment) {
                case strf::text_alignment::left:
                    right_fillcount_ = fillcount;
                    break;
                case strf::text_alignment::right:
                    left_fillcount_ = fillcount;
                    break;
                case strf::text_alignment::split:
                    split_fillcount_ = fillcount;
                    break;
                default:
                    STRF_ASSERT(afmt.alignment == strf::text_alignment::center);
                    left_fillcount_ = fillcount >> 1;
                    right_fillcount_ = fillcount - left_fillcount_;
            }
            return fillcount;
        }
        return 0;
    }

    strf::encode_char_f<CharSize> encode_char_ = nullptr;
    strf::encode_fill_f<CharSize> encode_fill_ = nullptr;
    strf::detail::hex_double_printer_data data_;
    std::uint16_t left_fillcount_ = 0;
    std::uint16_t split_fillcount_ = 0;
    std::uint16_t right_fillcount_ = 0;
    strf::invalid_seq_policy inv_seq_poli_ = strf::invalid_seq_policy::replace;
    strf::surrogate_policy surr_poli_ = strf::surrogate_policy::strict;
    strf::lettercase lettercase_;
    char32_t fillchar_ = ' ';
    char32_t decimal_point_ = '.';
    unsigned pointsize_ = 0;
};


template <std::size_t CharSize>
STRF_HD void hex_double_printer<CharSize>::print_to
    ( strf::underlying_outbuf<CharSize>& ob ) const
{
    if (left_fillcount_ != 0) {
        encode_fill_(ob, left_fillcount_, fillchar_, inv_seq_poli_, surr_poli_);
    }
    if (data_.showsign) {
        put(ob, static_cast<char_type>('+' + (data_.negative << 1)));
    }
    if (split_fillcount_ != 0) {
        encode_fill_(ob, split_fillcount_, fillchar_, inv_seq_poli_, surr_poli_);
    }
    auto data = data_;
    if (data.exponent != 1024) {
        ob.ensure(3 + pointsize_ + data.mantissa_digcount);
        auto it = ob.pointer();
        it[0] = '0';
        it[1] = 'X' | ((lettercase_ != strf::uppercase) << 5);
        it[2] = 0x30 | int(data.exponent != -1023);
        it += 3;
        if (pointsize_ == 1) {
            *it ++ = static_cast<char_type>(decimal_point_);
        } else if (pointsize_ != 0) {
            it = encode_char_(it, decimal_point_);
        }
        if (data.mantissa != 0) {
            std::uint8_t digits[13] =
                { static_cast<std::uint8_t>((data.mantissa & (0xFull << 48)) >> 48)
                , static_cast<std::uint8_t>((data.mantissa & (0xFull << 44)) >> 44)
                , static_cast<std::uint8_t>((data.mantissa & (0xFull << 40)) >> 40)
                , static_cast<std::uint8_t>((data.mantissa & (0xFull << 36)) >> 36)
                , static_cast<std::uint8_t>((data.mantissa & (0xFull << 32)) >> 32)
                , static_cast<std::uint8_t>((data.mantissa & (0xFull << 28)) >> 28)
                , static_cast<std::uint8_t>((data.mantissa & (0xFull << 24)) >> 24)
                , static_cast<std::uint8_t>((data.mantissa & (0xFull << 20)) >> 20)
                , static_cast<std::uint8_t>((data.mantissa & (0xFull << 16)) >> 16)
                , static_cast<std::uint8_t>((data.mantissa & (0xFull << 12)) >> 12)
                , static_cast<std::uint8_t>((data.mantissa & (0xFull <<  8)) >>  8)
                , static_cast<std::uint8_t>((data.mantissa & (0xFull <<  4)) >>  4)
                , static_cast<std::uint8_t>(data.mantissa & 0xFull) };

            const char offset_digit_a = ('A' | ((lettercase_ == strf::lowercase) << 5)) - 10;
            for(unsigned i = 0; i < data.mantissa_digcount; ++i) {
                auto digit = digits[i];
                *it ++ = ( digit < 10
                         ? ('0' + digit)
                         : (offset_digit_a + digit) );
            }
        }
        ob.advance_to(it);
        if (data.extra_zeros) {
            detail::write_fill(ob, data.extra_zeros,  (char_type)'0');
        }

        if (data.exponent == -1023) {
            if (data.mantissa == 0) {
                ob.ensure(3);
                it = ob.pointer();
                it[0] = 'P' | ((lettercase_ != strf::uppercase) << 5);
                it[1] = '+';
                it[2] = '0';
                ob.advance(3);
            } else {
                ob.ensure(6);
                it = ob.pointer();
                it[0] = 'P' | ((lettercase_ != strf::uppercase) << 5);
                it[1] = '-';
                it[2] = '1';
                it[3] = '0';
                it[4] = '2';
                it[5] = '2';
                ob.advance(6);
            }
        } else {
            ob.ensure(2 + data.exponent_digcount);
            it = ob.pointer();
            it[0] = 'P' | ((lettercase_ != strf::uppercase) << 5);
            it[1] = static_cast<char_type>('+') + ((data.exponent < 0) << 1);
            it += 2 + data.exponent_digcount;
            strf::detail::write_int_dec_txtdigits_backwards(data.exponent, it);
            ob.advance_to(it);
        }
    } else {
        if (data.mantissa == 0) {
            strf::detail::print_inf(ob, lettercase_);
        } else {
            strf::detail::print_nan(ob, lettercase_);
        }
    }
    if (right_fillcount_ != 0) {
        encode_fill_(ob, right_fillcount_, fillchar_, inv_seq_poli_, surr_poli_);
    }
}

#if defined(STRF_SEPARATE_COMPILATION)

STRF_EXPLICIT_TEMPLATE class punct_double_printer<1>;
STRF_EXPLICIT_TEMPLATE class punct_double_printer<2>;
STRF_EXPLICIT_TEMPLATE class punct_double_printer<4>;

STRF_EXPLICIT_TEMPLATE class double_printer<1>;
STRF_EXPLICIT_TEMPLATE class double_printer<2>;
STRF_EXPLICIT_TEMPLATE class double_printer<4>;

STRF_EXPLICIT_TEMPLATE class fast_double_printer<1>;
STRF_EXPLICIT_TEMPLATE class fast_double_printer<2>;
STRF_EXPLICIT_TEMPLATE class fast_double_printer<4>;

STRF_EXPLICIT_TEMPLATE class fast_punct_double_printer<1>;
STRF_EXPLICIT_TEMPLATE class fast_punct_double_printer<2>;
STRF_EXPLICIT_TEMPLATE class fast_punct_double_printer<4>;

STRF_EXPLICIT_TEMPLATE class hex_double_printer<1>;
STRF_EXPLICIT_TEMPLATE class hex_double_printer<2>;
STRF_EXPLICIT_TEMPLATE class hex_double_printer<4>;

#endif // defined(STRF_SEPARATE_COMPILATION)

} // namespace detail

} // namespace strf

#endif  // STRF_DETAIL_INPUT_TYPES_FLOAT_HPP

