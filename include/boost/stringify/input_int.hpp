#ifndef BOOST_STRINGIFY_INPUT_INT_HPP_INCLUDED
#define BOOST_STRINGIFY_INPUT_INT_HPP_INCLUDED

#include <boost/stringify/input_base.hpp>
#include <boost/stringify/formater_tuple.hpp>
#include <boost/stringify/fmt_showpos.hpp>
#include <boost/stringify/detail/characters_catalog.hpp>
#include <boost/stringify/detail/char_flags.hpp>
#include <boost/stringify/detail/uint_traits.hpp>
#include <boost/stringify/detail/int_digits.hpp>

#include <boost/logic/tribool.hpp>

namespace boost
{
namespace stringify
{
namespace detail
{

class local_formatting_int
{

public:

    constexpr local_formatting_int(const char* charflags, double d = 0.0) : m_charflags(charflags)
    {
    }

    constexpr local_formatting_int()
    {
    }

    constexpr local_formatting_int(const local_formatting_int&) = default;
    
    local_formatting_int& operator=(const local_formatting_int&) = default;
    
    boost::tribool showpos() const
    {
        if (m_charflags.has_char('-'))
            return false;

        if (m_charflags.has_char('+'))
            return true;
     
        return boost::logic::indeterminate;
    }

private:

    typedef boost::stringify::detail::char_flags<'+', '-'> char_flags_type;
    char_flags_type m_charflags;    
};

} // namespace detail

template <typename intT, typename charT, typename Output, typename Formating>
struct input_int: public boost::stringify::input_base<charT, Output, Formating>
{
    typedef boost::stringify::input_base<charT, Output, Formating> base;
    typedef typename std::make_unsigned<intT>::type unsigned_intT;
    typedef boost::stringify::detail::uint_traits<unsigned_intT> uint_traits;
    using base::noexcept_output;
    using base::random_access_output;
public:

    input_int() noexcept
        : m_value(0)
        , m_abs_value(0)
    {
    }

    input_int(intT _value) noexcept
    {
        set(_value);
    }

    void set(intT _value) noexcept
    {
        m_value = (_value);
        m_abs_value = (m_value > 0
                     ? static_cast<unsigned_intT>(m_value)
                     : 1 + static_cast<unsigned_intT>(-(m_value + 1)));
    }

    typedef boost::stringify::detail::local_formatting_int local_formatting;

    
    void set
        ( intT _value
        , local_formatting fmt
        ) noexcept
    {
        set(_value);
        m_local_fmt = fmt;
    }

    virtual std::size_t length(const Formating& fmt) const noexcept
    {
        return length_digits() + (has_sign(fmt) ? 1 : 0);
    }

    virtual void write
        ( Output& out
        , const Formating& fmt
        ) const noexcept(noexcept_output) override
    {
        write_sign(out, fmt);
        write_digits(out, fmt);
    }

private:

    virtual std::size_t length_digits() const noexcept
    {
        return uint_traits::number_of_digits(m_abs_value);
    }
    

    intT m_value;
    unsigned_intT m_abs_value; // TODO optimaze ( use a union when intT is usigned )
    local_formatting m_local_fmt;
    
    bool has_sign(const Formating& fmt) const noexcept
    {
        /*constexpr*/ if( std::is_signed<intT>::value)
        {
            return m_value < 0 || showpos(fmt);
        }
        return false;
    }

    void write_sign(Output& out, const Formating& fmt) const noexcept(noexcept_output)
    {
        /*constexpr*/ if( std::is_signed<intT>::value)
        {
            if (m_value < 0)
            {
                out.put(boost::stringify::detail::the_sign_minus<charT>());
            }
            else if(showpos(fmt))
            {
                out.put(boost::stringify::detail::the_sign_plus<charT>());
            }
        }
    }
    
    void write_digits(Output& out, const Formating& fmt) const noexcept(noexcept_output)
    {
        if (out.set_pos(out.get_pos() + length_digits()))
        {
            auto pos_bk = out.get_pos();
            unsigned_intT it_value = m_abs_value;
            while(it_value >= 10)
            {
                out.rput(character_of_digit(it_value % 10));
                it_value /= 10;
            }
            out.rput(character_of_digit(it_value));
            out.set_pos(pos_bk);
        }
        else
        {
            boost::stringify::detail::int_digits<unsigned_intT, 10> digits(m_abs_value);
            while(! digits.empty())
            {
                out.put(character_of_digit(digits.pop()));
            }
        }
    }

    bool showpos(const Formating& fmt) const noexcept
    {
        boost::tribool local_showpos = m_local_fmt.showpos();
        if(indeterminate(local_showpos))
        {
            return fmt.template get<boost::stringify::ftype_showpos, intT>().show();
        }
        return local_showpos;
    }
        
    charT character_of_digit(unsigned int digit) const noexcept
    {
        if (digit < 10)
        {
            return boost::stringify::detail::the_digit_zero<charT>() + digit;
        }
        return boost::stringify::detail::the_character_a<charT>() + digit - 10;
    }
};


template <typename charT, typename Output, typename Formating>
inline                                    
boost::stringify::input_int<int, charT, Output, Formating>
argf(int i) noexcept
{                                               
    return i;                                     
}

template <typename charT, typename Output, typename Formating>
inline                                    
boost::stringify::input_int<int, charT, Output, Formating>
argf(int i, const char*) noexcept
{                                               
    return i;                                     
}


template <typename charT, typename Output, typename Formating>
inline
boost::stringify::input_int<long, charT, Output, Formating>
argf(long i) noexcept
{
    return i;
}

template <typename charT, typename Output, typename Formating>
inline
boost::stringify::input_int<long long, charT, Output, Formating>
argf(long long i) noexcept
{
    return i;
}

template <typename charT, typename Output, typename Formating>
inline
boost::stringify::input_int<unsigned int, charT, Output, Formating>
argf(unsigned int i) noexcept
{
    return i;
}

template <typename charT, typename Output, typename Formating>
inline
boost::stringify::input_int<unsigned long, charT, Output, Formating>
argf(unsigned long i) noexcept
{
    return i;
}

template <typename charT, typename Output, typename Formating>
inline
boost::stringify::input_int<unsigned long long, charT, Output, Formating>
argf(unsigned long long i) noexcept
{
    return i;
}


}//namespace stringify
}//namespace boost


#endif
