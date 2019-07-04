#ifndef BOOST_STRINGIFY_V0_DETAIL_FACETS_NUMPUNCT_HPP
#define BOOST_STRINGIFY_V0_DETAIL_FACETS_NUMPUNCT_HPP

//  Distributed under the Boost Software License, Version 1.0.
//  (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

#include <string>
#include <boost/stringify/v0/config.hpp>

BOOST_STRINGIFY_V0_NAMESPACE_BEGIN

namespace detail {


class monotonic_grouping_impl
{
public:

    constexpr monotonic_grouping_impl(unsigned char groups_size)
        : _groups_size(groups_size)
    {
    }

    constexpr monotonic_grouping_impl(const monotonic_grouping_impl&) = default;

    unsigned get_thousands_sep_count(unsigned num_digits) const
    {
        return (_groups_size == 0 || num_digits == 0)
            ? 0
            : (num_digits - 1) / _groups_size;
    }

    unsigned char* get_groups
        ( unsigned num_digits
        , unsigned char* groups_array ) const;

private:

    const unsigned _groups_size;
};


class str_grouping_impl
{
public:

    str_grouping_impl(std::string grouping)
        : _grouping(std::move(grouping))
    {
    }

    str_grouping_impl(const str_grouping_impl&) = default;

    str_grouping_impl(str_grouping_impl&&) = default;

    unsigned get_thousands_sep_count(unsigned num_digits) const;

    unsigned char* get_groups( unsigned num_digits
                             , unsigned char* groups_array ) const;

private:

    std::string _grouping;
};


#if defined(BOOST_STRINGIFY_SOURCE) || ! defined(BOOST_STRINGIFY_SEPARATE_COMPILATION)

BOOST_STRINGIFY_INLINE
unsigned char* monotonic_grouping_impl::get_groups
    ( unsigned num_digits
    , unsigned char* groups_array ) const
{
    if (_groups_size == 0)
    {
        // this branch actually should never be executed
        BOOST_ASSERT(num_digits <= 0xFF);
        * groups_array = static_cast<std::uint8_t>(num_digits);
        return groups_array;
    }
    while(num_digits > _groups_size)
    {
        *groups_array = static_cast<unsigned char>(_groups_size);
        ++ groups_array;
        num_digits -= _groups_size;
    }
    *groups_array = static_cast<unsigned char>(num_digits);
    return groups_array;
}


BOOST_STRINGIFY_INLINE
unsigned str_grouping_impl::get_thousands_sep_count(unsigned num_digits) const
{
    if (_grouping.empty())
    {
        return 0;
    }
    unsigned count = 0;
    for(auto ch : _grouping)
    {
        auto grp = static_cast<unsigned>(ch);
        if(grp == 0 || grp >= num_digits)
        {
            return count;
        }
        if(grp < num_digits)
        {
            ++ count;
            num_digits -= grp;
        }
    }

    return count + (num_digits - 1) / _grouping.back();
}


BOOST_STRINGIFY_INLINE unsigned char* str_grouping_impl::get_groups
    ( unsigned num_digits
    , unsigned char* groups_array ) const
{
    if (_grouping.empty())
    {
        // this branch actually should never be executed
        BOOST_ASSERT(num_digits <= 0xFF);
        *groups_array = static_cast<unsigned char>(num_digits);
        return groups_array;
    }
    for(auto ch : _grouping)
    {
        auto group_size = static_cast<unsigned>(ch);
        if (group_size == 0)
        {
            *groups_array = static_cast<unsigned char>(num_digits);
            return groups_array;
        }
        if (group_size < num_digits)
        {
            *groups_array = static_cast<unsigned char>(group_size);
            num_digits -= group_size;
            ++ groups_array;
        }
        else
        {
            *groups_array = static_cast<unsigned char>(num_digits);
            return groups_array;
        }
    }
    const unsigned last_group_size = _grouping.back();
    while(num_digits > last_group_size)
    {
        *groups_array = static_cast<unsigned char>(last_group_size);
        ++ groups_array;
        num_digits -= last_group_size;
    }
    *groups_array = static_cast<unsigned char>(num_digits);
    return groups_array;
}


#endif //defined(BOOST_STRINGIFY_SOURCE) || ! defined(BOOST_STRINGIFY_SEPARATE_COMPILATION)

} // namespace detail


template <int Base> struct numpunct_c;

class numpunct_base
{
public:

    virtual ~numpunct_base()
    {
    }

    /**
    Caller must ensure that groups_array has at least num_digits elements
     */
    virtual unsigned groups
        ( unsigned num_digits
        , unsigned char* groups_array ) const = 0;

    /**
      return the number of thousands separators for such number of digits
     */
    virtual unsigned thousands_sep_count(unsigned num_digits) const = 0;

    char32_t thousands_sep() const
    {
        return _thousands_sep;
    }
    numpunct_base &  thousands_sep(char32_t ch) &
    {
        _thousands_sep = ch;
        return *this;
    }
    numpunct_base && thousands_sep(char32_t ch) &&
    {
        _thousands_sep = ch;
        return std::move(*this);
    }
    char32_t decimal_point() const
    {
        return _decimal_point;
    }
    numpunct_base &  decimal_point(char32_t ch) &
    {
        _decimal_point = ch;
        return *this;
    }
    numpunct_base && decimal_point(char32_t ch) &&
    {
        _decimal_point = ch;
        return std::move(*this);
    }
    // int char_width() const
    // {
    //     return _char_width; // todo
    // }

private:

    char32_t _thousands_sep = U',';
    char32_t _decimal_point = U'.';
};

template <int Base>
class numpunct: public stringify::v0::numpunct_base
{
public:

    using category = stringify::v0::numpunct_c<Base>;
};


template <int Base>
// made final to enable the implementation of has_i18n
class no_grouping final: public stringify::v0::numpunct<Base>
{
public:

    no_grouping()
    {
    }

    unsigned groups( unsigned num_digits
                   , unsigned char* groups_array ) const override
    {
        // this function actually should never be called
        // since thousands_sep_count(num_digits) aways returns 0.
        BOOST_ASSERT(num_digits <= 0xFF);
        *groups_array = static_cast<unsigned char>(num_digits);
        return 1;
    }
    unsigned thousands_sep_count(unsigned num_digits) const override
    {
        (void)num_digits;
        return 0;
    }
    no_grouping &  decimal_point(char32_t ch) &
    {
        numpunct_base::decimal_point(ch);
        return *this;
    }
    no_grouping && decimal_point(char32_t ch) &&
    {
        numpunct_base::decimal_point(ch);
        return std::move(*this);
    }
};


template <int Base>
class monotonic_grouping: public stringify::v0::numpunct<Base>
{
public:

    constexpr monotonic_grouping(unsigned char groups_size)
        : _impl(groups_size)
    {
    }

    constexpr monotonic_grouping(const monotonic_grouping&) = default;

    unsigned groups( unsigned num_digits
                   , unsigned char* groups_array ) const override
    {
        auto s = _impl.get_groups(num_digits, groups_array) - groups_array;
        return 1 + static_cast<unsigned>(s);
    }
    unsigned thousands_sep_count(unsigned num_digits) const override
    {
        return _impl.get_thousands_sep_count(num_digits);
    }
    monotonic_grouping &  thousands_sep(char32_t ch) &
    {
        numpunct_base::thousands_sep(ch);
        return *this;
    }
    monotonic_grouping && thousands_sep(char32_t ch) &&
    {
        numpunct_base::thousands_sep(ch);
        return std::move(*this);
    }
    monotonic_grouping &  decimal_point(char32_t ch) &
    {
        numpunct_base::decimal_point(ch);
        return *this;
    }
    monotonic_grouping && decimal_point(char32_t ch) &&
    {
        numpunct_base::decimal_point(ch);
        return std::move(*this);
    }

private:

    stringify::v0::detail::monotonic_grouping_impl _impl;
};


template <int Base>
class str_grouping: public stringify::v0::numpunct<Base>
{
public:

    str_grouping(std::string grouping)
        : _impl(grouping)
    {
    }

    str_grouping(const str_grouping&) = default;

    str_grouping(str_grouping&& other) = default;

    unsigned groups( unsigned num_digits
                   , unsigned char* groups_array ) const override
    {
        auto s = _impl.get_groups(num_digits, groups_array) - groups_array;
        return 1 + static_cast<unsigned>(s);
    }
    unsigned thousands_sep_count(unsigned num_digits) const override
    {
        return _impl.get_thousands_sep_count(num_digits);
    }
    str_grouping &  thousands_sep(char32_t ch) &
    {
        numpunct_base::thousands_sep(ch);
        return *this;
    }
    str_grouping && thousands_sep(char32_t ch) &&
    {
        numpunct_base::thousands_sep(ch);
        return std::move(*this);
    }
    str_grouping &  decimal_point(char32_t ch) &
    {
        numpunct_base::decimal_point(ch);
        return *this;
    }
    str_grouping && decimal_point(char32_t ch) &&
    {
        numpunct_base::decimal_point(ch);
        return std::move(*this);
    }

private:

    stringify::v0::detail::str_grouping_impl _impl;
};


template <int Base> struct numpunct_c
{
    constexpr static bool constrainable = true;

    constexpr static int base = Base;

    static const stringify::v0::no_grouping<base>& get_default()
    {
        static const stringify::v0::no_grouping<base> x{};
        return x;
    }
};

BOOST_STRINGIFY_V0_NAMESPACE_END

#endif  // BOOST_STRINGIFY_V0_DETAIL_FACETS_NUMPUNCT_HPP

