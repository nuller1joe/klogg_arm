#include <catch2/catch.hpp>

#include "wrappedstring.h"

TEST_CASE( "Wrapping preserves text and word boundaries", "[wrapping]" )
{
    const WrappedString text{ QStringLiteral( "hello world!" ), 8_length };
    REQUIRE( text.wrappedLinesCount() == 2 );
    REQUIRE( WrappedString::countWrappedLines( QStringLiteral( "hello world!" ), 8_length ) == 2 );
    REQUIRE( text.wrappedLine( 0 ).toString() == "hello " );
    REQUIRE( text.wrappedLine( 1 ).toString() == "world!" );
    const auto parts = text.mid( 4_lcol, 5_length );
    REQUIRE( parts.size() == 2 );
    REQUIRE( parts[ 0 ].toString() == "o " );
    REQUIRE( parts[ 1 ].toString() == "wor" );
}

TEST_CASE( "Wrapping always makes progress at zero or negative width", "[wrapping]" )
{
    for ( const auto width : { 0_length, LineLength{ -5 } } ) {
        const WrappedString text{ QStringLiteral( "abc" ), width };
        REQUIRE( text.wrappedLinesCount() == 3 );
        REQUIRE( WrappedString::countWrappedLines( QStringLiteral( "abc" ), width ) == 3 );
        REQUIRE( text.wrappedLine( 2 ).toString() == "c" );
    }
    const WrappedString empty{ QString{}, 0_length };
    REQUIRE( empty.wrappedLinesCount() == 1 );
    REQUIRE( empty.wrappedLine( 0 ).isEmpty() );
    REQUIRE( WrappedString::countWrappedLines( QString{}, 0_length ) == 1 );
}

TEST_CASE( "Very long lines only lay out the requested visible rows", "[wrapping]" )
{
    const QString line( 4 * 1024 * 1024, QLatin1Char{ 'x' } );
    const WrappedString text{ line, 80_length, 40 };
    REQUIRE( text.wrappedLinesCount() == 40 );
    REQUIRE( text.visibleLength() == 3200_length );
    REQUIRE( text.wrappedLine( 39 ).size() == 80 );
    REQUIRE( text.unwrappedLine().size() == line.size() );
    REQUIRE( text.mid( 0_lcol, LineLength{ line.size() } ).size() == 40 );
    REQUIRE( text.mid( 3200_lcol, 80_length ).empty() );
    const WrappedString oneRow{ line, 80_length, 1 };
    REQUIRE( oneRow.mid( 100_lcol, 20_length ).empty() );
    REQUIRE( WrappedString::countWrappedLines( line, 80_length )
             == static_cast<size_t>( ( line.size() + 79 ) / 80 ) );
}

TEST_CASE( "Copied wrap layouts keep their text alive", "[wrapping]" )
{
    const auto makeLayout = [] {
        return WrappedString{ QStringLiteral( "hello world!" ), 8_length };
    };
    auto copy = makeLayout();
    {
        auto original = makeLayout();
        copy = original;
    }
    REQUIRE( copy.wrappedLine( 1 ).toString() == "world!" );
    REQUIRE( copy.unwrappedLine().toString() == "hello world!" );
}
