#ifndef BOOST_STRINGIFY_DETAIL_UTF16_TO_UTF8_HPP_INCLUDED
#define BOOST_STRINGIFY_DETAIL_UTF16_TO_UTF8_HPP_INCLUDED

#include <boost/stringify/input_char.hpp>

namespace boost {
namespace stringify {
namespace detail {

template<typename charT, typename Formating>
struct utf16_to_utf8: boost::stringify::input_base<char, Formating>
{
    const charT* str;

    utf16_to_utf8() noexcept
        : str(0)
    {
    }

    utf16_to_utf8(const charT* _str) noexcept :
                                      str(_str)
    {
    }

    void set(const charT* _str) noexcept
    {
        str = _str;
    }

    
    virtual std::size_t length(const Formating& fmt) const noexcept
    {
        std::size_t len = 0;
        const charT* it = str;
        while (*it != charT())
        {
            char32_t codepoint = read_codepoint(it);
            if (codepoint)
            {
                len += char32_to_utf8<Formating>(codepoint).length(fmt);
            }
        }
        return len;
    }

    
    virtual char* write_without_termination_char
        ( char* out
        , const Formating& fmt
        )
        const noexcept
    {
        const charT* it = str;
        while (*it != charT())
        {
            char32_t codepoint = read_codepoint(it);
            if (codepoint)
            {
                out = char32_to_utf8<Formating>(codepoint)
                    .write_without_termination_char(out);
            }
        }
        return out;
    }

    
    virtual void write
        ( boost::stringify::simple_ostream<char>& out
        , const Formating& fmt
        )
        const
    {
        const charT* it = str;
        while (*it != charT())
        {
            char32_t codepoint = read_codepoint(it);
            if (codepoint)
            {
                char32_to_utf8<Formating>(codepoint)
                    .write(out, fmt);
            }
        }
    }

    
private:
    typedef const charT * const_charT_ptr;

    static char32_t read_codepoint(const_charT_ptr& it) noexcept
    {
        uint32_t unit = *it++;
        if (unit >= 0xd800 && unit <= 0xbdff) {
            uint32_t unit2 = *it++;
            return (unit2 >= 0xDC00 && unit2 <= 0xDFFF
                    ? (unit << 10) + unit2 - 0x35fdc00
                    : 0);
        }
        return unit;
    }
};

}; //namespace boost
}; //namespace stringify
}; //namespace detail


#endif













