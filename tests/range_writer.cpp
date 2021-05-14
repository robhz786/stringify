//  Distributed under the Boost Software License, Version 1.0.
//  (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

#include "test_utils.hpp"

#if defined(__GNUC__) && (__GNUC__ == 7 || __GNUC__ == 8)
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Warray-bounds"
#endif

template <typename CharT>
void STRF_TEST_FUNC char_range_basic_operations()
{
    constexpr std::size_t buff_size = 8;
    CharT buff[buff_size];
    {   // construct from array
        strf::basic_char_array_writer<CharT> sw(buff);
        TEST_TRUE(sw.pointer() == &buff[0]);
        TEST_EQ(sw.space(), buff_size);
        TEST_TRUE(sw.good());
    }
    {   // construct from pointer and size
        strf::basic_char_array_writer<CharT> sw(buff, 4);
        TEST_TRUE(sw.pointer() == &buff[0]);
        TEST_EQ(sw.space(), 4);
        TEST_TRUE(sw.good());
    }
    {   // construct from range
        strf::basic_char_array_writer<CharT> sw(buff, buff + 4);
        TEST_TRUE(sw.pointer() == &buff[0]);
        TEST_EQ(sw.space(), 4);
        TEST_TRUE(sw.good());
    }
    {   // Calling recycle always fails
        strf::basic_char_array_writer<CharT> sw(buff);
        sw.recycle();
        TEST_TRUE(sw.pointer() != &buff[0]);
        TEST_TRUE(sw.space() >= strf::min_space_after_recycle<CharT>())
        TEST_FALSE(sw.good());

        // and causes pointer() to point to somewhere else than
        // anywhere inside the initial range
        TEST_FALSE(&buff[0] <= sw.pointer() && sw.pointer() < buff + buff_size);
    }
    {   // When calling finish() after recycle(),
        // the returned pointer is equal to the value pointer()
        // would have returned just before recycle()

        strf::basic_char_array_writer<CharT> sw(buff);
        strf::put(sw, (CharT)'a');
        strf::put(sw, (CharT)'b');
        strf::put(sw, (CharT)'c');
        sw.recycle();
        auto r = sw.finish();
        TEST_TRUE(r.truncated);
        TEST_TRUE(r.ptr == buff + 3);
    }
    {   // Copy constructor
        strf::basic_char_array_writer<CharT> sw1(buff);
        auto sw2{sw1};
        TEST_TRUE(sw1 == sw2);
        TEST_TRUE(sw1.pointer() == sw2.pointer());
        TEST_TRUE(sw1.end() == sw2.end());
        TEST_EQ(sw1.good(), sw2.good());
        auto r1 = sw1.finish();
        auto r2 = sw2.finish();
        TEST_TRUE(r1.ptr == r2.ptr);
        TEST_EQ(r1.truncated, r2.truncated);
    }
    {   // Copy constructor
        // copy "bad" object

        strf::basic_char_array_writer<CharT> sw1(buff);
        strf::put(sw1, (CharT)'a');
        strf::put(sw1, (CharT)'b');
        strf::put(sw1, (CharT)'c');

        sw1.recycle();

        auto sw2{sw1};
        TEST_TRUE(sw1 == sw2);
        TEST_TRUE(sw1.pointer() == sw2.pointer());
        TEST_TRUE(sw1.end() == sw2.end());
        TEST_EQ(sw1.good(), sw2.good());
        auto r1 = sw1.finish();
        auto r2 = sw2.finish();
        TEST_TRUE(r1.ptr == r2.ptr);
        TEST_EQ(r1.truncated, r2.truncated);
    }
    {   // Copy assignment

        CharT buff2[10];
        strf::basic_char_array_writer<CharT> sw1(buff);
        strf::basic_char_array_writer<CharT> sw2(buff2);

        TEST_FALSE(sw1 == sw2);

        strf::put(sw1, (CharT)'a');
        strf::put(sw1, (CharT)'b');
        strf::put(sw1, (CharT)'c');

        strf::put(sw2, (CharT)'a');
        strf::put(sw2, (CharT)'b');
        strf::put(sw2, (CharT)'c');

        sw1 = sw2;
        TEST_TRUE(sw1 == sw2);
        TEST_TRUE(sw1.pointer() == sw2.pointer());
        TEST_TRUE(sw1.end() == sw2.end());
        TEST_EQ(sw1.good(), sw2.good());
        auto r1 = sw1.finish();
        auto r2 = sw2.finish();
        TEST_TRUE(r1.ptr == r2.ptr);
        TEST_EQ(r1.truncated, r2.truncated);
    }
    {   // Copy assignment
        // copy a "bad" object

        CharT buff2[10];
        strf::basic_char_array_writer<CharT> sw1(buff);
        strf::basic_char_array_writer<CharT> sw2(buff2);

        TEST_FALSE(sw1 == sw2);

        strf::put(sw1, (CharT)'a');
        strf::put(sw1, (CharT)'b');
        strf::put(sw1, (CharT)'c');

        strf::put(sw2, (CharT)'a');
        strf::put(sw2, (CharT)'b');
        strf::put(sw2, (CharT)'c');
        sw2.recycle();

        sw1 = sw2;
        TEST_TRUE(sw1 == sw2);
        TEST_TRUE(sw1.pointer() == sw2.pointer());
        TEST_TRUE(sw1.end() == sw2.end());
        TEST_EQ(sw1.good(), sw2.good());
        auto r1 = sw1.finish();
        auto r2 = sw2.finish();
        TEST_TRUE(r1.ptr == r2.ptr);
        TEST_EQ(r1.truncated, r2.truncated);
    }
}

static void STRF_TEST_FUNC char_range_destination_too_small()
{
    {
        char buff[4];
        strf::basic_char_array_writer<char> sw(buff);
        TEST_EQ(sw.space(), 4);
        strf::put(sw, 'a');
        TEST_EQ(sw.space(), 3);
        strf::put(sw, 'b');
        strf::put(sw, 'c');
        TEST_TRUE(sw.good());
        strf::put(sw, 'd');
        TEST_EQ(sw.space(), 0);
        strf::put(sw, 'e');
        TEST_FALSE(sw.good());
        strf::put(sw, 'f');

        auto r = sw.finish();

        TEST_TRUE(r.truncated);
        TEST_TRUE(r.ptr == buff + 4);
        TEST_STRVIEW_EQ(buff, "abcd", 4);
    }
    {
        char buff[8];
        strf::basic_char_array_writer<char> sw(buff);
        write(sw, "Hello");
        write(sw, " World");
        write(sw, "blah blah blah");
        auto r = sw.finish();

        TEST_TRUE(r.truncated);
        TEST_TRUE(r.ptr == buff + 8);
        TEST_STRVIEW_EQ(buff, "Hello Wo", 8);
    }
}

void STRF_TEST_FUNC test_to_range()
{
    char_range_basic_operations<char>();
    char_range_basic_operations<char16_t>();
    char_range_destination_too_small();
}


REGISTER_STRF_TEST(test_to_range);
