//  Distributed under the Boost Software License, Version 1.0.
//  (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

#include <strf.hpp>
#include <limits>
#include "test_utils.hpp"

int main()
{
    TEST ( "true") ( true );
    TEST (u"true") ( true );
    TEST (U"true") ( true );
    TEST (L"true") ( true );

    TEST ( "True").with(strf::mixedcase) ( true );
    TEST (u"True").with(strf::mixedcase) ( true );
    TEST (U"True").with(strf::mixedcase) ( true );
    TEST (L"True").with(strf::mixedcase) ( true );

    TEST ( "TRUE").with(strf::uppercase) ( true );
    TEST (u"TRUE").with(strf::uppercase) ( true );
    TEST (U"TRUE").with(strf::uppercase) ( true );
    TEST (L"TRUE").with(strf::uppercase) ( true );

    TEST ( "false") ( false );
    TEST (u"false") ( false );
    TEST (U"false") ( false );
    TEST (L"false") ( false );

    TEST ( "False").with(strf::mixedcase) ( false );
    TEST (u"False").with(strf::mixedcase) ( false );
    TEST (U"False").with(strf::mixedcase) ( false );
    TEST (L"False").with(strf::mixedcase) ( false );

    TEST ( "FALSE").with(strf::uppercase) ( false );
    TEST (u"FALSE").with(strf::uppercase) ( false );
    TEST (U"FALSE").with(strf::uppercase) ( false );
    TEST (L"FALSE").with(strf::uppercase) ( false );

    TEST ("true      ")  (  strf::left(true, 10) );
    TEST ("   true   ")  (  strf::center(true, 10) );
    TEST ("      true")  (  strf::right(true, 10) );
    TEST ("      true")  (  strf::split(true, 10) );
    TEST ("      True").with(strf::mixedcase)  (  strf::right(true, 10) );
    TEST ("      TRUE").with(strf::uppercase)  (  strf::right(true, 10) );

    TEST ("false     ")  (  strf::left(false, 10) );
    TEST ("  false   ")  (  strf::center(false, 10) );
    TEST ("     false")  (  strf::split(false, 10) );
    TEST ("     false")  (  strf::right(false, 10) );
    TEST ("     False").with(strf::mixedcase)  (  strf::right(false, 10) );
    TEST ("     FALSE").with(strf::uppercase)  (  strf::right(false, 10) );

    constexpr auto j = strf::join_right(20, U'_');

    TEST( "________________true") ( j(true) );
    TEST(u"________________true") ( j(true) );
    TEST(U"________________true") ( j(true) );
    TEST(L"________________true") ( j(true) );
    TEST( "_______________false") ( j(false) );
    TEST(u"_______________false") ( j(false) );
    TEST(U"_______________false") ( j(false) );
    TEST(L"_______________false") ( j(false) );

    TEST ("__________true      ") ( j(strf::left(true, 10)) );
    TEST ("__________   true   ") ( j(strf::center(true, 10)) );
    TEST ("__________      true") ( j(strf::right(true, 10)) );
    TEST ("__________      true") ( j(strf::split(true, 10)) );

    TEST ("__________false     ") ( j(strf::left(false, 10)) );
    TEST ("__________  false   ") ( j(strf::center(false, 10)) );
    TEST ("__________     false") ( j(strf::right(false, 10)) );
    TEST ("__________     false") ( j(strf::split(false, 10)) );

    return test_finish();
}