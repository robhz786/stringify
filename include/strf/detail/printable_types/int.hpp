#ifndef STRF_DETAIL_INPUT_TYPES_INT_HPP_INCLUDED
#define STRF_DETAIL_INPUT_TYPES_INT_HPP_INCLUDED

//  Distributed under the Boost Software License, Version 1.0.
//  (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

#include <strf/printer.hpp>
#include <strf/facets_pack.hpp>
#include <strf/detail/facets/char_encoding.hpp>
#include <strf/detail/facets/numpunct.hpp>
#include <strf/detail/int_digits.hpp>
#include <strf/detail/standard_lib_functions.hpp>

// todo: optimize as in:
// https://pvk.ca/Blog/2017/12/22/appnexus-common-framework-its-out-also-how-to-print-integers-faster/

namespace strf {

template <int Base>
struct int_formatter;

template <int Base>
struct int_format
{
    unsigned precision = 0;
    unsigned pad0width = 0;
    strf::showsign sign = strf::showsign::negative_only;
    bool showbase = false;
    constexpr static int base = Base;
};

template <int ToBase, int FromBase>
constexpr STRF_HD int_format<ToBase> change_base(int_format<FromBase> f) noexcept
{
    return {f.precision, f.pad0width, f.sign, f.showbase};
}

template <int Base>
constexpr STRF_HD bool operator==( strf::int_format<Base> lhs
                                 , strf::int_format<Base> rhs ) noexcept
{
    return lhs.precision == rhs.precision
        && lhs.pad0width == rhs.pad0width
        && lhs.sign == rhs.sign
        && lhs.showbase == rhs.showbase;
}

template <int Base>
constexpr STRF_HD bool operator!=( strf::int_format<Base> lhs
                                 , strf::int_format<Base> rhs ) noexcept
{
    return ! (lhs == rhs);
}

template <class T, int Base>
class int_formatter_fn
{
private:

    template <int OtherBase>
    using adapted_derived_type_
        = strf::fmt_replace<T, int_formatter<Base>, int_formatter<OtherBase> >;

public:

    constexpr STRF_HD int_formatter_fn()  noexcept { }

    template <typename U>
    constexpr STRF_HD int_formatter_fn(const int_formatter_fn<U, Base> & u) noexcept
        : data_(u.get_int_format())
    {
    }

    constexpr STRF_HD explicit int_formatter_fn(int_format<Base> data) noexcept
        : data_(data)
    {
    }

    template < int B = 16 >
    constexpr STRF_HD std::enable_if_t<Base == B && B == 16, T&&>
    hex() &&
    {
        return static_cast<T&&>(*this);
    }

    template < int B = 16 >
    constexpr STRF_HD std::enable_if_t<Base != B && B == 16, adapted_derived_type_<B>>
    hex() const &
    {
        return adapted_derived_type_<B>
            { static_cast<const T&>(*this)
            , strf::tag<strf::int_formatter<B>>{}
            , strf::change_base<B>(data_) };
    }

    template < int B = 10 >
    constexpr STRF_HD std::enable_if_t<Base == B && B == 10, T&&>
    dec() &&
    {
        return static_cast<T&&>(*this);
    }

    template < int B = 10 >
    constexpr STRF_HD std::enable_if_t<Base != B && B == 10, adapted_derived_type_<B>>
    dec() const &
    {
        return adapted_derived_type_<B>
            { static_cast<const T&>(*this)
            , strf::tag<strf::int_formatter<B>>{}
            , strf::change_base<B>(data_) };
    }

    template < int B = 8 >
    constexpr STRF_HD std::enable_if_t<Base == B && B == 8, T&&>
    oct() &&
    {
        return static_cast<T&&>(*this);
    }

    template < int B = 8 >
    constexpr STRF_HD std::enable_if_t<Base != B && B == 8, adapted_derived_type_<B>>
    oct() const &
    {
        return adapted_derived_type_<B>
            { static_cast<const T&>(*this)
            , strf::tag<strf::int_formatter<B>>{}
            , strf::change_base<B>(data_) };
    }

    template < int B = 2 >
    constexpr STRF_HD std::enable_if_t<Base == B && B == 2, T&&>
    bin() &&
    {
        return static_cast<T&&>(*this);
    }

    template < int B = 2 >
    constexpr STRF_HD std::enable_if_t<Base != B && B == 2, adapted_derived_type_<B>>
    bin() const &
    {
        return adapted_derived_type_<B>
            { static_cast<const T&>(*this)
            , strf::tag<strf::int_formatter<B>>{}
            , strf::change_base<B>(data_) };
    }

    constexpr STRF_HD T&& p(unsigned _) && noexcept
    {
        data_.precision = _;
        return static_cast<T&&>(*this);
    }
    constexpr STRF_HD T&& pad0(unsigned w) && noexcept
    {
        data_.pad0width = w;
        return static_cast<T&&>(*this);
    }
    template <bool DecimalBase = (Base == 10)>
    STRF_HD T&& operator+() && noexcept
    {
        static_assert(DecimalBase, "operator+ only allowed in decimal base");
        data_.sign = strf::showsign::positive_also;
        return static_cast<T&&>(*this);
    }
    template <bool DecimalBase = (Base == 10)>
    STRF_HD T&& fill_sign() && noexcept
    {
        static_assert(DecimalBase, "fill_sign() only allowed in decimal base");
        data_.sign = strf::showsign::fill_instead_of_positive;
        return static_cast<T&&>(*this);
    }
    template <bool DecimalBase = (Base == 10)>
    STRF_HD T&& operator~() && noexcept
    {
        static_assert(DecimalBase, "operator~ only allowed in decimal base");
        data_.sign = strf::showsign::fill_instead_of_positive;
        return static_cast<T&&>(*this);
    }

    template <bool DecimalBase = (Base == 10)>
    constexpr STRF_HD T&& operator*() && noexcept
    {
        static_assert(!DecimalBase, "operator* not allowed in decimal base");
        data_.showbase = true;
        return static_cast<T&&>(*this);
    }
    constexpr static STRF_HD int base() noexcept
    {
        return Base;
    }
    constexpr STRF_HD strf::int_format<Base> get_int_format() const noexcept
    {
        return data_;
    }
    constexpr STRF_HD T&& set_int_format(strf::int_format<Base> data) && noexcept
    {
        data_ = data;
        return static_cast<T&&>(*this);
    }
    template <int OtherBase>
    constexpr STRF_HD std::enable_if_t<Base != OtherBase, adapted_derived_type_<OtherBase>>
    set_int_format(strf::int_format<OtherBase> data) const & noexcept
    {
        return adapted_derived_type_<OtherBase>
            { static_cast<const T&>(*this)
            , strf::tag<strf::int_formatter<OtherBase>>{}
            , strf::change_base<OtherBase>(data) };
    }

private:

    strf::int_format<Base> data_;
};

template <int Base>
struct int_formatter
{
    template <typename T>
    using fn = strf::int_formatter_fn<T, Base>;
};

template <typename IntT, int Base, bool HasAlignment>
using int_with_formatters = strf::value_with_formatters
    < strf::print_traits<IntT>
    , strf::int_formatter<Base>
    , strf::alignment_formatter_q<HasAlignment> >;

namespace detail {

template <typename> class int_printer;
template <typename> class punct_int_printer;
template <typename, int> class punct_fmt_int_printer;

template <typename T>
constexpr STRF_HD bool negative_impl_(const T& x, std::integral_constant<bool, true>) noexcept
{
    return x < 0;
}
template <typename T>
constexpr STRF_HD bool negative_impl_(const T&, std::integral_constant<bool, false>) noexcept
{
    return false;
}
template <typename T>
constexpr STRF_HD bool negative(const T& x) noexcept
{
    return strf::detail::negative_impl_(x, std::is_signed<T>());
}

template <typename FPack, typename IntT, unsigned Base>
class has_intpunct_impl
{
public:

    static STRF_HD std::true_type  test_numpunct(strf::numpunct<Base>);
    static STRF_HD std::false_type test_numpunct(strf::default_numpunct<Base>);
    static STRF_HD std::false_type test_numpunct(strf::no_grouping<Base>);

    static STRF_HD const FPack& fp();

    using has_numpunct_type = decltype
        ( test_numpunct
            ( get_facet< strf::numpunct_c<Base>, IntT >(fp())) );
public:

    static constexpr bool value = has_numpunct_type::value;
};

template <typename FPack, typename IntT, unsigned Base>
constexpr STRF_HD bool has_intpunct()
{
    return has_intpunct_impl<FPack, IntT, Base>::value;
}

template <typename CharT, typename Preview, typename IntT>
struct nopunct_int_printer_input
{
    using printer_type = strf::detail::int_printer<CharT>;

    template<typename FPack>
    constexpr STRF_HD nopunct_int_printer_input
        ( Preview& preview_, const FPack&, IntT arg_) noexcept
        : preview(preview_)
        , value(arg_)
    {
    }

    Preview& preview;
    IntT value;
};

template <typename CharT, typename Preview, typename FPack, typename IntT>
struct punct_int_printer_input
{
    using printer_type = strf::detail::punct_int_printer<CharT>;
    using arg_type = IntT;

    Preview& preview;
    FPack facets;
    IntT value;
};

template <typename CharT, typename Preview, typename FPack, typename IntT>
using int_printer_input = std::conditional_t
    < strf::detail::has_intpunct<FPack, IntT, 10>()
    , strf::detail::punct_int_printer_input<CharT, Preview, FPack, IntT>
    , strf::detail::nopunct_int_printer_input<CharT, Preview, IntT> >;

template <typename IntT>
struct int_printing
{
    using override_tag = IntT;
    using forwarded_type = IntT;
    using formatters = strf::tag< strf::int_formatter<10>
                                , strf::alignment_formatter >;

    template <typename CharT, typename Preview, typename FPack>
    constexpr STRF_HD static auto make_printer_input
        (Preview& preview, const FPack& facets,  IntT x ) noexcept
        -> strf::detail::int_printer_input<CharT, Preview, FPack, IntT>
    {
        return {preview, facets, x};
    }

    template < typename CharT, typename Preview, typename FPack
             , int Base, bool HasAlignment >
    constexpr STRF_HD static auto make_printer_input
        ( Preview& preview
        , const FPack& facets
        , strf::int_with_formatters<IntT, Base, HasAlignment> x ) noexcept
        -> strf::usual_printer_input
            < CharT, Preview, FPack, strf::int_with_formatters<IntT, Base, HasAlignment>
            , strf::detail::punct_fmt_int_printer<CharT, Base> >
    {
        return {preview, facets, x};
    }
};

} // namespace detail

template <> struct print_traits<signed char>:
    public strf::detail::int_printing<signed char> {};
template <> struct print_traits<short>:
    public strf::detail::int_printing<short> {};
template <> struct print_traits<int>:
    public strf::detail::int_printing<int> {};
template <> struct print_traits<long>:
    public strf::detail::int_printing<long> {};
template <> struct print_traits<long long>:
    public strf::detail::int_printing<long long> {};

template <> struct print_traits<unsigned char>:
    public strf::detail::int_printing<unsigned char> {};
template <> struct print_traits<unsigned short>:
    public strf::detail::int_printing<unsigned short> {};
template <> struct print_traits<unsigned int>:
    public strf::detail::int_printing<unsigned int> {};
template <> struct print_traits<unsigned long>:
    public strf::detail::int_printing<unsigned long> {};
template <> struct print_traits<unsigned long long>:
    public strf::detail::int_printing<unsigned long long> {};

constexpr STRF_HD auto tag_invoke(strf::print_traits_tag, signed char) noexcept
    -> strf::print_traits<signed char>
    { return {}; }

constexpr STRF_HD auto tag_invoke(strf::print_traits_tag, short) noexcept
    -> strf::print_traits<short>
    { return {}; }

constexpr STRF_HD auto tag_invoke(strf::print_traits_tag, int) noexcept
    -> strf::print_traits<int>
    { return {}; }

constexpr STRF_HD auto tag_invoke(strf::print_traits_tag, long) noexcept
    -> strf::print_traits<long>
    { return {}; }

constexpr STRF_HD auto tag_invoke(strf::print_traits_tag, long long) noexcept
    -> strf::print_traits<long long>
    { return {}; }

constexpr STRF_HD auto tag_invoke(strf::print_traits_tag, unsigned char) noexcept
    -> strf::print_traits<unsigned char>
    { return {}; }

constexpr STRF_HD auto tag_invoke(strf::print_traits_tag, unsigned short) noexcept
    -> strf::print_traits<unsigned short>
    { return {}; }

constexpr STRF_HD auto tag_invoke(strf::print_traits_tag, unsigned int) noexcept
    -> strf::print_traits<unsigned int>
    { return {}; }

constexpr STRF_HD auto tag_invoke(strf::print_traits_tag, unsigned long) noexcept
    -> strf::print_traits<unsigned long>
    { return {}; }

constexpr STRF_HD auto tag_invoke(strf::print_traits_tag, unsigned long long) noexcept
    -> strf::print_traits<unsigned long long>
    { return {}; }

namespace detail {

struct voidptr_printing
{
    using override_tag = const void*;
    using forwarded_type = const void*;
    using formatters = strf::tag<strf::alignment_formatter>;

    template <typename CharT, typename Preview, typename FPack>
    constexpr STRF_HD static auto make_printer_input
        ( Preview& preview, const FPack& facets, const void* x ) noexcept
    {
        auto f1 = strf::get_facet<strf::numpunct_c<16>, const void*>(facets);
        auto f2 = strf::get_facet<strf::lettercase_c, const void*>(facets);
        auto f3 = strf::get_facet<strf::char_encoding_c<CharT>, const void*>(facets);
        auto fp2 = strf::pack(f1, f2, f3);
        auto x2 = *strf::hex(strf::detail::bit_cast<std::size_t>(x));
        return strf::make_default_printer_input<CharT>(preview, fp2, x2);
    }

    template <typename CharT, typename Preview, typename FPack, typename... T>
    constexpr STRF_HD static auto make_printer_input
        ( Preview& preview
        , const FPack& facets
        , strf::value_with_formatters<T...> x ) noexcept
    {
        auto f1 = strf::get_facet<strf::numpunct_c<16>, const void*>(facets);
        auto f2 = strf::get_facet<strf::lettercase_c, const void*>(facets);
        auto f3 = strf::get_facet<strf::char_encoding_c<CharT>, const void*>(facets);
        auto fp2 = strf::pack(f1, f2, f3);
        auto x2 = *strf::hex(strf::detail::bit_cast<std::size_t>(x.value()))
                             .set_alignment_format(x.get_alignment_format());
        return strf::make_default_printer_input<CharT>(preview, fp2, x2);
    }
};

} // namespace detail

constexpr STRF_HD auto tag_invoke(strf::print_traits_tag, const void*) noexcept
    -> strf::detail::voidptr_printing
    { return {}; }

namespace detail {

template <typename CharT>
class int_printer: public strf::printer<CharT>
{
public:

    template <typename... T>
    STRF_HD int_printer(strf::detail::nopunct_int_printer_input<T...> i)
    {
        init_(i.preview, i.value);
    }

    STRF_HD void print_to(strf::basic_outbuff<CharT>& ob) const override;

private:
    template <typename Preview>
    STRF_HD void init_(Preview& p, signed char value){ init2_(p, value); }
    template <typename Preview>
    STRF_HD void init_(Preview& p, short value){ init2_(p, value); }
    template <typename Preview>
    STRF_HD void init_(Preview& p, int value){ init2_(p, value); }
    template <typename Preview>
    STRF_HD void init_(Preview& p, long value){ init2_(p, value); }
    template <typename Preview>
    STRF_HD void init_(Preview& p, long long value){ init2_(p, value); }
    template <typename Preview>
    STRF_HD void init_(Preview& p, unsigned char value){ init2_(p, value); }
    template <typename Preview>
    STRF_HD void init_(Preview& p, unsigned short value){ init2_(p, value); }
    template <typename Preview>
    STRF_HD void init_(Preview& p, unsigned int value){ init2_(p, value); }
    template <typename Preview>
    STRF_HD void init_(Preview& p, unsigned long value){ init2_(p, value); }
    template <typename Preview>
    STRF_HD void init_(Preview& p, unsigned long long value){ init2_(p, value); }

    template <typename Preview, typename IntT>
    STRF_HD void init2_(Preview& preview, IntT value)
    {
        negative_ = strf::detail::negative(value);
        uvalue_ = strf::detail::unsigned_abs(value);
        digcount_ = strf::detail::count_digits<10>(uvalue_);
        auto size_ = digcount_ + negative_;
        preview.subtract_width(static_cast<std::int16_t>(size_));
        preview.add_size(size_);
    }


    unsigned long long uvalue_;
    unsigned digcount_;
    bool negative_;
};

template <typename CharT>
STRF_HD void int_printer<CharT>::print_to
    ( strf::basic_outbuff<CharT>& ob ) const
{
    unsigned size = digcount_ + negative_;
    ob.ensure(size);
    auto* it = write_int_dec_txtdigits_backwards(uvalue_, ob.pointer() + size);
    if (negative_) {
        it[-1] = '-';
    }
    ob.advance(size);
}

template <typename CharT>
class punct_int_printer: public strf::printer<CharT>
{
public:

    template <typename... T>
    STRF_HD punct_int_printer
        ( const strf::detail::punct_int_printer_input<T...>& i )
    {
        using int_type = typename strf::detail::punct_int_printer_input<T...>::arg_type;
        auto enc = get_facet<strf::char_encoding_c<CharT>, int_type>(i.facets);

        uvalue_ = strf::detail::unsigned_abs(i.value);
        digcount_ = strf::detail::count_digits<10>(uvalue_);
        auto punct = get_facet<strf::numpunct_c<10>, int_type>(i.facets);
        if (punct.any_group_separation(digcount_)) {
            grouping_ = punct.grouping();
            thousands_sep_ = punct.thousands_sep();
            std::size_t sepsize = enc.validate(thousands_sep_);
            if (sepsize != (std::size_t)-1) {
                sepsize_ = static_cast<unsigned>(sepsize);
                sepcount_ = punct.thousands_sep_count(digcount_);
                if (sepsize_ == 1) {
                    CharT little_sep[4];
                    enc.encode_char(little_sep, thousands_sep_);
                    thousands_sep_ = little_sep[0];
                } else {
                    encode_char_ = enc.encode_char_func();
                }
            }
        }
        negative_ = strf::detail::negative(i.value);
        i.preview.add_size(digcount_ + negative_ + sepsize_ * sepcount_);
        i.preview.subtract_width
            ( static_cast<std::int16_t>(sepcount_ + digcount_ + negative_) );
    }

    STRF_HD void print_to(strf::basic_outbuff<CharT>& ob) const override;

private:

    strf::encode_char_f<CharT> encode_char_;
    strf::digits_grouping grouping_;
    char32_t thousands_sep_;
    unsigned long long uvalue_;
    unsigned digcount_;
    unsigned sepcount_ = 0;
    unsigned sepsize_ = 0;
    bool negative_;
};

template <typename CharT>
STRF_HD void punct_int_printer<CharT>::print_to(strf::basic_outbuff<CharT>& ob) const
{
    if (sepcount_ == 0) {
        ob.ensure(negative_ + digcount_);
        auto it = ob.pointer();
        if (negative_) {
            *it = static_cast<CharT>('-');
            ++it;
        }
        it += digcount_;
        strf::detail::write_int_dec_txtdigits_backwards(uvalue_, it);
        ob.advance_to(it);
    } else {
        if (negative_) {
            put(ob, static_cast<CharT>('-'));
        }
        if (sepsize_ == 1) {
            strf::detail::write_int_little_sep<10>
                ( ob, uvalue_, grouping_, digcount_, sepcount_
                , static_cast<CharT>(thousands_sep_), strf::lowercase );
        } else {
            strf::detail::write_int_big_sep<10>
                ( ob, encode_char_, uvalue_, grouping_, thousands_sep_, sepsize_
                , digcount_, strf::lowercase );
        }
    }
}

struct fmt_int_printer_data {
    unsigned long long uvalue;
    unsigned digcount;
    unsigned leading_zeros;
    unsigned left_fillcount;
    unsigned right_fillcount;
    char32_t fillchar;
    bool has_prefix;
    char sign;
};

struct punct_fmt_int_printer_data: public fmt_int_printer_data {
    unsigned sepcount;
    unsigned sepsize;
    char32_t sepchar;
    strf::digits_grouping grouping;
};

template
    < typename IntT
    , std::enable_if_t<std::is_signed<IntT>::value, int> = 0 >
inline STRF_HD void init_1
    ( fmt_int_printer_data& data
    , strf::int_format<10> ifmt
    , IntT value ) noexcept
{
    if (value >= 0) {
        data.uvalue = value;
        data.sign = static_cast<char>(ifmt.sign);
        data.has_prefix = ifmt.sign != strf::showsign::negative_only;
    } else {
        using uvalue_type = decltype(data.uvalue);
        STRF_IF_CONSTEXPR (sizeof(IntT) < sizeof(data.uvalue)) {
            std::make_signed_t<uvalue_type> wide_value = value;
            data.uvalue = static_cast<uvalue_type>(-wide_value);
        } else {
            data.uvalue = 1 + static_cast<uvalue_type>(-(value + 1));
        }
        data.sign = '-';
        data.has_prefix = true;
    }
}

template
    < typename UIntT
    , std::enable_if_t<std::is_unsigned<UIntT>::value, int> = 0 >
inline STRF_HD void init_1
    ( fmt_int_printer_data& data
    , strf::int_format<10>
    , UIntT uvalue ) noexcept
{
    data.sign = '\0';
    data.has_prefix = false;
    data.uvalue = uvalue;
}

template <typename IntT, int Base>
inline STRF_HD void init_1
    ( fmt_int_printer_data& data
    , strf::int_format<Base>
    , IntT value ) noexcept
{
    data.uvalue = static_cast<decltype(data.uvalue)>(value);
}

struct punct_fmt_int_printer_data_init_result {
    unsigned sub_width;
    unsigned fillcount;
};

template <int Base>
STRF_HD punct_fmt_int_printer_data_init_result init_punct_fmt_int_printer_data
    ( punct_fmt_int_printer_data& data
    , strf::validate_f validate
    , strf::int_format<Base> ifmt
    , strf::alignment_format afmt ) noexcept
#if defined(STRF_OMIT_IMPL)
    ;
#else
{
    data.digcount = strf::detail::count_digits<Base>(data.uvalue);
    data.sepsize = 1;
    if (data.sepchar >= 0x80 && data.grouping.any_separator(data.digcount)) {
        auto sepsize = validate(data.sepchar);
        data.sepsize = static_cast<unsigned>(sepsize);
        if (sepsize == strf::invalid_char_len) {
            data.grouping = strf::digits_grouping{};
            data.sepsize = 0;
        }
    }
    data.sepcount = data.grouping.separators_count(data.digcount);
    unsigned prefix_size;
    STRF_IF_CONSTEXPR (Base == 10 ) {
        prefix_size = data.has_prefix;
    } else STRF_IF_CONSTEXPR (Base == 8 ) {
        if (ifmt.showbase && data.uvalue != 0) {
            data.has_prefix = true;
            prefix_size = 1;
            if (ifmt.precision > 0) {
                -- ifmt.precision;
            }
        } else {
            data.has_prefix = false;
            prefix_size = 0;
        }
    } else {
        data.has_prefix = ifmt.showbase;
        prefix_size = (unsigned)ifmt.showbase << 1;
    }
    unsigned content_width = data.digcount + data.sepcount + prefix_size;
    unsigned zeros_a = ifmt.precision > data.digcount ? ifmt.precision - data.digcount : 0;
    unsigned zeros_b = ifmt.pad0width > content_width ? ifmt.pad0width - content_width : 0;
    data.leading_zeros = (detail::max)(zeros_a, zeros_b);
    content_width += data.leading_zeros;
    auto fmt_width = afmt.width.round();
    data.fillchar = afmt.fill;
    bool fill_sign_space = Base == 10 && data.sign == ' ';
    if (fmt_width <= (int)content_width) {
        bool x = fill_sign_space && afmt.fill != ' ';
        data.left_fillcount = x;
        data.right_fillcount = 0;
        data.has_prefix &= !x;
        return {content_width - data.sepcount - x, x};
    }
    auto fillcount = static_cast<unsigned>(fmt_width - content_width);
    data.has_prefix &= !fill_sign_space;
    switch (afmt.alignment) {
        case strf::text_alignment::left:
            data.left_fillcount = fill_sign_space;
            data.right_fillcount = fillcount;
            break;
        case strf::text_alignment::right:
            data.left_fillcount = fillcount + fill_sign_space;
            data.right_fillcount = 0;
            break;
        default:
            auto halfcount = fillcount >> 1;
            data.left_fillcount = halfcount + fill_sign_space;
            data.right_fillcount = halfcount + (fillcount & 1);
    }
    return {content_width - data.sepcount - fill_sign_space, fillcount + fill_sign_space};
}
#endif // defined(STRF_OMIT_IMPL)

template <typename CharT, int Base>
class punct_fmt_int_printer: public printer<CharT>
{
public:

    template <typename... T>
    STRF_HD punct_fmt_int_printer
        ( const strf::usual_printer_input<T...>& i) noexcept
    {
        const auto ivalue = i.arg.value();
        using int_type = decltype(ivalue);
        lettercase_ = strf::get_facet<lettercase_c, int_type>(i.facets);
        const auto enc = strf::get_facet<char_encoding_c<CharT>, int_type>(i.facets);
        encode_fill_ = enc.encode_fill_func();
        encode_char_ = enc.encode_char_func();
        const auto punct = strf::get_facet<numpunct_c<Base>, int_type>(i.facets);
        data_.grouping = punct.grouping();
        data_.sepchar = punct.thousands_sep();
        const auto ifmt = i.arg.get_int_format();
        const auto afmt = i.arg.get_alignment_format();
        detail::init_1(data_, ifmt, ivalue);
        const auto w = detail::init_punct_fmt_int_printer_data<Base>
            (data_, enc.validate_func(), ifmt, afmt);
        i.preview.subtract_width(w.sub_width + w.fillcount + data_.sepcount);
        using preview_type = typename strf::usual_printer_input<T...>::preview_type;
        STRF_IF_CONSTEXPR (preview_type::size_required) {
            i.preview.add_size(w.sub_width);
            if (w.fillcount) {
                i.preview.add_size(w.fillcount * enc.encoded_char_size(afmt.fill));
            }
            i.preview.add_size(data_.sepcount * data_.sepsize);
        }

    }

    STRF_HD ~punct_fmt_int_printer();

    STRF_HD void print_to( strf::basic_outbuff<CharT>& ob ) const override;

private:

    strf::encode_fill_f<CharT> encode_fill_;
    strf::encode_char_f<CharT> encode_char_;
    strf::detail::punct_fmt_int_printer_data data_;
    strf::lettercase lettercase_;
};

template <typename CharT, int Base>
STRF_HD punct_fmt_int_printer<CharT, Base>::~punct_fmt_int_printer()
{
}

template <typename CharT, int Base>
STRF_HD void punct_fmt_int_printer<CharT, Base>::print_to
        ( strf::basic_outbuff<CharT>& ob ) const
{
    if (data_.left_fillcount > 0) {
        encode_fill_(ob, data_.left_fillcount, data_.fillchar);
    }
    if (data_.has_prefix) {
        STRF_IF_CONSTEXPR (Base == 10) {
            ob.ensure(1);
            * ob.pointer() = static_cast<CharT>(data_.sign);
            ob.advance();
        } else STRF_IF_CONSTEXPR (Base == 8) {
            ob.ensure(1);
            * ob.pointer() = static_cast<CharT>('0');
            ob.advance();
        } else {
            constexpr CharT xb = Base == 16 ? 'X' : 'B';
            ob.ensure(2);
            auto it = ob.pointer();
            it[0] = static_cast<CharT>('0');
            it[1] = static_cast<CharT>(xb | ((lettercase_ != strf::uppercase) << 5));
            ob.advance_to(it + 2);
        }
    }
    if (data_.leading_zeros > 0) {
        strf::detail::write_fill(ob, data_.leading_zeros, CharT('0'));
    }
    using dig_writer = detail::intdigits_writer<Base>;
    if (data_.sepcount == 0) {
        dig_writer::write(ob, data_.uvalue, data_.digcount, lettercase_);
    } else if (data_.sepsize == 1) {
        CharT sepchar = static_cast<CharT>(data_.sepchar);
        if (data_.sepchar >= 0x80) {
            encode_char_(&sepchar, data_.sepchar);
        }
        dig_writer::write_little_sep
            ( ob, data_.uvalue, data_.grouping, data_.digcount, data_.sepcount
            , sepchar, lettercase_ );
    } else {
        dig_writer::write_big_sep
            ( ob, encode_char_, data_.uvalue, data_.grouping
            , data_.sepchar, data_.sepsize, data_.digcount, lettercase_ );
    }
    if (data_.right_fillcount > 0) {
        encode_fill_(ob, data_.right_fillcount, data_.fillchar);
    }
}

#if defined(STRF_SEPARATE_COMPILATION)

STRF_EXPLICIT_TEMPLATE
STRF_HD punct_fmt_int_printer_data_init_result init_punct_fmt_int_printer_data<2>
    ( punct_fmt_int_printer_data& data
    , strf::validate_f validate
    , strf::int_format<2> ifmt
    , strf::alignment_format afmt ) noexcept;

STRF_EXPLICIT_TEMPLATE
STRF_HD punct_fmt_int_printer_data_init_result init_punct_fmt_int_printer_data<8>
    ( punct_fmt_int_printer_data& data
    , strf::validate_f validate
    , strf::int_format<8> ifmt
    , strf::alignment_format afmt ) noexcept;

STRF_EXPLICIT_TEMPLATE
STRF_HD punct_fmt_int_printer_data_init_result init_punct_fmt_int_printer_data<10>
    ( punct_fmt_int_printer_data& data
    , strf::validate_f validate
    , strf::int_format<10> ifmt
    , strf::alignment_format afmt ) noexcept;

STRF_EXPLICIT_TEMPLATE
STRF_HD punct_fmt_int_printer_data_init_result init_punct_fmt_int_printer_data<16>
    ( punct_fmt_int_printer_data& data
    , strf::validate_f validate
    , strf::int_format<16> ifmt
    , strf::alignment_format afmt ) noexcept;

#if defined(__cpp_char8_t)
STRF_EXPLICIT_TEMPLATE class int_printer<char8_t>;
STRF_EXPLICIT_TEMPLATE class punct_int_printer<char8_t>;
STRF_EXPLICIT_TEMPLATE class punct_fmt_int_printer<char8_t,  8>;
STRF_EXPLICIT_TEMPLATE class punct_fmt_int_printer<char8_t, 10>;
STRF_EXPLICIT_TEMPLATE class punct_fmt_int_printer<char8_t, 16>;
#endif

STRF_EXPLICIT_TEMPLATE class int_printer<char>;
STRF_EXPLICIT_TEMPLATE class int_printer<char16_t>;
STRF_EXPLICIT_TEMPLATE class int_printer<char32_t>;
STRF_EXPLICIT_TEMPLATE class int_printer<wchar_t>;

STRF_EXPLICIT_TEMPLATE class punct_int_printer<char>;
STRF_EXPLICIT_TEMPLATE class punct_int_printer<char16_t>;
STRF_EXPLICIT_TEMPLATE class punct_int_printer<char32_t>;
STRF_EXPLICIT_TEMPLATE class punct_int_printer<wchar_t>;


STRF_EXPLICIT_TEMPLATE class punct_fmt_int_printer<char,  8>;
STRF_EXPLICIT_TEMPLATE class punct_fmt_int_printer<char, 10>;
STRF_EXPLICIT_TEMPLATE class punct_fmt_int_printer<char, 16>;
STRF_EXPLICIT_TEMPLATE class punct_fmt_int_printer<char16_t,  8>;
STRF_EXPLICIT_TEMPLATE class punct_fmt_int_printer<char16_t, 10>;
STRF_EXPLICIT_TEMPLATE class punct_fmt_int_printer<char16_t, 16>;
STRF_EXPLICIT_TEMPLATE class punct_fmt_int_printer<char32_t,  8>;
STRF_EXPLICIT_TEMPLATE class punct_fmt_int_printer<char32_t, 10>;
STRF_EXPLICIT_TEMPLATE class punct_fmt_int_printer<char32_t, 16>;
STRF_EXPLICIT_TEMPLATE class punct_fmt_int_printer<wchar_t,  8>;
STRF_EXPLICIT_TEMPLATE class punct_fmt_int_printer<wchar_t, 10>;
STRF_EXPLICIT_TEMPLATE class punct_fmt_int_printer<wchar_t, 16>;

#endif // defined(STRF_SEPARATE_COMPILATION)

} // namespace detail

template <typename> struct is_int_number: public std::false_type {};
template <> struct is_int_number<short>: public std::true_type {};
template <> struct is_int_number<int>: public std::true_type {};
template <> struct is_int_number<long>: public std::true_type {};
template <> struct is_int_number<long long>: public std::true_type {};
template <> struct is_int_number<unsigned short>: public std::true_type {};
template <> struct is_int_number<unsigned int>: public std::true_type {};
template <> struct is_int_number<unsigned long>: public std::true_type {};
template <> struct is_int_number<unsigned long long>: public std::true_type {};

} // namespace strf

#endif // STRF_DETAIL_INPUT_TYPES_FMT_INT_HPP_INCLUDED
