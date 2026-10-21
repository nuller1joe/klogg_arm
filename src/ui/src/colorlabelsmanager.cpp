/*
 * Copyright (C) 2021 Anton Filimonov and other contributors
 *
 * This file is part of klogg.
 *
 * klogg is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * klogg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with klogg.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "colorlabelsmanager.h"
#include "highlighterset.h"
#include <algorithm>
#include <vector>

QStringList ColorLabelsManager::selectionTexts( const QString& text )
{
    auto texts = text.split( QChar::LineFeed );
    for ( auto& line : texts ) {
        if ( line.endsWith( QChar::CarriageReturn ) ) {
            line.chop( 1 );
        }
    }
    texts.removeAll( QString{} );
    texts.removeDuplicates();
    return texts;
}

ColorLabelsManager::QuickHighlightersCollection ColorLabelsManager::colorLabels() const
{
    return quickHighlighters_;
}

ColorLabelsManager::QuickHighlightersCollection ColorLabelsManager::clear()
{
    for ( auto& quickHighlighters : quickHighlighters_ ) {
        quickHighlighters.clear();
    }
    currentLabel_.reset();
    
    return quickHighlighters_;
}

ColorLabelsManager::QuickHighlightersCollection
ColorLabelsManager::setColorLabel( size_t label, const QString& text )
{
    return updateColorLabel( label, text, true );
}

ColorLabelsManager::QuickHighlightersCollection
ColorLabelsManager::setNextColorLabel( const QString& text )
{
    const auto& quickHighlightersConfiguration
        = HighlighterSetCollection::get().quickHighlighters();

    std::vector<size_t> cycle;
    cycle.reserve( static_cast<size_t>( quickHighlightersConfiguration.size() ) );

    for ( auto i = 0; i < quickHighlightersConfiguration.size(); ++i ) {
        if ( quickHighlightersConfiguration[ i ].useInCycle ) {
            cycle.push_back( static_cast<size_t>( i ) );
        }
    }

    if ( cycle.empty() ) {
        return quickHighlighters_;
    }

    auto nextLabel = cycle.front();

    if ( currentLabel_.has_value() ) {
        auto nextIt = std::upper_bound( cycle.cbegin(), cycle.cend(), *currentLabel_ );
        if ( nextIt != cycle.cend() ) {
            nextLabel = *nextIt;
        }
    }

    currentLabel_ = nextLabel;

    return updateColorLabel( nextLabel, text, false );
}

ColorLabelsManager::QuickHighlightersCollection
ColorLabelsManager::updateColorLabel( size_t label, const QString& text, bool replaceCurrent )
{
    if ( label > quickHighlighters_.size() ) {
        return quickHighlighters_;
    }

    const auto texts = selectionTexts( text );
    // Apply one consistent action to the whole selection. Choosing an existing
    // label toggles it off only when every selected text already has that label.
    const auto removeLabel = std::all_of( texts.cbegin(), texts.cend(),
        [ & ]( const QString& line ) {
            if ( label == quickHighlighters_.size() ) {
                return true;
            }
            if ( replaceCurrent ) {
                return quickHighlighters_[ label ].contains( line );
            }
            return std::any_of( quickHighlighters_.cbegin(), quickHighlighters_.cend(),
                [ & ]( const auto& highlights ) { return highlights.contains( line ); } );
        } );

    for ( const auto& line : texts ) {
        for ( auto& highlights : quickHighlighters_ ) {
            highlights.removeAll( line );
        }
        if ( !removeLabel ) {
            quickHighlighters_[ label ].append( line );
        }
    }

    return quickHighlighters_;
}
