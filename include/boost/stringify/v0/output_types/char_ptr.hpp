#ifndef BOOST_STRINGIFY_V0_OUTPUT_TYPES_CHAR_PTR_HPP
#define BOOST_STRINGIFY_V0_OUTPUT_TYPES_CHAR_PTR_HPP

//  Distributed under the Boost Software License, Version 1.0.
//  (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

#include <string>
#include <boost/stringify/v0/args_handler.hpp>

namespace boost{
namespace stringify{
inline namespace v0 {
namespace detail{

template<typename CharT, typename Traits>
class char_ptr_writer
{
public:

    typedef CharT char_type;

    char_ptr_writer(const char_ptr_writer&) = default;
    
    explicit char_ptr_writer(CharT* out)
        : m_out(out)
    {
    }

    void put(CharT character) noexcept
    {
        Traits::assign(*m_out++, character);
    }

    void put(CharT character, std::size_t repetitions) noexcept
    {
        Traits::assign(m_out, repetitions, character);
        m_out += repetitions;
    }

    void put(const CharT* str, std::size_t count) noexcept
    {
        Traits::copy(m_out, str, count);
        m_out += count;
    }
    
    CharT* finish() noexcept
    {
        Traits::assign(*m_out, CharT());
        return m_out;
    }

private:

    CharT* m_out;
};

} // namespace detail


template<typename CharT, typename CharTraits = std::char_traits<CharT> >
auto write_to(CharT* destination)
{
    using writer = boost::stringify::v0::detail::char_ptr_writer<CharT, CharTraits>;
    return boost::stringify::v0::make_args_handler<writer, CharT*>(destination);
}

} // inline namespace v0
} // namespace stringify
} // namespace boost    

#endif  /* BOOST_STRINGIFY_V0_OUTPUT_TYPES_CHAR_PTR_HPP */

