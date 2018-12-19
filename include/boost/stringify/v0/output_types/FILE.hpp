#ifndef BOOST_STRINGIFY_V0_OUTPUT_TYPES_FILE_HPP
#define BOOST_STRINGIFY_V0_OUTPUT_TYPES_FILE_HPP

//  Distributed under the Boost Software License, Version 1.0.
//  (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

#include <cstdio>
#include <cstring>
#include <boost/stringify/v0/basic_types.hpp>

BOOST_STRINGIFY_V0_NAMESPACE_BEGIN

namespace detail {

template <typename CharT>
class narrow_file_writer final: public stringify::v0::buffer_recycler<CharT>
{
public:
    constexpr static std::size_t _buff_size = stringify::v0::min_buff_size;

private:
    CharT _buff[_buff_size];

public:

    using char_type = CharT;

    narrow_file_writer(std::FILE* file, std::size_t* count)
        : _file(file)
        , _count(count)
    {
        if (_count != nullptr)
        {
            *_count = 0;
        }
    }

    ~narrow_file_writer()
    {
    }

    stringify::v0::expected_output_buffer<CharT> start() noexcept
    {
        return { stringify::v0::in_place_t{}
               , stringify::v0::output_buffer<CharT>
                   { _buff, _buff + _buff_size } };
    }

    stringify::v0::expected_output_buffer<CharT> recycle(CharT* it) override;

    stringify::v0::expected<void, std::error_code> finish(CharT* it);

protected:

    bool do_put(const CharT* it);

    std::FILE* _file;
    std::size_t* _count = nullptr;
};


template <typename CharT>
stringify::v0::expected_output_buffer<CharT>
narrow_file_writer<CharT>::recycle(CharT* it)
{
    if (!do_put(it))
    {
        return { stringify::v0::unexpect_t{}
               , std::error_code{errno, std::generic_category() } };
    }
    return start();
}


template <typename CharT>
stringify::v0::expected<void, std::error_code>
narrow_file_writer<CharT>::finish(CharT* it)
{
    if (!do_put(it))
    {
        return { stringify::v0::unexpect_t{}
               , std::error_code{errno, std::generic_category() } };
    }
    return {};
}

template <typename CharT>
bool narrow_file_writer<CharT>::do_put(const CharT* it)
{
    BOOST_ASSERT(_buff <= it && it <= _buff + _buff_size);
    std::size_t count = it - _buff;
    auto count_inc = std::fwrite(_buff, sizeof(CharT), count, _file);

    if (_count != nullptr)
    {
        *_count += count_inc;
    }
    return count == count_inc;
}

#if defined(BOOST_STRINGIFY_NOT_HEADER_ONLY)

BOOST_STRINGIFY_EXPLICIT_TEMPLATE class narrow_file_writer<char>;
BOOST_STRINGIFY_EXPLICIT_TEMPLATE class narrow_file_writer<char16_t>;
BOOST_STRINGIFY_EXPLICIT_TEMPLATE class narrow_file_writer<char32_t>;
BOOST_STRINGIFY_EXPLICIT_TEMPLATE class narrow_file_writer<wchar_t>;

#endif

class wide_file_writer final: public stringify::v0::buffer_recycler<wchar_t>
{
    constexpr static std::size_t _buff_size = stringify::v0::min_buff_size;
    wchar_t _buff[_buff_size];

public:

    using char_type = wchar_t;

    wide_file_writer(std::FILE* file, std::size_t* count)
        : _file(file)
        , _count(count)
    {
        if (_count != nullptr)
        {
            *_count = 0;
        }
    }

    stringify::v0::expected_output_buffer<char_type> start() noexcept
    {
        return { stringify::v0::in_place_t{}
               , stringify::v0::output_buffer<char_type>
                   { _buff, _buff + _buff_size } };
    }

    stringify::v0::expected_output_buffer<char_type> recycle(char_type* it) override
    {
        if (do_put(it))
        {
            return start();
        }
        return { stringify::v0::unexpect_t{}
               , std::error_code{errno, std::generic_category() } };

    }

    stringify::v0::expected<void, std::error_code> finish(char_type* it)
    {
        if (do_put(it))
        {
            return {};
        }
        return { stringify::v0::unexpect_t{}
               , std::error_code{errno, std::generic_category() } };
    }

private:

    bool do_put(const wchar_t* it);

    std::FILE* _file;
    std::size_t* _count = nullptr;
};

#if ! defined(BOOST_STRINGIFY_OMIT_IMPL)

BOOST_STRINGIFY_INLINE bool wide_file_writer::do_put(const wchar_t* end)
{
    BOOST_ASSERT(_buff <= end && end <= _buff + _buff_size);

    std::size_t count = 0;
    bool good = true;
    for(auto it = _buff ; it != end; ++it, ++count)
    {
        auto ret = std::fputwc(*it, _file);
        if(ret == WEOF)
        {
            good = false;
            break;
        }
    }
    if (_count != nullptr)
    {
        *_count += count;
    }
    return good;
}

#endif //! defined(BOOST_STRINGIFY_OMIT_IMPL)

} // namespace detail

template <typename CharT = char>
inline auto write(std::FILE* destination, std::size_t* count = nullptr)
{
    using writer = stringify::v0::detail::narrow_file_writer<CharT>;
    return stringify::v0::make_destination<writer>(destination, count);
}

inline auto wwrite(std::FILE* destination, std::size_t* count = nullptr)
{
    using writer = boost::stringify::v0::detail::wide_file_writer;
    return stringify::v0::make_destination<writer>(destination, count);
}

BOOST_STRINGIFY_V0_NAMESPACE_END

#endif  // BOOST_STRINGIFY_V0_OUTPUT_TYPES_FILE_HPP

