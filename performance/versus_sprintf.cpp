#include <iostream>
#include <stdio.h>
#include <climits>
#include <boost/timer/timer.hpp>
#include <boost/stringify.hpp>
#include "loop_timer.hpp"

#define PRINT_BENCHMARK(label)  \
  BOOST_LOOP_TIMER(500, boost::timer::print_mean_time(label))

int main()
{
    namespace strf = boost::stringify;  
    char buff[1000000];
    char* char_ptr_output = buff;

    std::cout << std::endl 
              << "Copy a string literal:" 
              << std::endl;

    PRINT_BENCHMARK("lengthf<char>({}, \"hello\")")
    {
        strf::lengthf<char>({}, "hello");
    }
    PRINT_BENCHMARK("lengthf_il<char>({}, {\"hello\")}")
    {
        strf::lengthf_il<char>({}, {"hello"});
    }
    PRINT_BENCHMARK("writef(char_ptr_output, {}, \"hello\")")
    {
        strf::writef(char_ptr_output, {}, "hello");
    }
    PRINT_BENCHMARK("writef_il(char_ptr_output, {}, {\"hello\"})")
    {
        strf::writef_il(char_ptr_output, {}, {"hello"});
    }
    PRINT_BENCHMARK("sprintf(char_ptr_output, \"hello\")")
    {
        sprintf(char_ptr_output, "hello");
    }
    PRINT_BENCHMARK("sprintf(char_ptr_output, \"%s\", \"hello\")")
    {
        sprintf(char_ptr_output, "%s", "hello");
    }
    PRINT_BENCHMARK("strcpy(char_ptr_output, \"hello\")")
    {
        strcpy(char_ptr_output, "hello");
    }

    std::cout << std::endl
              << "Copy a heap allocated short string:"
              << std::endl;
    {
        std::string std_string_hello("hello");
        const char* hello = std_string_hello.c_str();

        std::string std_string_fmt("%s");
        const char* fmt = std_string_fmt.c_str();


        PRINT_BENCHMARK("lengthf<char>({}, hello)")
        {
            strf::lengthf<char>({}, hello);
        }
        PRINT_BENCHMARK("lengthf_il<char>({}, {hello})")
        {
            strf::lengthf_il<char>({}, {hello});
        }
        PRINT_BENCHMARK("writef(char_ptr_output, {}, hello)")
        {
            strf::writef(char_ptr_output, {}, hello);
        }
        PRINT_BENCHMARK("writef_il(char_ptr_output, {}, {hello})")
        {
            strf::writef_il(char_ptr_output, {}, {hello});
        }
        PRINT_BENCHMARK("strcpy(char_ptr_output, hello)")
        {
            strcpy(char_ptr_output, hello);
        }
        PRINT_BENCHMARK("sprintf(char_ptr_output, hello)")
        {
            sprintf(char_ptr_output, hello);
        }
        PRINT_BENCHMARK("sprintf(char_ptr_output, \"%s\", hello)")
        {
            sprintf(char_ptr_output, "%s", hello);
        }
        PRINT_BENCHMARK("sprintf(char_ptr_output, fmt, hello)")
        {
            sprintf(char_ptr_output, fmt, hello);
        }
    }

    std::cout << std::endl 
              << "Copy two strings" 
              << std::endl;

    PRINT_BENCHMARK("lengthf<char>({}, \"hello\", \"hello\")")
    {
        strf::lengthf<char>({}, "hello", "hello");
    }
    PRINT_BENCHMARK("lengthf_il<char>({}, {\"hello\", \"hello\")}")
    {
        strf::lengthf_il<char>({}, {"hello", "hello"});
    }
    PRINT_BENCHMARK("writef(char_ptr_output, {}, \"hello\", \"hello\")")
    {
        strf::writef(char_ptr_output, {}, "hello", "hello");
    }
    PRINT_BENCHMARK("writef_il(char_ptr_output, {}, {\"hello\", \"hello\"})")
    {
        strf::writef_il(char_ptr_output, {}, {"hello", "hello"});
    }
    PRINT_BENCHMARK("sprintf(char_ptr_output, \"%s%s\", \"hello\", \"hello\")")
    {
        sprintf(char_ptr_output, "%s%s", "hello", "hello");
    }

    std::cout << std::endl
              << "Copy a long string ( 1000 characters ):"
              << std::endl;
    {
        std::string std_string_long_string(1000, 'x');
        const char* long_string = std_string_long_string.c_str();

        std::string std_string_fmt("%s");
        const char* fmt = std_string_fmt.c_str();

        PRINT_BENCHMARK("lengthf<char>({}, long_string)")
        {
            strf::lengthf<char>({}, long_string);
        }
        PRINT_BENCHMARK("lengthf_il<char>({}, {long_string})")
        {
            strf::lengthf_il<char>({}, {long_string});
        }
        PRINT_BENCHMARK("writef(char_ptr_output, {}, long_string)")
        {
            strf::writef(char_ptr_output, {}, long_string);
        }
        PRINT_BENCHMARK("writef_il(char_ptr_output, {}, {long_string})")
        {
            strf::writef_il(char_ptr_output, {}, {long_string});
        }
        PRINT_BENCHMARK("strcpy(char_ptr_output, long_string)")
        {
            strcpy(char_ptr_output, long_string);
        }
        PRINT_BENCHMARK("sprintf(char_ptr_output, long_string)")
        {
            sprintf(char_ptr_output, long_string);
        }
        PRINT_BENCHMARK("sprintf(char_ptr_output, \"%s\", long_string)")
        {
            sprintf(char_ptr_output, "%s", long_string);
        }
        PRINT_BENCHMARK("sprintf(char_ptr_output, fmt, long_string)")
        {
            sprintf(char_ptr_output, fmt, long_string);
        }
    }

    std::cout << std::endl
              << "write integers" 
              << std::endl;

    PRINT_BENCHMARK("lengthf<char>({}, 25)")
    {
        strf::lengthf<char>({}, 25);
    }
    PRINT_BENCHMARK("lengthf_il<char>({}, {25})")
    {
        strf::lengthf_il<char>({}, {25});
    }
    PRINT_BENCHMARK("writef(char_ptr_output, {}, 25)")
    {
        strf::writef(char_ptr_output, {}, 25);
    }
    PRINT_BENCHMARK("writef_il(char_ptr_output, {}, {25})")
    {
        strf::writef_il(char_ptr_output, {}, {25});
    }
    PRINT_BENCHMARK("sprintf(char_ptr_output, \"%d\", 25)")
    {
        sprintf(char_ptr_output, "%d", 12345);
    }
    std::cout << std::endl;
    PRINT_BENCHMARK("lengthf<char>({}, INT_MAX)")
    {
        strf::lengthf<char>({}, INT_MAX);
    }
    PRINT_BENCHMARK("lengthf_il<char>({}, {INT_MAX})")
    {
        strf::lengthf_il<char>({}, {INT_MAX});
    }
    PRINT_BENCHMARK("writef(char_ptr_output, {}, INT_MAX)")
    {
        strf::writef(char_ptr_output, {}, INT_MAX);
    }
    PRINT_BENCHMARK("writef_il(char_ptr_output, {}, {INT_MAX})")
    {
        strf::writef_il(char_ptr_output, {}, {INT_MAX});
    }
    PRINT_BENCHMARK("sprintf(char_ptr_output, \"%d\", INT_MAX)")
    {
        sprintf(char_ptr_output, "%d", INT_MAX);
    }
    std::cout << std::endl;
    PRINT_BENCHMARK("lengthf<char>({}, LLONG_MAX)")
    {
        strf::lengthf<char>({}, LLONG_MAX);
    }
    PRINT_BENCHMARK("lengthf_il<char>({}, {LLONG_MAX})")
    {
        strf::lengthf_il<char>({}, {LLONG_MAX});
    }
    PRINT_BENCHMARK("writef(char_ptr_output, {}, LLONG_MAX)")
    {
        strf::writef(char_ptr_output, {}, LLONG_MAX);
    }
    PRINT_BENCHMARK("writef_il(char_ptr_output, {}, {LLONG_MAX})")
    {
        strf::writef_il(char_ptr_output, {}, {LLONG_MAX});
    }
    PRINT_BENCHMARK("sprintf(char_ptr_output, \"%lld\", LLONG_MAX)")
    {
        sprintf(char_ptr_output, "%lld", LLONG_MAX);
    }
    std::cout << std::endl;
    PRINT_BENCHMARK("lengthf<char>({}, 25, 25, 25)")
    {
        strf::lengthf<char>({}, 25, 25, 25);
    }
    PRINT_BENCHMARK("lengthf_il<char>({}, {25, 25, 25})")
    {
        strf::lengthf_il<char>({}, {25, 25, 25});
    }
    PRINT_BENCHMARK("writef(char_ptr_output, {}, 25, 25, 25)")
    {
        strf::writef(char_ptr_output, {}, 25, 25, 25);
    }
    PRINT_BENCHMARK("writef_il(char_ptr_output, {}, {25, 25, 25})")
    {
        strf::writef_il(char_ptr_output, {}, {25, 25, 25});
    }
    PRINT_BENCHMARK("sprintf(char_ptr_output, \"%d%d%d\", 25, 25, 25)")
    {
        sprintf(char_ptr_output, "%d%d%d", 25, 25, 25);
    }
    std::cout << std::endl;
    PRINT_BENCHMARK("lengthf<char>({}, LLONG_MAX, LLONG_MAX, LLONG_MAX)")
    {
        strf::lengthf<char>({}, LLONG_MAX, LLONG_MAX, LLONG_MAX);
    }
    PRINT_BENCHMARK("lengthf_il<char>({}, {LLONG_MAX, LLONG_MAX, LLONG_MAX})")
    {
        strf::lengthf_il<char>({}, {LLONG_MAX, LLONG_MAX, LLONG_MAX});
    }
     PRINT_BENCHMARK("writef(char_ptr_output, {}, LLONG_MAX, LLONG_MAX, LLONG_MAX)")
    {
        strf::writef(char_ptr_output, {}, LLONG_MAX, LLONG_MAX, LLONG_MAX);
    }
    PRINT_BENCHMARK("writef_il(char_ptr_output, {}, {LLONG_MAX, LLONG_MAX, LLONG_MAX})")
    {
        strf::writef_il(char_ptr_output, {}, {LLONG_MAX, LLONG_MAX, LLONG_MAX});
    }
    PRINT_BENCHMARK("sprintf(char_ptr_output, \"%d%d%d\", LLONG_MAX, LLONG_MAX, LLONG_MAX)")
    {
        sprintf(char_ptr_output, "%lld%lld%lld", LLONG_MAX, LLONG_MAX, LLONG_MAX);
    }
    std::cout << std::endl;
    PRINT_BENCHMARK("lengthf<char>({} \"ten =  \", 10, \", twenty = \", 20)")
    {
        strf::lengthf<char>({}, "ten =  ", 10, ", twenty = ", 20);
    }
    PRINT_BENCHMARK("lengthf_il<char>({}, {\"ten =  \", 10, \", twenty = \", 20})")
    {
        strf::lengthf_il<char>({}, {"ten =  ", 10, ", twenty = ", 20});
    }
    PRINT_BENCHMARK("writef(char_ptr_output, {} \"ten =  \", 10, \", twenty = \", 20)")
    {
        strf::writef(char_ptr_output, {}, "ten =  ", 10, ", twenty = ", 20);
    }
    PRINT_BENCHMARK("writef_il(char_ptr_output, {}, {\"ten =  \", 10, \", twenty = \", 20})")
    {
        strf::writef_il(char_ptr_output, {}, {"ten =  ", 10, ", twenty = ", 20});
    }
    PRINT_BENCHMARK("sprintf(char_ptr_output, \"ten = %d, twenty= %d\", 10, 20)")
    {
        sprintf(char_ptr_output, "ten = %d, twenty= %d", 10, 20);
    }

    return 1;
}
