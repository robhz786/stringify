//  Distributed under the Boost Software License, Version 1.0.
//  (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

#define  _CRT_SECURE_NO_WARNINGS

#include <boost/detail/lightweight_test.hpp>
#include "test_utils.hpp"
#include "error_code_emitter_arg.hpp"
#include <boost/stringify.hpp>

namespace strf = boost::stringify::v0;

class reservation_tester : public strf::output_writer<char>
{
public:

    reservation_tester
        ( strf::output_writer_init<char> init
        , std::size_t& reserve_size
        )
        : strf::output_writer<char>{init}
        , m_reserve_size(reserve_size)
    {
    }
    
    virtual void set_error(std::error_code) override
    {
    }

    virtual bool good() const override
    {
        return true;
    }

    virtual bool put(strf::piecemeal_input<char>&) override
    {
        return true;
    }

    virtual bool put(const char*, std::size_t) override
    {
        return true;
    }

    virtual bool put(char) override
    {
        return true;
    }

    virtual bool put(std::size_t, char) override
    {
        return true;
    }

    void reserve(std::size_t s)
    {
        m_reserve_size = s;
    }

    std::error_code finish()
    {
        return {};
    }

    void finish_exception()
    {
    }
    
private:

    std::size_t & m_reserve_size;
};


auto reservation_test(std::size_t & s)
{
    return strf::make_destination<reservation_tester, std::size_t&>(s);
}


int main()
{
    constexpr std::size_t initial_value = std::numeric_limits<std::size_t>::max();

    // on non-const rval ref
   
    {
        std::size_t size{initial_value};
        (void)reservation_test(size) .no_reserve() ("abcd");
        BOOST_TEST(size == initial_value); 
    }
    {
        std::size_t size{initial_value};
        (void)reservation_test(size) .reserve(5555) ("abcd");
        BOOST_TEST(size == 5555); 
    }
    {
        std::size_t size{initial_value};
        (void)reservation_test(size) ("abcd");
        BOOST_TEST(size == 4); 
    }
    {
        std::size_t size{initial_value};
        (void)reservation_test(size) .reserve(5555) .reserve_auto() ("abcd");
        BOOST_TEST(size == 4); 
    }

    // on non-const ref
    
    {
        std::size_t size{initial_value};
        auto tester = reservation_test(size);
        (void)tester.no_reserve() ("abcd");
        BOOST_TEST(size == initial_value); 
    }
    {
        std::size_t size{initial_value};
        auto tester = reservation_test(size);
        (void)tester.reserve(5555) ("abcd");
        BOOST_TEST(size == 5555); 
    }
    {
        std::size_t size{initial_value};
        auto tester = reservation_test(size);
        (void)tester ("abcd");
        BOOST_TEST(size == 4); 
    }
    {
        std::size_t size{initial_value};
        auto tester = reservation_test(size) .reserve(5555);
        (void)tester.reserve_auto() ("abcd");
        BOOST_TEST(size == 4); 
    }

    // on const ref

    {
        std::size_t size{initial_value};
        const auto tester = reservation_test(size);
        (void)tester.no_reserve() ("abcd");
        BOOST_TEST(size == initial_value); 
    }
    {
        std::size_t size{initial_value};
        const auto tester = reservation_test(size);
        (void)tester.reserve(5555) ("abcd");
        BOOST_TEST(size == 5555); 
    }
    {
        std::size_t size{initial_value};
        const auto tester = reservation_test(size) .reserve(5555);
        (void)tester.reserve_auto() ("abcd");
        BOOST_TEST(size == 4); 
    }

    // on const rval ref
    
    {
        std::size_t size{initial_value};
        const auto tester = reservation_test(size);
        (void)std::move(tester).no_reserve() ("abcd");
        BOOST_TEST(size == initial_value); 
    }
    {
        std::size_t size{initial_value};
        const auto tester = reservation_test(size);
        (void)std::move(tester).reserve(5555) ("abcd");
        BOOST_TEST(size == 5555); 
    }
    {
        std::size_t size{initial_value};
        const auto tester = reservation_test(size) .reserve(5555);
        (void)std::move(tester).reserve_auto() ("abcd");
        BOOST_TEST(size == 4); 
    }


    
    return report_errors() || boost::report_errors();
}