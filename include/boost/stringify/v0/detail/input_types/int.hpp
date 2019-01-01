#ifndef BOOST_STRINGIFY_V0_DETAIL_INPUT_TYPES_INT_HPP
#define BOOST_STRINGIFY_V0_DETAIL_INPUT_TYPES_INT_HPP

//  Distributed under the Boost Software License, Version 1.0.
//  (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

#include <boost/stringify/v0/printer.hpp>
#include <boost/stringify/v0/facets_pack.hpp>
#include <boost/stringify/v0/detail/int_digits.hpp>
#include <boost/assert.hpp>
#include <algorithm>

BOOST_STRINGIFY_V0_NAMESPACE_BEGIN
namespace detail {

template <typename IntT, typename CharT>
class int_printer: public printer<CharT>
{

public:

    int_printer (IntT value)
        : _value(value)
        , _digcount(stringify::v0::detail::count_digits<10>(value))
    {
    }

    std::size_t necessary_size() const override;

    int remaining_width(int w) const override;

    stringify::v0::expected_output_buffer<CharT> write
        ( stringify::v0::output_buffer<CharT> buff
        , stringify::buffer_recycler<CharT>& recycler ) const override;

private:

    IntT _value;
    unsigned _digcount;

    CharT* _write(CharT* it) const noexcept;

    stringify::v0::expected_output_buffer<CharT> _buff_write
        ( stringify::v0::output_buffer<CharT> buff
        , stringify::buffer_recycler<CharT>& recycler ) const;
};

template <typename IntT, typename CharT>
std::size_t int_printer<IntT, CharT>::necessary_size() const
{
    return _value >= 0 ? _digcount : _digcount + 1;
}

template <typename IntT, typename CharT>
int int_printer<IntT, CharT>::remaining_width(int w) const
{
    int width = _value >= 0 ? _digcount : _digcount + 1;
    return w > width ? w - width : 0;
}

template <typename IntT, typename CharT>
stringify::v0::expected_output_buffer<CharT> int_printer<IntT, CharT>::write
    ( stringify::v0::output_buffer<CharT> buff
    , stringify::buffer_recycler<CharT>& recycler ) const
{
    std::size_t space = buff.end - buff.it;
    unsigned necessary_space = _value >= 0 ? _digcount : _digcount + 1;
    if (space >= necessary_space)
    {
        buff.it = _write(buff.it);
        return { stringify::v0::in_place_t{}, buff };
    }
    else
    {
        return _buff_write(buff, recycler);
    }
}

template <typename IntT, typename CharT>
CharT* int_printer<IntT, CharT>::_write(CharT* it) const noexcept
{
    if(_value < 0)
    {
        *it = '-';
        ++it;
    }
    auto end = it + _digcount;
    auto it2
        = stringify::v0::detail::write_int_dec_txtdigits_backwards<IntT, CharT>
        (_value, end);
    BOOST_ASSERT(it == it2);
    return end;
}


template <typename IntT, typename CharT>
stringify::v0::expected_output_buffer<CharT> int_printer<IntT, CharT>::_buff_write
    ( stringify::v0::output_buffer<CharT> buff
    , stringify::buffer_recycler<CharT>& recycler ) const
{
    char tmp[sizeof(CharT) * 3];
    char* tmp_end = tmp + sizeof(tmp) / sizeof(tmp[0]);
    auto it =
        stringify::v0::detail::write_int_dec_txtdigits_backwards<IntT, char>
        (_value, tmp_end);

    if (_value < 0)
    {
        if (buff.it != buff.end)
        {
            auto x = recycler.recycle(buff.it);
            BOOST_STRINGIFY_RETURN_ON_ERROR(x);
            buff = * x;
        }
        *buff.it = '-';
        ++buff.it;
    }

    auto count = _digcount;
    while(true)
    {
        std::size_t space = buff.end - buff.it;
        if (space < count)
        {
            std::copy_n(it, space, buff.it);
            it += space;
            count -= space;
            auto x = recycler.recycle(buff.it + space);
            BOOST_STRINGIFY_RETURN_ON_ERROR(x);
            buff = * x;
        }
        else
        {
            std::copy_n(it, count, buff.it);
            buff.it += count;
            return { stringify::v0::in_place_t{}, buff };
        }
    }
}

#if defined(BOOST_STRINGIFY_NOT_HEADER_ONLY)

BOOST_STRINGIFY_EXPLICIT_TEMPLATE class int_printer<int, char>;
BOOST_STRINGIFY_EXPLICIT_TEMPLATE class int_printer<int, char16_t>;
BOOST_STRINGIFY_EXPLICIT_TEMPLATE class int_printer<int, char32_t>;
BOOST_STRINGIFY_EXPLICIT_TEMPLATE class int_printer<int, wchar_t>;
BOOST_STRINGIFY_EXPLICIT_TEMPLATE class int_printer<long, char>;
BOOST_STRINGIFY_EXPLICIT_TEMPLATE class int_printer<long, char16_t>;
BOOST_STRINGIFY_EXPLICIT_TEMPLATE class int_printer<long, char32_t>;
BOOST_STRINGIFY_EXPLICIT_TEMPLATE class int_printer<long, wchar_t>;
BOOST_STRINGIFY_EXPLICIT_TEMPLATE class int_printer<long long, char>;
BOOST_STRINGIFY_EXPLICIT_TEMPLATE class int_printer<long long, char16_t>;
BOOST_STRINGIFY_EXPLICIT_TEMPLATE class int_printer<long long, char32_t>;
BOOST_STRINGIFY_EXPLICIT_TEMPLATE class int_printer<long long, wchar_t>;
BOOST_STRINGIFY_EXPLICIT_TEMPLATE class int_printer<unsigned int, char>;
BOOST_STRINGIFY_EXPLICIT_TEMPLATE class int_printer<unsigned int, char16_t>;
BOOST_STRINGIFY_EXPLICIT_TEMPLATE class int_printer<unsigned int, char32_t>;
BOOST_STRINGIFY_EXPLICIT_TEMPLATE class int_printer<unsigned int, wchar_t>;
BOOST_STRINGIFY_EXPLICIT_TEMPLATE class int_printer<unsigned long, char>;
BOOST_STRINGIFY_EXPLICIT_TEMPLATE class int_printer<unsigned long, char16_t>;
BOOST_STRINGIFY_EXPLICIT_TEMPLATE class int_printer<unsigned long, char32_t>;
BOOST_STRINGIFY_EXPLICIT_TEMPLATE class int_printer<unsigned long, wchar_t>;
BOOST_STRINGIFY_EXPLICIT_TEMPLATE class int_printer<unsigned long long, char>;
BOOST_STRINGIFY_EXPLICIT_TEMPLATE class int_printer<unsigned long long, char16_t>;
BOOST_STRINGIFY_EXPLICIT_TEMPLATE class int_printer<unsigned long long, char32_t>;
BOOST_STRINGIFY_EXPLICIT_TEMPLATE class int_printer<unsigned long long, wchar_t>;

#endif // defined(BOOST_STRINGIFY_NOT_HEADER_ONLY)


} // namespace detail

template <typename CharT, typename FPack>
inline stringify::v0::detail::int_printer<short, CharT>
make_printer(const FPack& fp, short x)
{
    (void)fp;
    return {x};
}
template <typename CharT, typename FPack>
inline stringify::v0::detail::int_printer<int, CharT>
make_printer(const FPack& fp, int x)
{
    (void)fp;
    return {x};
}
template <typename CharT, typename FPack>
inline stringify::v0::detail::int_printer<long, CharT>
make_printer(const FPack& fp, long x)
{
    (void)fp;
    return {x};
}
template <typename CharT, typename FPack>
inline stringify::v0::detail::int_printer<long long, CharT>
make_printer(const FPack& fp, long long x)
{
    (void)fp;
    return {x};
}
template <typename CharT, typename FPack>
inline stringify::v0::detail::int_printer<unsigned short, CharT>
make_printer(const FPack& fp, unsigned short x)
{
    (void)fp;
    return {x};
}
template <typename CharT, typename FPack>
inline stringify::v0::detail::int_printer<unsigned int, CharT>
make_printer(const FPack& fp, unsigned int x)
{
    (void)fp;
    return {x};
}
template <typename CharT, typename FPack>
inline stringify::v0::detail::int_printer<unsigned long, CharT>
make_printer(const FPack& fp, unsigned long x)
{
    (void)fp;
    return {x};
}
template <typename CharT, typename FPack>
inline stringify::v0::detail::int_printer<unsigned long long, CharT>
make_printer(const FPack& fp, unsigned long long x)
{
    (void)fp;
    return {x};
}

BOOST_STRINGIFY_V0_NAMESPACE_END

#endif  // BOOST_STRINGIFY_V0_DETAIL_INPUT_TYPES_INT_HPP
