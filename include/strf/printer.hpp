#ifndef STRF_PRINTER_HPP
#define STRF_PRINTER_HPP

//  Distributed under the Boost Software License, Version 1.0.
//  (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

#include <strf/outbuf.hpp>
#include <strf/width_t.hpp>

namespace strf {

template <std::size_t CharSize>
class printer
{
public:

    constexpr static std::size_t char_size = CharSize;

    STRF_HD virtual ~printer()
    {
    }

    STRF_HD virtual void print_to(strf::underlying_outbuf<CharSize>& ob) const = 0;
};

struct string_input_tag_base
{
};

template <typename CharIn>
struct string_input_tag: string_input_tag_base
{
};

template <typename CharT>
struct is_string_of
{
    template <typename T>
    using fn = std::is_base_of<string_input_tag<CharT>, T>;
};

template <typename T>
using is_string = std::is_base_of<string_input_tag_base, T>;

template <typename CharIn>
struct tr_string_input_tag: strf::string_input_tag<CharIn>
{
};

template <typename CharIn>
struct range_separator_input_tag: strf::string_input_tag<CharIn>
{
};

template <typename CharIn>
struct is_tr_string_of
{
    template <typename T>
    using fn = std::is_same<strf::tr_string_input_tag<CharIn>, T>;
};

template <typename T>
struct is_tr_string: std::false_type
{
};

template <typename CharIn>
struct is_tr_string<strf::is_tr_string_of<CharIn>> : std::true_type
{
};

template <bool Active>
class width_preview;

template <>
class width_preview<true>
{
public:

    explicit constexpr STRF_HD width_preview(strf::width_t initial_width) noexcept
        : width_(initial_width)
    {}

    STRF_HD width_preview(const width_preview&) = delete;

    constexpr STRF_HD void subtract_width(strf::width_t w) noexcept
    {
        width_ -= w;
    }

    constexpr STRF_HD void checked_subtract_width(strf::width_t w) noexcept
    {
        if (w < width_) {
            width_ -= w;
        } else {
            width_ = 0;
        }
    }

    constexpr STRF_HD void checked_subtract_width(std::ptrdiff_t w) noexcept
    {
        if (w < width_.ceil()) {
            width_ -= static_cast<std::int16_t>(w);
        } else {
            width_ = 0;
        }
    }

    constexpr STRF_HD void clear_remaining_width() noexcept
    {
        width_ = 0;
    }

    constexpr STRF_HD strf::width_t remaining_width() const noexcept
    {
        return width_;
    }

private:

    strf::width_t width_;
};

template <>
class width_preview<false>
{
public:

    constexpr STRF_HD width_preview() noexcept
    {
    }

    constexpr STRF_HD void subtract_width(strf::width_t) noexcept
    {
    }

    constexpr STRF_HD void checked_subtract_width(strf::width_t) noexcept
    {
    }

    constexpr STRF_HD void checked_subtract_width(std::ptrdiff_t) noexcept
    {
    }

    constexpr STRF_HD void clear_remaining_width() noexcept
    {
    }

    constexpr STRF_HD strf::width_t remaining_width() const noexcept
    {
        return 0;
    }
};

template <bool Active>
class size_preview;

template <>
class size_preview<true>
{
public:
    explicit constexpr STRF_HD size_preview(std::size_t initial_size = 0) noexcept
        : size_(initial_size)
    {
    }

    STRF_HD size_preview(const size_preview&) = delete;

    constexpr STRF_HD void add_size(std::size_t s) noexcept
    {
        size_ += s;
    }

    constexpr STRF_HD std::size_t get_size() const noexcept
    {
        return size_;
    }

private:

    std::size_t size_;
};

template <>
class size_preview<false>
{
public:

    constexpr STRF_HD size_preview() noexcept
    {
    }

    constexpr STRF_HD void add_size(std::size_t) noexcept
    {
    }

    constexpr STRF_HD std::size_t get_size() const noexcept
    {
        return 0;
    }
};

template <bool SizeRequired, bool WidthRequired>
class print_preview
    : public strf::size_preview<SizeRequired>
    , public strf::width_preview<WidthRequired>
{
public:

    static constexpr bool size_required = SizeRequired;
    static constexpr bool width_required = WidthRequired;
    static constexpr bool nothing_required = ! SizeRequired && ! WidthRequired;

    template <bool W = WidthRequired>
    STRF_HD constexpr explicit print_preview
        ( std::enable_if_t<W, strf::width_t> initial_width ) noexcept
        : strf::width_preview<WidthRequired>{initial_width}
    {
    }

    constexpr STRF_HD print_preview() noexcept
    {
    }
};

namespace detail {

#if defined(__cpp_fold_expressions)

template <std::size_t CharSize, typename ... Printers>
inline STRF_HD void write_args( strf::underlying_outbuf<CharSize>& ob
                              , const Printers& ... printers )
{
    (... , printers.print_to(ob));
}

#else // defined(__cpp_fold_expressions)

template <std::size_t CharSize>
inline STRF_HD void write_args(strf::underlying_outbuf<CharSize>&)
{
}

template <std::size_t CharSize, typename Printer, typename ... Printers>
inline STRF_HD void write_args
    ( strf::underlying_outbuf<CharSize>& ob
    , const Printer& printer
    , const Printers& ... printers )
{
    printer.print_to(ob);
    if (ob.good()) {
        write_args(ob, printers ...);
    }
}

#endif // defined(__cpp_fold_expressions)

} // namespace detail

} // namespace strf

#endif // STRF_PRINTER_HPP
