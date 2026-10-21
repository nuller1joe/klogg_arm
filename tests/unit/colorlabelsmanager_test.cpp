#include <catch2/catch.hpp>

#include "colorlabelsmanager.h"

TEST_CASE( "Color labels apply to each selected line", "[colorlabels]" )
{
    ColorLabelsManager manager;

    auto labels = manager.setColorLabel( 0, "first\nsecond\nfirst\n\n" );
    REQUIRE( ( labels[ 0 ] == QStringList{ "first", "second" } ) );

    // Mixed selections must receive one consistent color.
    manager.setColorLabel( 1, "second" );
    labels = manager.setColorLabel( 0, "first\nsecond\nthird" );
    REQUIRE( ( labels[ 0 ] == QStringList{ "first", "second", "third" } ) );
    REQUIRE( labels[ 1 ].isEmpty() );

    // Selecting the same color again removes it from the entire selection.
    labels = manager.setColorLabel( 0, "first\nsecond\nthird" );
    REQUIRE( labels[ 0 ].isEmpty() );
}

TEST_CASE( "Color labels preserve single-line behavior", "[colorlabels]" )
{
    ColorLabelsManager manager;
    REQUIRE( manager.setColorLabel( 0, "some text" )[ 0 ].contains( "some text" ) );
    REQUIRE( manager.setColorLabel( 0, "some text" )[ 0 ].isEmpty() );

    manager.setColorLabel( 0, "some text" );
    const auto labels = manager.setColorLabel( 1, "some text" );
    REQUIRE( labels[ 0 ].isEmpty() );
    REQUIRE( labels[ 1 ].contains( "some text" ) );
}

TEST_CASE( "Removing selection labels preserves unrelated labels", "[colorlabels]" )
{
    ColorLabelsManager manager;
    manager.setColorLabel( 0, "first\nunrelated" );
    manager.setColorLabel( 1, "second" );
    const auto labels = manager.setColorLabel( manager.colorLabels().size(), "first\nsecond" );
    REQUIRE( labels[ 0 ] == QStringList{ "unrelated" } );
    REQUIRE( labels[ 1 ].isEmpty() );
}

TEST_CASE( "Color label selections handle CRLF and empty lines", "[colorlabels]" )
{
    REQUIRE( ( ColorLabelsManager::selectionTexts( "first\r\n\r\nsecond\r\n" )
               == QStringList{ "first", "second" } ) );
    REQUIRE( ColorLabelsManager::selectionTexts( "\n\n" ).isEmpty() );

    ColorLabelsManager manager;
    REQUIRE( manager.setColorLabel( 0, "" )[ 0 ].isEmpty() );
    REQUIRE( manager.setColorLabel( 99, "first" ) == manager.colorLabels() );
}
