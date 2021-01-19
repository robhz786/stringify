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

namespace strf {
namespace detail {

inline STRF_HD std::uint32_t to_bits(float f)
{
    return strf::detail::bit_cast<std::uint32_t>(f);
}

inline STRF_HD std::uint64_t to_bits(const double d)
{
    return strf::detail::bit_cast<std::uint64_t>(d);
}

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

STRF_FUNC_IMPL STRF_HD double_dec_base trivial_float_dec(
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

STRF_FUNC_IMPL STRF_HD double_dec_base trivial_double_dec(
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

STRF_FUNC_IMPL STRF_HD detail::double_dec decode(float f)
{
    constexpr int bias = 127;
    constexpr int e_size = 8;
    constexpr int m_size = 23;

    std::uint32_t bits = strf::detail::to_bits(f);
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


STRF_FUNC_IMPL STRF_HD detail::double_dec decode(double d)
{
    constexpr int bias = 1023;
    constexpr int e_size = 11; // bits in exponent
    constexpr int m_size = 52; // bits in matissa

    std::uint64_t bits = strf::detail::to_bits(d);
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

using chars_count_t = unsigned;

} // namespace detail

enum class float_notation{fixed, scientific, general, hex};

template <strf::float_notation Notation>
struct float_format
{
    detail::chars_count_t precision = (detail::chars_count_t)-1;
    detail::chars_count_t pad0width = 0;
    strf::showsign sign = strf::showsign::negative_only;
    bool showpoint = false;
    constexpr static strf::float_notation notation = Notation;
};

template <strf::float_notation To, strf::float_notation From>
constexpr STRF_HD strf::float_format<To> change_notation(strf::float_format<From> x) noexcept
{
    return float_format<To>{ x.precision, x.pad0width, x.sign, x.showpoint };
}

template <strf::float_notation Notation>
constexpr STRF_HD bool operator==( strf::float_format<Notation> lhs
                                 , strf::float_format<Notation> rhs ) noexcept
{
    return lhs.precision == rhs.precision
        && lhs.pad0width == lhs.pad0width
        && lhs.sign == rhs.sign
        && lhs.showpoint == rhs.showpoint ;
}

template <strf::float_notation Notation>
constexpr STRF_HD bool operator!=( strf::float_format<Notation> lhs
                                 , strf::float_format<Notation> rhs ) noexcept
{
    return ! (lhs == rhs);
}

template <typename T, strf::float_notation Notation>
class float_formatter_fn;

template <strf::float_notation Notation>
struct float_formatter
{
    template <typename T>
    using fn = float_formatter_fn<T, Notation>;
};

template <typename T, strf::float_notation Notation>
class float_formatter_fn
{
    template <strf::float_notation OtherNotation>
    using adapted_derived_type_
        = strf::fmt_replace<T, float_formatter<Notation>, float_formatter<OtherNotation> >;

public:

    constexpr float_formatter_fn() noexcept = default;

    constexpr STRF_HD explicit float_formatter_fn
        ( const strf::float_format<Notation>& data ) noexcept
        : data_(data)
    {
    }

    template <typename U>
    constexpr STRF_HD explicit float_formatter_fn
        ( const float_formatter_fn<U, Notation>& other ) noexcept
    {
        auto data = other.get_float_format();
        data_.precision = data.precision;
        data_.pad0width = data.pad0width;
        data_.sign = data.sign;
        data_.showpoint = data.showpoint;
    }
    constexpr STRF_HD T&& operator+() && noexcept
    {
        data_.sign = strf::showsign::positive_also;
        T* base_ptr = static_cast<T*>(this); // work around UBSan bug
        return static_cast<T&&>(*base_ptr);
    }
    constexpr STRF_HD T&& fill_sign() && noexcept
    {
        data_.sign = strf::showsign::fill_instead_of_positive;
        T* base_ptr = static_cast<T*>(this);
        return static_cast<T&&>(*base_ptr);
    }
    constexpr STRF_HD T&& operator~() && noexcept
    {
        data_.sign = strf::showsign::fill_instead_of_positive;
        T* base_ptr = static_cast<T*>(this);
        return static_cast<T&&>(*base_ptr);
    }
    constexpr STRF_HD T&& operator*() && noexcept
    {
        data_.showpoint = true;
        T* base_ptr = static_cast<T*>(this);
        return static_cast<T&&>(*base_ptr);
    }
    constexpr STRF_HD T&& p(detail::chars_count_t _) && noexcept
    {
        data_.precision = _;
        T* base_ptr = static_cast<T*>(this);
        return static_cast<T&&>(*base_ptr);
    }
    constexpr STRF_HD T&& pad0(detail::chars_count_t width) && noexcept
    {
        data_.pad0width = width;
        T* base_ptr = static_cast<T*>(this);
        return static_cast<T&&>(*base_ptr);
    }

    template <strf::float_notation N = strf::float_notation::scientific>
    constexpr STRF_HD
    std::enable_if_t<N == Notation && N == strf::float_notation::scientific, T&&>
    sci() && noexcept
    {
        T* base_ptr = static_cast<T*>(this);
        return static_cast<T&&>(*base_ptr);
    }

    template <strf::float_notation N = strf::float_notation::scientific>
    constexpr STRF_HD
    std::enable_if_t< N != Notation && N == strf::float_notation::scientific
                    , adapted_derived_type_<N> >
    sci() const & noexcept
    {
        const T* base_ptr = static_cast<const T*>(this);
        return adapted_derived_type_<N>
            { *base_ptr
            , strf::tag<strf::float_formatter<N>>{}
            , change_notation<N>(data_) };
    }

    template <strf::float_notation N = strf::float_notation::fixed>
    constexpr STRF_HD
    std::enable_if_t<N == Notation && N == strf::float_notation::fixed, T&&>
    fixed() && noexcept
    {
        T* base_ptr = static_cast<T*>(this);
        return static_cast<T&&>(*base_ptr);
    }

    template <strf::float_notation N = strf::float_notation::fixed>
    constexpr STRF_HD
    std::enable_if_t< N != Notation && N == strf::float_notation::fixed
                    , adapted_derived_type_<N> >
    fixed() const & noexcept
    {
        const T* base_ptr = static_cast<const T*>(this);
        return adapted_derived_type_<N>
            { *base_ptr
            , strf::tag<strf::float_formatter<N>>{}
            , change_notation<N>(data_) };
    }

    template <strf::float_notation N = strf::float_notation::general>
    constexpr STRF_HD
    std::enable_if_t<N == Notation && N == strf::float_notation::general, T&&>
    gen() && noexcept
    {
        T* base_ptr = static_cast<T*>(this);
        return static_cast<T&&>(*base_ptr);
    }

    template <strf::float_notation N = strf::float_notation::general>
    constexpr STRF_HD
    std::enable_if_t< N != Notation && N == strf::float_notation::general
                    , adapted_derived_type_<N> >
    gen() const & noexcept
    {
        const T* base_ptr = static_cast<const T*>(this);
        return adapted_derived_type_<N>
            { *base_ptr
            , strf::tag<strf::float_formatter<N>>{}
            , change_notation<N>(data_) };
    }

    template <strf::float_notation N = strf::float_notation::hex>
    constexpr STRF_HD
    std::enable_if_t<N == Notation && N == strf::float_notation::hex, T&&>
    hex() && noexcept
    {
        T* base_ptr = static_cast<T*>(this);
        return static_cast<T&&>(*base_ptr);
    }
    template <strf::float_notation N = strf::float_notation::hex>
    constexpr STRF_HD
    std::enable_if_t< N != Notation && N == strf::float_notation::hex
                    , adapted_derived_type_<N> >
    hex() const & noexcept
    {
        const T* base_ptr = static_cast<const T*>(this);
        return adapted_derived_type_<N>
            { *base_ptr
            , strf::tag<strf::float_formatter<N>>{}
            , change_notation<N>(data_) };
    }

    static constexpr STRF_HD strf::float_notation float_notation() noexcept
    {
        return Notation;
    }

    constexpr STRF_HD strf::float_format<Notation> get_float_format() const noexcept
    {
        return data_;
    }
    constexpr STRF_HD auto pad0width() const
    {
        return data_.pad0width;
    }
    template <strf::float_notation N>
    constexpr STRF_HD std::enable_if_t<N == Notation, T&&>
    set_float_format(strf::float_format<N> data) && noexcept
    {
        data_ = data;
        T* base_ptr = static_cast<T*>(this);
        return static_cast<T&&>(*base_ptr);
    }

    template <strf::float_notation N>
    constexpr STRF_HD std::enable_if_t<N != Notation, adapted_derived_type_<N>>
    set_float_format(strf::float_format<N> data) && noexcept
    {
        const T* base_ptr = static_cast<const T*>(this);
        return adapted_derived_type_<N>
            { *base_ptr
            , strf::tag<strf::float_formatter<N>>{}
            , data };
    }

private:

    strf::float_format<Notation> data_;
};

namespace detail {

template <typename> class fast_double_printer;
template <typename> class fast_punct_double_printer;
template <typename> class punct_double_printer;
template <typename> class hex_double_printer;

template <typename> struct float_printing;

template
    < typename FloatT, strf::float_notation Notation, bool Align >
using float_with_formatters = strf::value_with_formatters
    < strf::print_traits<FloatT>
    , strf::float_formatter<Notation>
    , strf::alignment_formatter_q<Align> >;

template < typename CharT, typename Preview, typename FloatT>
struct fast_double_printer_input
{
    using printer_type = strf::detail::fast_double_printer<CharT>;

    template <typename FPack>
    STRF_HD fast_double_printer_input(Preview& preview_, const FPack& fp_, FloatT arg_)
        : preview(preview_)
        , value(arg_)
        , lcase(strf::get_facet<strf::lettercase_c, float>(fp_))
    {
    }

    fast_double_printer_input(const fast_double_printer_input&) = default;
    fast_double_printer_input(fast_double_printer_input&&) = default;

    Preview& preview;
    FloatT value;
    strf::lettercase lcase;
};


template <typename CharT, typename Preview, typename FPack, typename FloatT>
using fast_punct_double_printer_input =
    strf::usual_printer_input< CharT, Preview, FPack, FloatT
                             , strf::detail::fast_punct_double_printer<CharT> >;

template < typename CharT, typename Preview, typename FPack
         , typename FloatT, strf::float_notation Notation, bool HasAlignment >
using fmt_double_printer_input =
    strf::usual_printer_input
        < CharT, Preview, FPack
        , strf::detail::float_with_formatters<FloatT, Notation, HasAlignment>
        , std::conditional_t
            < Notation == float_notation::hex
            , strf::detail::hex_double_printer<CharT>
            , strf::detail::punct_double_printer<CharT> > >;
            // , std::conditional_t
            //     < strf::detail::has_punct<CharT, FPack, FloatT, 10>
            //     , strf::detail::punct_double_printer<CharT>
            //     , strf::detail::double_printer<CharT> > > >;

template <typename FloatT>
struct float_printing
{
    using override_tag = FloatT;
    using forwarded_type = FloatT;
    using formatters = strf::tag
        < strf::float_formatter<strf::float_notation::general>
        , strf::empty_alignment_formatter >;

    template <typename CharT, typename Preview, typename FPack>
    STRF_HD constexpr static auto make_printer_input
        ( Preview& preview, const FPack& fp, FloatT x ) noexcept
        -> std::conditional_t
            < strf::detail::has_punct<CharT, FPack, FloatT, 10>
            , strf::detail::fast_punct_double_printer_input<CharT, Preview, FPack, FloatT>
            , strf::detail::fast_double_printer_input<CharT, Preview, FloatT> >
    {
        return {preview, fp, x};
    }

    template < typename CharT, typename Preview, typename FPack
             , strf::float_notation Notation, bool HasAlignment >
    STRF_HD constexpr static auto make_printer_input
        ( Preview& preview
        , const FPack& fp
        , strf::detail::float_with_formatters<FloatT, Notation, HasAlignment> x) noexcept
        -> strf::detail::fmt_double_printer_input
            < CharT, Preview, FPack, FloatT, Notation, HasAlignment >
    {
        return {preview, fp, x};
    }
};

} // namespace detail

template <> struct print_traits<float>:  public strf::detail::float_printing<float> {};
template <> struct print_traits<double>: public strf::detail::float_printing<double> {};

STRF_HD constexpr auto tag_invoke(strf::print_traits_tag, float)
    -> strf::print_traits<float>
    { return {}; }

STRF_HD constexpr auto tag_invoke(strf::print_traits_tag, double)
    -> strf::print_traits<double>
    { return {}; }

void tag_invoke(strf::print_traits_tag, long double) = delete;

namespace detail {

enum class float_form : std::uint8_t { nan, inf, fixed, sci };

constexpr bool inf_or_nan(float_form f)
{
    return static_cast<std::uint8_t>(f) < 2;
}

struct double_printer_data
{
    strf::detail::float_form form;
    char sign;
    bool showsign;
    bool showpoint;

    detail::chars_count_t chars_count;
    detail::chars_count_t pad0width;
    detail::chars_count_t extra_zeros;

    detail::chars_count_t sep_count;
    detail::chars_count_t m10_digcount;
    std::uint64_t m10;
    std::int32_t e10;
};

template <strf::float_notation Notation>
STRF_HD double_printer_data init_double_printer_data
    ( detail::double_dec d
    , float_format<Notation> fdata
    , strf::digits_grouping grp ) noexcept;

template <strf::float_notation Notation>
inline STRF_HD double_printer_data init_double_printer_data
    ( float f
    , float_format<Notation> fdata
    , strf::digits_grouping grp = strf::digits_grouping{} ) noexcept
{
    return init_double_printer_data<Notation>(detail::decode(f), fdata, grp);
}

template <strf::float_notation Notation>
inline STRF_HD double_printer_data init_double_printer_data
    ( double d
    , float_format<Notation> fdata
    , strf::digits_grouping grp = strf::digits_grouping{} ) noexcept
{
    return init_double_printer_data<Notation>(detail::decode(d), fdata, grp);
}

template <strf::float_notation Notation>
STRF_HD double_printer_data init_double_printer_data
    ( detail::double_dec dd
    , float_format<Notation> fdata
    , strf::digits_grouping grouping ) noexcept
{
    static_assert(Notation != strf::float_notation::hex, "");
    double_printer_data data;
    data.m10      = dd.m10;
    data.e10      = dd.e10;
    data.sep_count = 0;
    data.extra_zeros = 0;
    data.pad0width = fdata.pad0width;
    data.sign = dd.negative ? '-' : static_cast<char>(fdata.sign);
    data.showsign = dd.negative || fdata.sign != strf::showsign::negative_only;
    data.chars_count = data.showsign;
    if (dd.nan || dd.infinity) {
        data.form = dd.nan ? detail::float_form::nan : detail::float_form::inf;
        data.showpoint = false;
        data.m10_digcount = 0;
        data.chars_count += 3;
    } else {
        data.m10_digcount = static_cast<detail::chars_count_t>(strf::detail::count_digits<10>(data.m10));
        if (fdata.precision == (decltype(fdata.precision))-1) {
            STRF_IF_CONSTEXPR (Notation == float_notation::general) {
                const int sci_notation_exp = (int)data.m10_digcount + data.e10 - 1;
                const int scientific_width
                    = data.m10_digcount
                    + 4 + (sci_notation_exp > 99 || sci_notation_exp < -99)
                    + (fdata.showpoint || data.m10_digcount != 1);
                const auto fixed_int_digcount = (int)data.m10_digcount + data.e10;
                int fixed_width = ( data.e10 >= 0
                                  ? fixed_int_digcount + fdata.showpoint
                                  : data.e10 <= -(int)data.m10_digcount
                                  ? 2 - data.e10
                                  : 1 + (int)data.m10_digcount );
                if (fixed_width <= scientific_width) {
                    if (grouping.any_separator(fixed_int_digcount)) {
                        auto sep_count = grouping.separators_count(fixed_int_digcount);
                        fixed_width += sep_count;
                        if (fixed_width > scientific_width) {
                            goto general_scientific;
                        }
                        data.sep_count = static_cast<decltype(data.sep_count)>(sep_count);
                    }
                    data.form = detail::float_form::fixed;
                    data.showpoint = fdata.showpoint || (data.e10 < 0);
                    data.chars_count += static_cast<detail::chars_count_t>(fixed_width);
                } else {
                    general_scientific:
                    data.form = detail::float_form::sci;
                    data.chars_count += static_cast<detail::chars_count_t>(scientific_width);
                    data.showpoint = fdata.showpoint || (data.m10_digcount != 1);
                }
            }
            STRF_IF_CONSTEXPR (Notation == float_notation::fixed) {
                data.form = detail::float_form::fixed;
                data.showpoint = fdata.showpoint || (data.e10 < 0);
                auto int_digcount = (int)data.m10_digcount + data.e10;
                if (grouping.any_separator(int_digcount)) {
                    data.sep_count = static_cast<detail::chars_count_t>(grouping.separators_count(int_digcount));
                    data.chars_count += data.sep_count;
                }
                data.chars_count += ( data.e10 >= 0
                                    ? static_cast<detail::chars_count_t>(int_digcount + fdata.showpoint)
                                    : data.e10 <= -(int)data.m10_digcount
                                    ? static_cast<detail::chars_count_t>(2 - data.e10)
                                    : 1 + data.m10_digcount );
            }
            STRF_IF_CONSTEXPR (Notation == float_notation::scientific) {
                const int sci_notation_exp = (int)data.m10_digcount + data.e10 - 1;
                data.form = detail::float_form::sci;
                data.showpoint = fdata.showpoint || (data.m10_digcount != 1);
                data.chars_count += 4 + data.showpoint;
                data.chars_count += (sci_notation_exp > 99 || sci_notation_exp < -99);
                data.chars_count += data.m10_digcount;
            }
        } else {
            int xz; // number of zeros to be added or ( if negative ) digits to be removed
            STRF_IF_CONSTEXPR (Notation == float_notation::general) {
                // As in printf:
                // - Select the scientific notation if the resulting exponent
                //   is less than -4 or greater than or equal to the precision
                // - The precision specifies the number of significant digits.
                // - If the precision is 0, it is treated as 1
                // - Trailing fractional zeros are removed when fdata.showpoint is false.

                int p = fdata.precision != 0 ? fdata.precision : 1;
                const int int_digcount_fixed = (int)data.m10_digcount + data.e10;
                // same as:
                // const int sci_notation_exp = (int)data.m10_digcount + data.e10 - 1;
                // if (sci_notation_exp < -4 || sci_notation_exp >= p);
                if (int_digcount_fixed < -3 || int_digcount_fixed > p) {
                    data.form = detail::float_form::sci;
                    const int sci_notation_exp = (int)data.m10_digcount + data.e10 - 1;
                    data.showpoint = fdata.showpoint || (p > 1 && data.m10_digcount > 1);
                    data.chars_count += 4 + (sci_notation_exp > 99 || sci_notation_exp < -99);

                    xz = ( (p < (int)data.m10_digcount || fdata.showpoint)
                         ? p - (int)data.m10_digcount
                         : 0 );
                    data.chars_count += (int)data.m10_digcount;
                } else {
                    data.form = detail::float_form::fixed;
                    STRF_ASSERT (p >= int_digcount_fixed);
                    data.showpoint = fdata.showpoint || (p > int_digcount_fixed && data.e10 < 0);
                    if (grouping.any_separator(int_digcount_fixed)) {
                        data.sep_count = static_cast<detail::chars_count_t>(grouping.separators_count(int_digcount_fixed));
                        data.chars_count += data.sep_count;
                    }
                    if (data.e10 >= 0) {
                        data.chars_count += static_cast<detail::chars_count_t>(int_digcount_fixed);
                        //data.showpoint = fdata.showpoint;
                        // STRF_ASSERT(p >= int_digcount_fixed);
                        // STRF_ASSERT(p >= (int)data.m10_digcount);
                        xz = fdata.showpoint ? p - int_digcount_fixed : 0;
                    } else {
                        const int digcount = (int)data.m10_digcount;
                        if (p < digcount || fdata.showpoint) {
                            xz = p - digcount;
                            //data.showpoint = fdata.showpoint || (p > int_digcount_fixed);
                        } else {
                            xz = 0;
                            //data.showpoint = true;
                        }
                        if (data.e10 <= -digcount) {
                            data.chars_count += static_cast<detail::chars_count_t>(1 - data.e10);
                        } else {
                            data.chars_count += static_cast<detail::chars_count_t>(digcount);
                        }
                    }
                }
                data.chars_count += static_cast<std::make_signed_t<detail::chars_count_t>>(xz);
            }
            STRF_IF_CONSTEXPR (Notation == float_notation::scientific) {
                const int sci_notation_exp = (int)data.m10_digcount + data.e10 - 1;
                const unsigned frac_digits = data.m10_digcount - 1;
                xz = (fdata.precision - frac_digits);
                data.form = detail::float_form::sci;
                data.showpoint = fdata.showpoint || (fdata.precision != 0);
                data.chars_count += 5 + fdata.precision;
                data.chars_count += (sci_notation_exp > 99 || sci_notation_exp < -99);
            }
            STRF_IF_CONSTEXPR (Notation == float_notation::fixed) {
                data.showpoint = fdata.showpoint || (fdata.precision != 0);
                const int frac_digits = (data.e10 < 0) * -data.e10;
                xz = (fdata.precision - frac_digits);
                data.form = detail::float_form::fixed;
                auto int_digcount = ( (int)data.m10_digcount > -data.e10
                                    ? (int)data.m10_digcount + data.e10
                                    : 1 );
                if (grouping.any_separator(int_digcount)) {
                    data.sep_count = static_cast<detail::chars_count_t>(grouping.separators_count(int_digcount));
                    data.chars_count += static_cast<detail::chars_count_t>(data.sep_count);
                }
                data.chars_count += static_cast<detail::chars_count_t>(int_digcount + fdata.precision);
            }
            if (xz < 0) {
                data.extra_zeros = 0;
                unsigned dp = -xz;
                data.m10_digcount -= static_cast<detail::chars_count_t>(dp);
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
                        -- data.chars_count;
                        ++ data.e10;
                    }
                    const bool is_sci = data.form == detail::float_form::sci;
                    int frac_digits = is_sci * (data.m10_digcount - 1)
                        - ! is_sci * (data.e10 < 0) * data.e10;
                    data.showpoint = fdata.showpoint || (frac_digits != 0);
                }
            } else {
                data.extra_zeros = static_cast<detail::chars_count_t>(xz);
            }
            data.chars_count += data.showpoint;
        }
    }
    return data;
}

template <int Base, typename CharT, typename IntT>
inline STRF_HD void write_int_with_leading_zeros
    ( strf::basic_outbuff<CharT>& ob
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
        strf::detail::str_fill_n<CharT>(p, p2 - p, '0');
    }
    ob.advance_to(end);
}

template <typename CharT>
STRF_HD void print_amplified_integer_small_separator_1
    ( strf::basic_outbuff<CharT>& ob
    , unsigned long long value
    , unsigned num_digits
    , strf::digits_distribution dist
    , CharT separator )
{
    STRF_ASSERT(num_digits <= dist.highest_group);

    ob.ensure(num_digits);
    auto ptr = ob.pointer() + num_digits;
    strf::detail::write_int_dec_txtdigits_backwards(value, ptr);
    ob.advance_to(ptr);
    dist.highest_group -= num_digits;
    if (dist.highest_group != 0) {
        strf::detail::write_fill(ob, dist.highest_group, (CharT)'0');
    }
    if (dist.middle_groups_count) {
        auto middle_groups = dist.low_groups.highest_group();
        dist.low_groups.pop_high();
        do {
            ob.ensure(middle_groups + 1);
            auto oit = ob.pointer();
            *oit = separator;
            strf::detail::str_fill_n<CharT>(++oit, middle_groups, '0');
            ob.advance_to(oit + middle_groups);
        } while (--dist.middle_groups_count);
    }
    while ( ! dist.low_groups.empty()) {
        auto grp = dist.low_groups.highest_group();
        ob.ensure(grp + 1);
        auto oit = ob.pointer();
        *oit = separator;
        strf::detail::str_fill_n<CharT>(++oit, grp, '0');
        ob.advance_to(oit + grp);
        dist.low_groups.pop_high();
    }
}

template <typename CharT>
STRF_HD void print_amplified_integer_small_separator_2
    ( strf::basic_outbuff<CharT>& ob
    , unsigned long long value
    , unsigned num_digits
    , strf::digits_distribution dist
    , CharT separator )
{
    STRF_ASSERT(dist.highest_group < num_digits);

    constexpr std::size_t size_after_recycle = strf::min_space_after_recycle<CharT>();
    (void) size_after_recycle;

    constexpr auto max_digits = detail::max_num_digits<unsigned long long, 10>();
    char digits_buff[max_digits];
    auto digits = strf::detail::write_int_dec_txtdigits_backwards
        (value, digits_buff + max_digits);

    unsigned grp_size;

    ob.ensure(dist.highest_group);
    strf::detail::copy_n(digits, dist.highest_group, ob.pointer());
    num_digits -= dist.highest_group;
    digits += dist.highest_group;
    ob.advance(dist.highest_group);

    if (dist.middle_groups_count) {
        auto middle_groups = dist.low_groups.highest_group();
        dist.low_groups.pop_high();
        while (num_digits >= middle_groups) {
            ob.ensure(1 + middle_groups);
            auto oit = ob.pointer();
            *oit = separator;
            strf::detail::copy_n(digits, middle_groups, oit + 1);
            ob.advance(1 + middle_groups);
            num_digits -= middle_groups;
            digits += middle_groups;
            if (--dist.middle_groups_count == 0) {
                goto lower_groups;
            }
        }
        STRF_ASSERT(dist.middle_groups_count != 0);
        STRF_ASSERT(num_digits < middle_groups);
        if (num_digits != 0) {
            ob.ensure(1 + num_digits);
            auto oit = ob.pointer();
            *oit = separator;
            strf::detail::copy_n(digits, num_digits, oit + 1);
            ob.advance(1 + num_digits);
            auto remaining = middle_groups - num_digits;
            num_digits = 0;
            strf::detail::write_fill(ob, remaining, (CharT)'0');
            -- dist.middle_groups_count;
        }
        STRF_ASSERT(num_digits == 0);
        while (dist.middle_groups_count) {
            strf::put(ob, separator);
            strf::detail::write_fill(ob, middle_groups, (CharT)'0');
            -- dist.middle_groups_count;
        }
        STRF_ASSERT(dist.middle_groups_count == 0);
        goto lower_groups_in_trailing_zeros;
    }
    lower_groups:
    if (num_digits != 0) {
        STRF_ASSERT(dist.middle_groups_count == 0);
        grp_size = dist.low_groups.highest_group();
        dist.low_groups.pop_high();
        while (num_digits > grp_size) {
            STRF_ASSERT(! dist.low_groups.empty());
            STRF_ASSERT(grp_size + 1 <= size_after_recycle);
            ob.ensure(grp_size + 1);
            auto oit = ob.pointer();
            *oit = separator;
            strf::detail::copy_n(digits, grp_size, oit + 1);
            digits += grp_size;
            ob.advance(grp_size + 1);
            num_digits -= grp_size;
            grp_size = dist.low_groups.highest_group();
            dist.low_groups.pop_high();
        }
        STRF_ASSERT(num_digits != 0);
        STRF_ASSERT(num_digits + 1 <= size_after_recycle);
        ob.ensure(num_digits + 1);
        auto oit = ob.pointer();
        *oit = separator;
        strf::detail::copy_n(digits, num_digits, oit + 1);
        ob.advance(num_digits + 1);
        if (grp_size > num_digits) {
            grp_size -= num_digits;
            STRF_ASSERT(grp_size <= size_after_recycle);
            ob.ensure(grp_size + (num_digits == 0));
            oit = ob.pointer();
            strf::detail::str_fill_n<CharT>(oit, grp_size, '0');
            ob.advance_to(oit + grp_size);
        }
    }
    lower_groups_in_trailing_zeros:
    while (! dist.low_groups.empty()) {
        grp_size = dist.low_groups.highest_group();
        dist.low_groups.pop_high();
        STRF_ASSERT(grp_size + 1 <= size_after_recycle);
        ob.ensure(grp_size + 1);
        auto it = ob.pointer();
        *it = separator;
        strf::detail::str_fill_n<CharT>(it + 1, grp_size, '0');
        ob.advance(grp_size + 1);
    }
}


template <typename CharT>
inline STRF_HD void print_amplified_integer_small_separator
    ( strf::basic_outbuff<CharT>& ob
    , unsigned long long value
    , strf::digits_grouping grouping
    , unsigned num_digits
    , unsigned num_trailing_zeros
    , CharT separator )
{
    auto dist = grouping.distribute(num_digits + num_trailing_zeros);
    if (num_digits <= dist.highest_group) {
        print_amplified_integer_small_separator_1
            ( ob, value, num_digits, dist, separator );
    } else {
        print_amplified_integer_small_separator_2
            ( ob, value, num_digits, dist, separator );
    }
}

template <typename CharT>
STRF_HD void print_amplified_integer_big_separator_1
    ( strf::basic_outbuff<CharT>& ob
    , strf::encode_char_f<CharT> encode_char
    , unsigned long long value
    , unsigned num_digits
    , strf::digits_distribution dist
    , char32_t separator
    , unsigned separator_size )
{
    STRF_ASSERT(num_digits <= dist.highest_group);

    ob.ensure(num_digits);
    auto ptr = ob.pointer() + num_digits;
    strf::detail::write_int_dec_txtdigits_backwards(value, ptr);
    ob.advance_to(ptr);
    dist.highest_group -= num_digits;
    if (dist.highest_group != 0) {
        strf::detail::write_fill(ob, dist.highest_group, (CharT)'0');
    }
    if (dist.middle_groups_count) {
        auto middle_groups = dist.low_groups.highest_group();
        dist.low_groups.pop_high();
        do {
            ob.ensure(separator_size + middle_groups);
            auto oit = encode_char(ob.pointer(), separator);
            strf::detail::str_fill_n<CharT>(oit, middle_groups, '0');
            ob.advance_to(oit + middle_groups);
        } while (--dist.middle_groups_count);
    }
    while ( ! dist.low_groups.empty()) {
        auto grp = dist.low_groups.highest_group();
        dist.low_groups.pop_high();
        ob.ensure(separator_size + grp);
        auto oit = encode_char(ob.pointer(), separator);
        strf::detail::str_fill_n<CharT>(oit, grp, '0');
        ob.advance(separator_size + grp);
    }
}

template <typename CharT>
STRF_HD void print_amplified_integer_big_separator_2
    ( strf::basic_outbuff<CharT>& ob
    , strf::encode_char_f<CharT> encode_char
    , unsigned long long value
    , unsigned num_digits
    , strf::digits_distribution dist
    , char32_t separator
    , unsigned separator_size )
{
    STRF_ASSERT(dist.highest_group < num_digits);

    constexpr std::size_t size_after_recycle = strf::min_space_after_recycle<CharT>();
    (void) size_after_recycle;

    constexpr auto max_digits = detail::max_num_digits<unsigned long long, 10>();
    char digits_buff[max_digits];
    auto digits = strf::detail::write_int_dec_txtdigits_backwards
        (value, digits_buff + max_digits);

    unsigned grp_size;

    ob.ensure(dist.highest_group);
    strf::detail::copy_n(digits, dist.highest_group, ob.pointer());
    num_digits -= dist.highest_group;
    digits += dist.highest_group;
    ob.advance(dist.highest_group);

    if (dist.middle_groups_count) {
        auto middle_groups = dist.low_groups.highest_group();
        dist.low_groups.pop_high();
        while (num_digits >= middle_groups) {
            ob.ensure(separator_size + middle_groups);
            auto *oit = encode_char(ob.pointer(), separator);
            strf::detail::copy_n(digits, middle_groups, oit);
            ob.advance_to(oit + middle_groups);
            num_digits -= middle_groups;
            digits += middle_groups;
            if (--dist.middle_groups_count == 0) {
                goto lower_groups;
            }
        }
        STRF_ASSERT(dist.middle_groups_count != 0);
        STRF_ASSERT(num_digits < middle_groups);
        if (num_digits != 0) {
            ob.ensure(separator_size + middle_groups);
            const auto remaining = middle_groups - num_digits;
            auto oit = encode_char(ob.pointer(), separator);
            strf::detail::copy_n(digits, num_digits, oit);
            strf::detail::str_fill_n<CharT>(oit + num_digits, remaining, '0');
            ob.advance_to(oit + middle_groups);
            num_digits = 0;
            --dist.middle_groups_count;
        }
        STRF_ASSERT(num_digits == 0);
        while (dist.middle_groups_count) {
            ob.ensure(separator_size + middle_groups);
            auto oit = encode_char(ob.pointer(), separator);
            strf::detail::str_fill_n<CharT>(oit, middle_groups, '0');
            ob.advance_to(oit + middle_groups);
            -- dist.middle_groups_count;
        }
        STRF_ASSERT(dist.middle_groups_count == 0);
        goto lower_groups_in_trailing_zeros;
    }

    lower_groups:
    if (num_digits) {
        STRF_ASSERT(dist.middle_groups_count == 0);
        grp_size = dist.low_groups.highest_group();
        dist.low_groups.pop_high();
        while (num_digits > grp_size) {
            STRF_ASSERT(! dist.low_groups.empty());
            // `-> otherwise (num_digits > grp_size) should be false
            STRF_ASSERT(grp_size + separator_size <= size_after_recycle);
            ob.ensure(separator_size + grp_size);
            auto oit = encode_char(ob.pointer(), separator);
            strf::detail::copy_n(digits, grp_size, oit);
            ob.advance_to(oit + grp_size);
            digits += grp_size;
            num_digits -= grp_size;
            grp_size = dist.low_groups.highest_group();
            dist.low_groups.pop_high();
        }
        STRF_ASSERT(num_digits + separator_size <= size_after_recycle);
        ob.ensure(separator_size + num_digits);
        auto oit = encode_char(ob.pointer(), separator);
        strf::detail::copy_n(digits, num_digits, oit);
        ob.advance_to(oit + num_digits);
        if (grp_size > num_digits) {
            grp_size -= num_digits;
            STRF_ASSERT(grp_size <= size_after_recycle);
            ob.ensure(grp_size);
            oit = ob.pointer();
            strf::detail::str_fill_n<CharT>(oit, grp_size, '0');
            ob.advance_to(oit + grp_size);
        }
    }
    lower_groups_in_trailing_zeros:
    while (! dist.low_groups.empty()) {
        grp_size = dist.low_groups.highest_group();
        dist.low_groups.pop_high();
        STRF_ASSERT(separator_size + grp_size <= size_after_recycle);
        ob.ensure(separator_size + grp_size);
        auto oit = encode_char(ob.pointer(), separator);
        strf::detail::str_fill_n<CharT>(oit, grp_size, '0');
        ob.advance_to(oit + grp_size);
    }
}

template <typename CharT>
STRF_HD void print_amplified_integer_big_separator
    ( strf::basic_outbuff<CharT>& ob
    , strf::encode_char_f<CharT> encode_char
    , unsigned long long value
    , strf::digits_grouping grouping
    , unsigned num_digits
    , unsigned num_trailing_zeros
    , unsigned separator_size
    , char32_t separator )
{
    auto dist = grouping.distribute(num_digits + num_trailing_zeros);
    if (num_digits <= dist.highest_group) {
        print_amplified_integer_big_separator_1
            ( ob, encode_char, value, num_digits, dist, separator, separator_size );
    } else {
        print_amplified_integer_big_separator_2
            ( ob, encode_char, value, num_digits, dist, separator, separator_size );
    }
}

// to-do move this function to punct_double_printer class template
template <typename CharT>
STRF_HD void print_scientific_notation
    ( strf::basic_outbuff<CharT>& ob
    , strf::encode_char_f<CharT> encode_char
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
    print_point |= num_digits != 1;
    ob.ensure(num_digits + print_point * decimal_point_size);
    if (num_digits == 1) {
        auto it = ob.pointer();
        *it = static_cast<CharT>('0' + digits);
        ++it;
        if (print_point) {
            if (decimal_point_size == 1) {
                *it++ = static_cast<CharT>(decimal_point);
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
           *it++ = static_cast<CharT>(decimal_point);
       } else {
           encode_char(it, decimal_point);
       }
       ob.advance_to(end);
    }

    // extra trailing zeros

    if (trailing_zeros != 0) {
        strf::detail::write_fill(ob, trailing_zeros, CharT('0'));
    }

    // exponent

    unsigned adv = 4;
    CharT* it;
    unsigned e10u = std::abs(exponent);
    STRF_ASSERT(e10u < 1000);

    if (e10u >= 100) {
        ob.ensure(5);
        it = ob.pointer();
        it[4] = static_cast<CharT>('0' + e10u % 10);
        e10u /= 10;
        it[3] = static_cast<CharT>('0' + e10u % 10);
        it[2] = static_cast<CharT>('0' + e10u / 10);
        adv = 5;
    } else if (e10u >= 10) {
        ob.ensure(4);
        it = ob.pointer();
        it[3] = static_cast<CharT>('0' + e10u % 10);
        it[2] = static_cast<CharT>('0' + e10u / 10);
    } else {
        ob.ensure(4);
        it = ob.pointer();
        it[3] = static_cast<CharT>('0' + e10u);
        it[2] = '0';
    }
    it[0] = 'E' | ((lc != strf::uppercase) << 5);
    it[1] = static_cast<CharT>('+' + ((exponent < 0) << 1));
    ob.advance(adv);
}

template <typename CharT>
STRF_HD void print_nan(strf::basic_outbuff<CharT>& ob, strf::lettercase lc)
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
template <typename CharT>
STRF_HD void print_nan(strf::basic_outbuff<CharT>& ob, strf::lettercase lc
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

template <typename CharT>
STRF_HD void print_inf(strf::basic_outbuff<CharT>& ob, strf::lettercase lc)
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

template <typename CharT>
STRF_HD void print_inf( strf::basic_outbuff<CharT>& ob
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

template <typename CharT>
class punct_double_printer: public strf::printer<CharT>
{
public:

    template < typename Preview, typename FPack
             , strf::float_notation Notation, typename FloatT
             , bool HasAlignment >
    STRF_HD punct_double_printer
        ( const strf::detail::fmt_double_printer_input
            < CharT, Preview, FPack, FloatT, Notation, HasAlignment >& input )
        : lettercase_(strf::get_facet<strf::lettercase_c, FloatT>(input.facets))
    {
        static_assert(Notation != strf::float_notation::hex, "");

        auto enc = get_facet<strf::char_encoding_c<CharT>, FloatT>(input.facets);
        auto punct = strf::get_facet<strf::numpunct_c<10>, FloatT>(input.facets);
        grouping_ = punct.grouping();
        decimal_point_ = punct.decimal_point();
        thousands_sep_ = punct.thousands_sep();
        auto dd = detail::decode(input.arg.value());
        init_(input.preview, dd, input.arg.get_float_format(), enc);
        init_fill_(input.arg.get_alignment_format(), input.preview, enc);
    }

    STRF_HD void print_to(strf::basic_outbuff<CharT>&) const override;

private:

    template < typename Preview, strf::float_notation Notation, typename Encoding >
    STRF_HD void init_
        ( Preview& preview, detail::double_dec dd
        , strf::float_format<Notation> ffmt, Encoding enc )noexcept;

    template <typename Preview, typename Encoding>
    STRF_HD void init_fill_
        ( strf::default_alignment_format, Preview&, Encoding ) noexcept;

    template <typename Preview, typename Encoding>
    STRF_HD void init_fill_
        ( strf::alignment_format afmt, Preview& preview, Encoding enc ) noexcept;

    STRF_HD void print_fixed_
        ( strf::basic_outbuff<CharT>& ob ) const noexcept;

    STRF_HD void print_scientific_
        ( strf::basic_outbuff<CharT>& ob ) const noexcept;

    STRF_HD void print_inf_or_nan_
        ( strf::basic_outbuff<CharT>& ob ) const noexcept;

    strf::encode_char_f<CharT> encode_char_;
    strf::encode_fill_f<CharT> encode_fill_;
    strf::digits_grouping grouping_;
    char32_t fillchar_ = U' ';
    unsigned left_fillcount_ = 0;
    unsigned right_fillcount_ = 0;
    unsigned sep_size_ = 0;
    unsigned decimal_point_size_ = 0;
    char32_t decimal_point_;
    char32_t thousands_sep_;
    strf::lettercase lettercase_;
    strf::detail::double_printer_data data_;
};

template <typename CharT>
template < typename Preview, strf::float_notation Notation, typename Encoding >
STRF_HD void punct_double_printer<CharT>::init_
    ( Preview& preview, detail::double_dec dd
    , strf::float_format<Notation> ffmt, Encoding enc ) noexcept
{
    encode_char_ = enc.encode_char_func();
    auto sep_validation = strf::invalid_char_len;
    if ( ! grouping_.empty()) {
        sep_validation = enc.validate(thousands_sep_);
        if (sep_validation == strf::invalid_char_len) {
            grouping_ = strf::digits_grouping{};
        }
    }
    data_ = strf::detail::init_double_printer_data(dd, ffmt, grouping_);
    int content_width =
        ( detail::inf_or_nan(data_.form) || data_.pad0width < data_.chars_count
        ? (int)data_.chars_count
        : data_.pad0width );
    std::size_t content_size = content_width;
    (void) content_width;
    (void) content_size;
    if (data_.sep_count > 0) {
        sep_size_ = static_cast<unsigned>(sep_validation);
        if (sep_size_ == 1) {
            CharT little_sep[4];
            enc.encode_char(little_sep, thousands_sep_);
            thousands_sep_ = little_sep[0];
        } else {
            STRF_IF_CONSTEXPR (Preview::size_required) {
                content_size -= data_.sep_count;
                content_size += data_.sep_count * sep_size_;
            }
        }
    }
    if (data_.showpoint) {
        auto validation = enc.validate(decimal_point_);
        if (validation == 1) {
            decimal_point_size_ = 1;
            CharT ch;
            enc.encode_char(&ch, decimal_point_);
            decimal_point_ = ch;
        } else{
            if (validation != strf::invalid_char_len) {
                decimal_point_size_ = static_cast<unsigned>(validation);
            } else {
                decimal_point_size_ = static_cast<unsigned>(enc.replacement_char_size());
                decimal_point_ = enc.replacement_char();
            }
            STRF_IF_CONSTEXPR (Preview::size_required) {
                --content_size;
                content_size += decimal_point_size_;
            }
        }
    }
    preview.subtract_width(content_width);
    preview.add_size(content_size);
}

template <typename CharT>
template <typename Preview, typename Encoding>
STRF_HD void punct_double_printer<CharT>::init_fill_
    ( strf::default_alignment_format, Preview& preview, Encoding ) noexcept
{
    if (detail::inf_or_nan(data_.form) && data_.pad0width > data_.chars_count) {
        encode_fill_ = strf::detail::trivial_fill_f;
        // bool fill_sign_space = data_.sign == ' ';
        // data_.showsign &= ! fill_sign_space;
        // data_.chars_count -= fill_sign_space;
        left_fillcount_ = data_.pad0width - (int)data_.chars_count;// + fill_sign_space;
        preview.subtract_width(left_fillcount_);
        preview.add_size(left_fillcount_);
    }
}

template <typename CharT>
template <typename Preview, typename Encoding>
STRF_HD void punct_double_printer<CharT>::init_fill_
    ( strf::alignment_format afmt, Preview& preview, Encoding enc ) noexcept
{
    encode_fill_ = enc.encode_fill_func();
    fillchar_ = afmt.fill;
    int fmt_width = afmt.width.round();
    auto content_width = data_.chars_count;
    if (detail::inf_or_nan(data_.form)) {
        if (fmt_width < (int)data_.pad0width) {
            fmt_width = data_.pad0width;
        }
    } else if (data_.pad0width > content_width) {
        content_width = data_.pad0width;
    }
    const bool sign_space = data_.sign == ' ';
    unsigned whole_fillcount = 0;
    if ((int)content_width < fmt_width) {
        auto partial_fillcount = fmt_width - content_width;
        whole_fillcount = partial_fillcount + sign_space;
        data_.showsign &= ! sign_space;
        data_.chars_count -= sign_space;
        if (data_.pad0width) {
            data_.pad0width -= sign_space;
        }
        preview.subtract_width(partial_fillcount);
        switch (afmt.alignment) {
            case strf::text_alignment::right:
                left_fillcount_ = whole_fillcount;
                break;
            case strf::text_alignment::left:
                left_fillcount_ = sign_space;
                right_fillcount_ = partial_fillcount;
                break;
            default:
                STRF_ASSERT(afmt.alignment == strf::text_alignment::center);
                auto half_fillcount = partial_fillcount >> 1;
                left_fillcount_ = half_fillcount + sign_space;
                right_fillcount_ = half_fillcount + (partial_fillcount & 1);
        }
    } else if (sign_space && afmt.fill != ' ') {
        left_fillcount_ = sign_space;
        whole_fillcount = sign_space;
        data_.showsign &= ! sign_space;
        data_.chars_count -= sign_space;
        if (data_.pad0width) {
            data_.pad0width -= sign_space;
        }
    }
    if (Preview::size_required && whole_fillcount) {
        std::size_t fillchar_size = enc.encoded_char_size(fillchar_);
        preview.add_size(fillchar_size * whole_fillcount - sign_space);
    }
}

template <typename CharT>
STRF_HD void punct_double_printer<CharT>::print_to
    (strf::basic_outbuff<CharT>& ob) const
{
    if (left_fillcount_ != 0) {
        encode_fill_(ob, left_fillcount_, fillchar_);
    }
    switch (data_.form) {
        case detail::float_form::fixed:
            print_fixed_(ob);
            break;
        case detail::float_form::sci:
            print_scientific_(ob);
            break;
        default:
            print_inf_or_nan_(ob);
    }
    if (right_fillcount_ != 0) {
        encode_fill_(ob, right_fillcount_, fillchar_);
    }
}

template <typename CharT>
STRF_HD void punct_double_printer<CharT>::print_fixed_
    ( strf::basic_outbuff<CharT>& ob ) const noexcept
{
    if (data_.showsign) {
        put(ob, static_cast<CharT>(data_.sign));
    }
    if (data_.pad0width > data_.chars_count) {
        int count = data_.pad0width - data_.chars_count;
        strf::detail::write_fill(ob, count, (CharT)'0');
    }
    if (data_.e10 >= 0) {
        if (data_.sep_count == 0) {
            strf::detail::write_int<10>( ob, data_.m10, data_.m10_digcount
                                       , strf::lowercase );
            strf::detail::write_fill(ob, data_.e10, (CharT)'0');
        } else if (sep_size_ == 1) {
            strf::detail::print_amplified_integer_small_separator
                ( ob, data_.m10, grouping_, data_.m10_digcount, data_.e10
                , static_cast<CharT>(thousands_sep_) );
        } else {
            strf::detail::print_amplified_integer_big_separator
                ( ob, encode_char_, data_.m10, grouping_, data_.m10_digcount
                , data_.e10, sep_size_, thousands_sep_ );
        }
        if (decimal_point_size_ == 1) {
            strf::put(ob, static_cast<CharT>(decimal_point_));
        } else if (decimal_point_size_ != 0) {
            ob.ensure(decimal_point_size_);
            ob.advance_to(encode_char_(ob.pointer(), decimal_point_));
        }
        if (data_.extra_zeros) {
            detail::write_fill(ob, data_.extra_zeros,  (CharT)'0');
        }
    } else {
        STRF_ASSERT(data_.e10 < 0);

        detail::chars_count_t e10u = - data_.e10;
        if (e10u >= data_.m10_digcount) {
            ob.ensure(1 + decimal_point_size_);
            auto it = ob.pointer();
            *it++ = static_cast<CharT>('0');
            if (decimal_point_size_ == 1) {
                *it++ = static_cast<CharT>(decimal_point_);
            } else {
                STRF_ASSERT(decimal_point_size_ != 0);
                it = encode_char_(it, decimal_point_);
            }
            ob.advance_to(it);

            if (e10u > data_.m10_digcount) {
                strf::detail::write_fill(ob, e10u - data_.m10_digcount, (CharT)'0');
            }
            strf::detail::write_int<10>( ob, data_.m10, data_.m10_digcount
                                       , strf::lowercase);
            if (data_.extra_zeros != 0) {
                strf::detail::write_fill(ob, data_.extra_zeros,  (CharT)'0');
            }
        } else {
            //auto v = std::lldiv(data_.m10, detail::pow10(e10u)); // todo test this
            auto p10 = strf::detail::pow10(e10u);
            auto integral_part = data_.m10 / p10;
            auto fractional_part = data_.m10 % p10;
            auto idigcount = data_.m10_digcount - e10u;

            STRF_ASSERT(idigcount == detail::count_digits<10>(integral_part));

            if (data_.sep_count == 0) {
                strf::detail::write_int<10>(ob, integral_part, idigcount, strf::lowercase);
            } else if (sep_size_ == 1) {
                strf::detail::write_int_little_sep<10>
                    ( ob, integral_part, grouping_, idigcount, data_.sep_count
                    , static_cast<CharT>(thousands_sep_) );
            } else {
                strf::detail::write_int_big_sep<10>
                    ( ob, encode_char_, integral_part, grouping_, thousands_sep_
                    , sep_size_, idigcount );
            }

            ob.ensure(decimal_point_size_);
            auto it = ob.pointer();
            if (decimal_point_size_ == 1) {
                *it++ = static_cast<CharT>(decimal_point_);
            } else {
                STRF_ASSERT(decimal_point_size_ != 0);
                it = encode_char_(it, decimal_point_);
            }
            ob.advance_to(it);

            strf::detail::write_int_with_leading_zeros<10>
                (ob, fractional_part, e10u, strf::lowercase);
            if (data_.extra_zeros) {
                detail::write_fill(ob, data_.extra_zeros,  (CharT)'0');
            }
        }
    }
}

template <typename CharT>
STRF_HD void punct_double_printer<CharT>::print_scientific_
    ( strf::basic_outbuff<CharT>& ob ) const noexcept
{
    if (data_.showsign) {
        put(ob, static_cast<CharT>(data_.sign));
    }
    if (data_.pad0width > data_.chars_count) {
        int count = data_.pad0width - data_.chars_count;
        strf::detail::write_fill(ob, count, (CharT)'0');
    }
    strf::detail::print_scientific_notation
        ( ob, encode_char_, data_.m10, data_.m10_digcount
        , decimal_point_, decimal_point_size_
        , data_.e10 + data_.m10_digcount - 1
        , data_.showpoint, data_.extra_zeros, lettercase_ );
}

template <typename CharT>
STRF_HD void punct_double_printer<CharT>::print_inf_or_nan_
    ( strf::basic_outbuff<CharT>& ob ) const noexcept
{
    if (data_.showsign) {
        put(ob, static_cast<CharT>(data_.sign));
    }
    if (data_.form == detail::float_form::nan) {
        strf::detail::print_nan(ob, lettercase_);
    } else {
        strf::detail::print_inf(ob, lettercase_);
    }
}

template <typename CharT>
class fast_double_printer: public strf::printer<CharT>
{
public:

    template <typename FloatT, typename Preview>
    STRF_HD fast_double_printer
        ( strf::detail::fast_double_printer_input<CharT, Preview, FloatT> input) noexcept
        : fast_double_printer(input.value, input.lcase)
    {
        std::size_t s = 0;
        STRF_IF_CONSTEXPR (Preview::width_required || Preview::size_required) {
            s = size();
        }
        input.preview.subtract_width(s);
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

    STRF_HD void print_to(strf::basic_outbuff<CharT>&) const override;

    STRF_HD std::size_t size() const;

private:

    const detail::double_dec value_;
    bool sci_notation_ ;
    const unsigned m10_digcount_;
    strf::lettercase lettercase_;
};

template <typename CharT>
STRF_HD std::size_t fast_double_printer<CharT>::size() const
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

template <typename CharT>
STRF_HD void fast_double_printer<CharT>::print_to
    ( strf::basic_outbuff<CharT>& ob ) const
{
    if (value_.nan) {
        strf::detail::print_nan(ob, lettercase_, value_.negative);
    } else if (value_.infinity) {
        strf::detail::print_inf(ob, lettercase_, value_.negative);
    } else if (sci_notation_) {
        ob.ensure( value_.negative + m10_digcount_ + (m10_digcount_ != 1) + 4
                 + (value_.e10 > 99 || value_.e10 < -99) );
        CharT* it = ob.pointer();
        if (value_.negative) {
            * it = '-';
            ++it;
        }
        if (m10_digcount_ == 1) {
            * it = static_cast<CharT>('0' + value_.m10);
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
        it[1] = static_cast<CharT>('+' + ((e10 < 0) << 1));
        unsigned e10u = std::abs(e10);
        if (e10u >= 100) {
            it[4] = static_cast<CharT>('0' + e10u % 10);
            e10u /= 10;
            it[3] = static_cast<CharT>('0' + e10u % 10);
            it[2] = static_cast<CharT>('0' + e10u / 10);
            it += 5;
        } else if (e10u >= 10) {
            it[3] = static_cast<CharT>('0' + e10u % 10);
            it[2] = static_cast<CharT>('0' + e10u / 10);
            it += 4;
        } else {
            it[3] = static_cast<CharT>('0' + e10u);
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
                detail::write_fill(ob, value_.e10, (CharT)'0');
            }
        } else {
            unsigned e10u = - value_.e10;
            if (e10u >= m10_digcount_) {
                it[0] = '0';
                it[1] = '.';
                ob.advance_to(it + 2);
                detail::write_fill(ob, e10u - m10_digcount_, (CharT)'0');

                ob.ensure(m10_digcount_);
                auto end = ob.pointer() + m10_digcount_;
                write_int_dec_txtdigits_backwards(value_.m10, end);
                ob.advance_to(end);
            } else {
                const char* const arr = strf::detail::chars_00_to_99();
                auto m = value_.m10;
                it += m10_digcount_ + 1;
                CharT* const end = it;
                while(e10u >= 2) {
                    auto index = (m % 100) << 1;
                    it[-2] = arr[index];
                    it[-1] = arr[index + 1];
                    it -= 2;
                    m /= 100;
                    e10u -= 2;
                }
                if (e10u != 0) {
                    *--it = static_cast<CharT>('0' + (m % 10));
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
                    *--it = static_cast<CharT>('0' + m);
                }
                ob.advance_to(end);
            }
        }
    }
}


template <typename CharT>
class fast_punct_double_printer: public strf::printer<CharT>
{
public:

    template <typename Preview, typename FPack, typename FloatT>
    STRF_HD fast_punct_double_printer
        ( const strf::detail::fast_punct_double_printer_input
              < CharT, Preview, FPack, FloatT >& input )
        : value_(decode(input.arg))
        , m10_digcount_(strf::detail::count_digits<10>(value_.m10))
        , sep_count_(0)
        , lettercase_(strf::get_facet<strf::lettercase_c, FloatT>(input.facets))
    {
        auto punct = strf::get_facet<strf::numpunct_c<10>, FloatT>(input.facets);
        grouping_ = punct.grouping();
        decimal_point_ = punct.decimal_point();
        thousands_sep_ = punct.thousands_sep();
        init_(strf::get_facet<strf::char_encoding_c<CharT>, FloatT>(input.facets));
        STRF_IF_CONSTEXPR (Preview::width_required) {
            input.preview.subtract_width(width_());
        }
        STRF_IF_CONSTEXPR (Preview::size_required) {
            input.preview.add_size(size_());
        }
    }


    STRF_HD void print_to(strf::basic_outbuff<CharT>&) const override;

private:

    template <typename Encoding>
    STRF_HD void init_(Encoding enc);

    STRF_HD strf::width_t width_() const;
    STRF_HD std::size_t size_() const;

    strf::encode_char_f<CharT> encode_char_;
    strf::digits_grouping grouping_;
    const detail::double_dec value_;
    const unsigned m10_digcount_;
    unsigned sep_count_ = 0;
    unsigned sep_size_ = 0;
    unsigned decimal_point_size_ = 0;
    char32_t decimal_point_ = '.';
    char32_t thousands_sep_;
    strf::lettercase lettercase_;
    bool sci_notation_ ;

};

template <typename CharT>
template <typename Encoding>
STRF_HD void fast_punct_double_printer<CharT>::init_(Encoding enc)
{
    encode_char_ = enc.encode_char_func();
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
            if (grouping_.any_separator(int_dig_count)){
                auto sep_validation = enc.validate(thousands_sep_);
                if (sep_validation != strf::invalid_char_len) {
                    sep_count_ = grouping_.separators_count(int_dig_count);
                    if (scientific_width < fixed_width + (int)sep_count_) {
                        sep_count_ = 0;
                        sci_notation_ = true;
                        showpoint = m10_digcount_ != 1;
                        goto init_decimal_point;
                    }
                    sep_size_ = static_cast<unsigned>(sep_validation);
                    if (sep_size_ == 1) {
                        CharT little_sep;
                        encode_char_(&little_sep, thousands_sep_);
                        thousands_sep_ = little_sep;
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
        auto validation = enc.validate(decimal_point_);
        if (validation == 1) {
            decimal_point_size_ = 1;
            CharT ch;
            enc.encode_char(&ch, decimal_point_);
            decimal_point_ = ch;
        } else if (validation != strf::invalid_char_len) {
            decimal_point_size_ = static_cast<unsigned>(validation);
        } else {
            decimal_point_size_ = static_cast<unsigned>(enc.replacement_char_size());
            decimal_point_ = enc.replacement_char();
        }
    }
}


template <typename CharT>
STRF_HD std::size_t fast_punct_double_printer<CharT>::size_() const
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

template <typename CharT>
STRF_HD strf::width_t fast_punct_double_printer<CharT>::width_() const
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
    auto sep_w = sep_count_;
    auto idigcount = (int)m10_digcount_ + value_.e10;
    if (value_.e10 < 0) {
        auto w = value_.negative + m10_digcount_ + decpoint_width + sep_w;
        return static_cast<std::int16_t>(w);
    }
    return static_cast<std::int16_t>(value_.negative + idigcount + sep_w);
}

template <typename CharT>
STRF_HD void fast_punct_double_printer<CharT>::print_to
    ( strf::basic_outbuff<CharT>& ob ) const
{
    if (value_.negative) {
        put(ob, static_cast<CharT>('-'));
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
                strf::detail::write_fill(ob, value_.e10, (CharT)'0');
            } else if (sep_size_ == 1) {
                strf::detail::print_amplified_integer_small_separator
                    ( ob, value_.m10, grouping_, m10_digcount_, value_.e10
                    , static_cast<CharT>(thousands_sep_) );
            } else {
                strf::detail::print_amplified_integer_big_separator
                    ( ob, encode_char_, value_.m10, grouping_, m10_digcount_
                    , value_.e10, sep_size_, thousands_sep_ );
            }
        } else {
            unsigned e10u = - value_.e10;
            if (e10u >= m10_digcount_) {
                ob.ensure(1 + decimal_point_size_);
                auto it = ob.pointer();
                *it = static_cast<CharT>('0');
                if (decimal_point_size_ == 1) {
                    it[1] = static_cast<CharT>(decimal_point_);
                    ob.advance_to(it + 2);
                } else {
                    ob.advance_to(encode_char_(it + 1, decimal_point_));
                }
                if (e10u > m10_digcount_) {
                    strf::detail::write_fill(ob, e10u - m10_digcount_, (CharT)'0');
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
                        ( ob, integral_part, grouping_, idigcount, sep_count_
                        , static_cast<CharT>(thousands_sep_), strf::lowercase);
                } else {
                    strf::detail::write_int_big_sep<10>
                        ( ob, encode_char_, integral_part, grouping_, thousands_sep_
                        , sep_size_, idigcount, strf::lowercase );
                }
                ob.ensure(decimal_point_size_);
                if (decimal_point_size_ == 1) {
                    *ob.pointer() = static_cast<CharT>(decimal_point_);
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

inline STRF_HD detail::chars_count_t exponent_hex_digcount(std::uint32_t abs_exponent)
{
    return 1 + (abs_exponent >= 1000) + (abs_exponent >= 100) + (abs_exponent >= 10);
}

inline STRF_HD detail::chars_count_t mantissa_hex_digcount(std::uint64_t mantissa)
{
    STRF_ASSERT(mantissa != 0);
    STRF_ASSERT(mantissa == (mantissa & 0xFFFFFFFFFFFFFull));

#if defined(STRF_USE_STD_BITOPS)
    unsigned lz = std::countl_zero(mantissa) >> 3;
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
    return static_cast<detail::chars_count_t>(13 - lz);
}

struct hex_double_printer_data
{
    std::uint64_t mantissa;
    std::int32_t exponent;
    detail::chars_count_t sub_chars_count;
    detail::chars_count_t pad0width;
    detail::chars_count_t exponent_digcount;
    detail::chars_count_t mantissa_digcount;
    detail::chars_count_t extra_zeros;
    bool showpoint;
    bool showsign;
    char sign;
};

#if ! defined(STRF_OMIT_IMPL)

STRF_FUNC_IMPL STRF_HD strf::detail::hex_double_printer_data init_hex_double_printer_data
    ( float_format<strf::float_notation::hex> fdata, double x ) noexcept
{
    strf::detail::hex_double_printer_data data;

    std::uint64_t bits = strf::detail::to_bits(x);

    data.mantissa = bits & 0xFFFFFFFFFFFFFull;
    data.exponent = static_cast<std::int32_t>((bits << 1) >> 53) - 1023;
    data.extra_zeros = 0;
    bool negative = bits & (1ull << 63);
    data.sign = negative ? '-' : static_cast<char>(fdata.sign);
    data.showsign = negative || fdata.sign != strf::showsign::negative_only;
    data.sub_chars_count = data.showsign;
    data.pad0width = fdata.pad0width;
    if (data.exponent == 1024) {
        data.sub_chars_count += 3;
        data.showpoint = false;
        data.exponent_digcount = 0;
    } else {
        data.exponent_digcount = 1;
        data.sub_chars_count += 5; // "0x0p+"
        if ((bits & 0x7FFFFFFFFFFFFFFFull) == 0) {
            data.mantissa_digcount = 0;
            data.extra_zeros = (fdata.precision != (detail::chars_count_t)-1) * fdata.precision;
            data.showpoint = data.extra_zeros || fdata.showpoint;
            data.sub_chars_count += 1 + data.showpoint;
        } else {
            if (data.exponent != -1023 || data.mantissa != 0) {
                data.exponent_digcount =
                    strf::detail::exponent_hex_digcount(std::abs(data.exponent));
            }
            data.sub_chars_count += data.exponent_digcount;
            if (data.mantissa == 0){
                data.mantissa_digcount = 0;
                data.extra_zeros = (fdata.precision != (detail::chars_count_t)-1) * fdata.precision;
                data.showpoint = data.extra_zeros || fdata.showpoint;
                data.sub_chars_count += data.showpoint;
            } else {
                data.mantissa_digcount = strf::detail::mantissa_hex_digcount(data.mantissa);
                if (fdata.precision >= data.mantissa_digcount) {
                    static_assert(std::is_unsigned<detail::chars_count_t>::value, "");
                    data.showpoint = true;
                    data.sub_chars_count += 1 + data.mantissa_digcount;
                    if (fdata.precision != (detail::chars_count_t)-1) {
                        data.extra_zeros = fdata.precision - data.mantissa_digcount;
                    }
                } else {
                    // round mantissa if necessary
                    unsigned s = (13 - fdata.precision) << 2;
                    auto d = 1ull << s;
                    auto mask = d - 1;
                    auto mantissa_low = data.mantissa & mask;
                    if ( mantissa_low > (d >> 1)) {
                        data.mantissa += d;
                    } else if (mantissa_low == (d >> 1) && (data.mantissa & d)) {
                        data.mantissa += d;
                    }
                    data.mantissa_digcount = fdata.precision;
                    data.showpoint = fdata.precision || fdata.showpoint;
                    data.sub_chars_count += data.showpoint + data.mantissa_digcount;
                }
            }
        }
    }
    return data;
}

#else // ! defined(STRF_OMIT_IMPL)

STRF_HD strf::detail::hex_double_printer_data init_hex_double_printer_data
    ( float_format<strf::float_notation::hex> fdata, double d ) noexcept;

#endif // ! defined(STRF_OMIT_IMPL)

template <typename CharT>
class hex_double_printer: public strf::printer<CharT>
{
public:

    template <typename Preview, typename FPack, typename FloatT, bool HasAlignment>
    STRF_HD hex_double_printer
        ( const strf::detail::fmt_double_printer_input
            < CharT, Preview, FPack, FloatT, strf::float_notation::hex, HasAlignment>&
            input )
        : data_( strf::detail::init_hex_double_printer_data
                   ( input.arg.get_float_format(), input.arg.value() ) )
        , lettercase_(strf::get_facet<strf::lettercase_c, FloatT>(input.facets))
        , decimal_point_(strf::get_facet<strf::numpunct_c<16>, FloatT>(input.facets)
                         .decimal_point())
    {
        auto enc = strf::get_facet<strf::char_encoding_c<CharT>, FloatT>(input.facets);
        auto afmt = input.arg.get_alignment_format();
        init_(input.preview, enc, afmt);
    }

    STRF_HD void print_to(strf::basic_outbuff<CharT>&) const override;

private:

    template <typename Preview, typename Encoding>
    STRF_HD void init_
        ( Preview& preview
        , Encoding enc
        , strf::alignment_format afmt ) noexcept;

    template <typename Preview, typename Encoding>
    STRF_HD void init_
        ( Preview& preview
        , Encoding enc
        , strf::default_alignment_format afmt ) noexcept;

    strf::encode_char_f<CharT> encode_char_ = nullptr;
    strf::encode_fill_f<CharT> encode_fill_ = nullptr;
    strf::detail::hex_double_printer_data data_;
    strf::detail::chars_count_t left_fillcount_ = 0;
    strf::detail::chars_count_t right_fillcount_ = 0;
    strf::lettercase lettercase_;
    char32_t fillchar_ = ' ';
    char32_t decimal_point_ = '.';
    unsigned pointsize_ = 0;
};

template <typename CharT>
template <typename Preview, typename Encoding>
STRF_HD void hex_double_printer<CharT>::init_
    ( Preview& preview
    , Encoding enc
    , strf::alignment_format afmt ) noexcept
{
    encode_fill_ = enc.encode_fill_func();
    fillchar_ = afmt.fill;
    int rounded_fmt_width = afmt.width.round();
    detail::chars_count_t content_width;
    if (data_.exponent != 1024) {
        content_width = (detail::max)(data_.pad0width, data_.sub_chars_count + data_.extra_zeros);
    } else {
        content_width = data_.sub_chars_count;
        if ((int)data_.pad0width > rounded_fmt_width) {
            rounded_fmt_width = data_.pad0width;
        }
    }
    if (data_.showpoint) {
        encode_char_ = enc.encode_char_func();
        pointsize_ = static_cast<unsigned>(enc.encoded_char_size(decimal_point_));
        if (pointsize_ == 1) {
            CharT ch;
            enc.encode_char(&ch, decimal_point_);
            decimal_point_ = ch;
        }
        preview.add_size(pointsize_ - 1);
    }
    const bool fill_sign_space = data_.sign == ' ';
    detail::chars_count_t fillcount = 0;
    if (rounded_fmt_width <= (int)content_width) {
        preview.subtract_width(content_width);
        if (fill_sign_space && afmt.fill != ' ') {
            goto adapt_fill_sign_space;
        }
    } else {
        preview.subtract_width((unsigned)rounded_fmt_width);
        fillcount = static_cast<detail::chars_count_t>(rounded_fmt_width - content_width);
        switch(afmt.alignment) {
            case strf::text_alignment::left:
                right_fillcount_ = fillcount;
                break;
            case strf::text_alignment::right:
                left_fillcount_ = fillcount;
                break;
            default:
                STRF_ASSERT(afmt.alignment == strf::text_alignment::center);
                auto half_fillcount = fillcount >> 1;
                left_fillcount_ = half_fillcount;
                right_fillcount_ = half_fillcount + (fillcount & 1);
        }
        if (fill_sign_space) {
            adapt_fill_sign_space:
            data_.showsign = false;
            ++left_fillcount_;
            ++fillcount;
            --data_.sub_chars_count;
            --content_width;
            if (data_.pad0width) {
                --data_.pad0width;
            }
        }
    }
    preview.add_size(content_width);
    if (Preview::size_required && fillcount) {
        std::size_t fillchar_size = enc.encoded_char_size(fillchar_);
        preview.add_size(fillchar_size * fillcount);
    }
}

template <typename CharT>
template <typename Preview, typename Encoding>
STRF_HD void hex_double_printer<CharT>::init_
    ( Preview& preview
    , Encoding enc
    , strf::default_alignment_format ) noexcept
{
    detail::chars_count_t content_width = data_.sub_chars_count + data_.extra_zeros;
    if (data_.pad0width > content_width) {
        if (data_.exponent == 1024) {
            encode_fill_ = strf::detail::trivial_fill_f;
            left_fillcount_ = data_.pad0width - content_width;
        }
        content_width = data_.pad0width;
    }
    preview.subtract_width(content_width);
    if (data_.showpoint) {
        encode_char_ = enc.encode_char_func();
        pointsize_ = static_cast<unsigned>(enc.encoded_char_size(decimal_point_));
        if (pointsize_ == 1) {
            CharT ch;
            enc.encode_char(&ch, decimal_point_);
            decimal_point_ = ch;
        }
        preview.add_size(content_width - 1 + pointsize_);
    } else {
        preview.add_size(content_width);
    }
}

template <typename CharT>
STRF_HD void hex_double_printer<CharT>::print_to
    ( strf::basic_outbuff<CharT>& ob ) const
{
    if (left_fillcount_ > 0) {
        encode_fill_(ob, left_fillcount_, fillchar_);
    }
    if (data_.exponent != 1024) {
        std::size_t sub_size = data_.sub_chars_count + pointsize_ - data_.showpoint;
        ob.ensure(data_.sub_chars_count);
        auto it = ob.pointer();
        if (data_.showsign)  {
            *it++ = static_cast<CharT>(data_.sign);
        }
        *it++ = '0';
        *it++ = 'X' | ((lettercase_ != strf::uppercase) << 5);
        auto content_width = data_.sub_chars_count + data_.extra_zeros;
        if (data_.pad0width > content_width) {
            ob.advance_to(it);
            strf::detail::write_fill(ob, data_.pad0width - content_width, (CharT)'0');
            ob.ensure(sub_size - 2 - data_.showsign);
            it = ob.pointer();
        }
        *it ++ = 0x30 | int(data_.exponent != -1023); // '0' or  '1'
        if (pointsize_ == 1) {
            *it ++ = static_cast<CharT>(decimal_point_);
        } else if (pointsize_ != 0) {
            it = encode_char_(it, decimal_point_);
        }
        if (data_.mantissa != 0) {
            std::uint8_t digits[13] =
                { static_cast<std::uint8_t>((data_.mantissa & (0xFull << 48)) >> 48)
                , static_cast<std::uint8_t>((data_.mantissa & (0xFull << 44)) >> 44)
                , static_cast<std::uint8_t>((data_.mantissa & (0xFull << 40)) >> 40)
                , static_cast<std::uint8_t>((data_.mantissa & (0xFull << 36)) >> 36)
                , static_cast<std::uint8_t>((data_.mantissa & (0xFull << 32)) >> 32)
                , static_cast<std::uint8_t>((data_.mantissa & (0xFull << 28)) >> 28)
                , static_cast<std::uint8_t>((data_.mantissa & (0xFull << 24)) >> 24)
                , static_cast<std::uint8_t>((data_.mantissa & (0xFull << 20)) >> 20)
                , static_cast<std::uint8_t>((data_.mantissa & (0xFull << 16)) >> 16)
                , static_cast<std::uint8_t>((data_.mantissa & (0xFull << 12)) >> 12)
                , static_cast<std::uint8_t>((data_.mantissa & (0xFull <<  8)) >>  8)
                , static_cast<std::uint8_t>((data_.mantissa & (0xFull <<  4)) >>  4)
                , static_cast<std::uint8_t>(data_.mantissa & 0xFull) };

            const char offset_digit_a = ('A' | ((lettercase_ == strf::lowercase) << 5)) - 10;
            for(detail::chars_count_t i = 0; i < data_.mantissa_digcount; ++i) {
                auto digit = digits[i];
                *it ++ = ( digit < 10
                         ? ('0' + digit)
                         : (offset_digit_a + digit) );
            }
        }
        if (data_.extra_zeros) {
            ob.advance_to(it);
            detail::write_fill(ob, data_.extra_zeros,  (CharT)'0');
            ob.ensure(2 + data_.exponent_digcount);
            it = ob.pointer();
        }
        if (data_.exponent == -1023) {
            if (data_.mantissa == 0) {
                it[0] = 'P' | ((lettercase_ != strf::uppercase) << 5);
                it[1] = '+';
                it[2] = '0';
                ob.advance_to(it + 3);
            } else {
                it[0] = 'P' | ((lettercase_ != strf::uppercase) << 5);
                it[1] = '-';
                it[2] = '1';
                it[3] = '0';
                it[4] = '2';
                it[5] = '2';
                ob.advance_to(it + 6);
            }
        } else {
            it[0] = 'P' | ((lettercase_ != strf::uppercase) << 5);
            it[1] = static_cast<CharT>('+') + ((data_.exponent < 0) << 1);
            it += 2 + data_.exponent_digcount;
            strf::detail::write_int_dec_txtdigits_backwards
                ( strf::detail::unsigned_abs(data_.exponent), it );
            ob.advance_to(it);
        }
    } else {
        if (data_.showsign) {
            put(ob, static_cast<CharT>(data_.sign));
        }
        if (data_.mantissa == 0) {
            strf::detail::print_inf(ob, lettercase_);
        } else {
            strf::detail::print_nan(ob, lettercase_);
        }
    }
    if (right_fillcount_ > 0) {
        encode_fill_(ob, right_fillcount_, fillchar_);
    }
}

#if defined(STRF_SEPARATE_COMPILATION)

#if defined(__cpp_char8_t)
STRF_EXPLICIT_TEMPLATE class punct_double_printer<char8_t>;
STRF_EXPLICIT_TEMPLATE class fast_double_printer<char8_t>;
STRF_EXPLICIT_TEMPLATE class fast_punct_double_printer<char8_t>;
STRF_EXPLICIT_TEMPLATE class hex_double_printer<char8_t>;
#endif

STRF_EXPLICIT_TEMPLATE class punct_double_printer<char>;
STRF_EXPLICIT_TEMPLATE class punct_double_printer<char16_t>;
STRF_EXPLICIT_TEMPLATE class punct_double_printer<char32_t>;
STRF_EXPLICIT_TEMPLATE class punct_double_printer<wchar_t>;

STRF_EXPLICIT_TEMPLATE class fast_double_printer<char>;
STRF_EXPLICIT_TEMPLATE class fast_double_printer<char16_t>;
STRF_EXPLICIT_TEMPLATE class fast_double_printer<char32_t>;
STRF_EXPLICIT_TEMPLATE class fast_double_printer<wchar_t>;

STRF_EXPLICIT_TEMPLATE class fast_punct_double_printer<char>;
STRF_EXPLICIT_TEMPLATE class fast_punct_double_printer<char16_t>;
STRF_EXPLICIT_TEMPLATE class fast_punct_double_printer<char32_t>;
STRF_EXPLICIT_TEMPLATE class fast_punct_double_printer<wchar_t>;

STRF_EXPLICIT_TEMPLATE class hex_double_printer<char>;
STRF_EXPLICIT_TEMPLATE class hex_double_printer<char16_t>;
STRF_EXPLICIT_TEMPLATE class hex_double_printer<char32_t>;
STRF_EXPLICIT_TEMPLATE class hex_double_printer<wchar_t>;

#endif // defined(STRF_SEPARATE_COMPILATION)

} // namespace detail

} // namespace strf

#endif  // STRF_DETAIL_INPUT_TYPES_FLOAT_HPP

