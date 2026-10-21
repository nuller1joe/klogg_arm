/*
 * Copyright (C) 2023 -- 2024 Anton Filimonov and other contributors
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

#ifndef KLOGG_WRAPPEDSTRING_H
#define KLOGG_WRAPPEDSTRING_H

#include <algorithm>
#include <iterator>
#include <limits>
#include <memory>
#include <QString>
#include <cstddef>
#include <qchar.h>
#include <qglobal.h>

#include <QStringView>

#include "containers.h"
#include "linetypes.h"

class WrappedString {
public:
    using WrappedStringPart = QStringView;

    static WrappedStringPart makeWrappedStringPart(const QString& lineText, 
        LineColumn firstCol, LineLength length ) {
        return QStringView( lineText ).mid( firstCol.get(), length.get() );
    }

    explicit WrappedString( QString longLine, LineLength visibleColumns,
                            size_t maxWrappedLines = std::numeric_limits<size_t>::max() )
    {
        unwrappedLine_ = std::move( longLine );
        // Even a collapsed viewport must consume at least one character per row.
        const auto width = std::max( visibleColumns.get(), LineLength::UnderlyingType{ 1 } );
        maxWrappedLines = std::max( maxWrappedLines, size_t{ 1 } );
        if ( unwrappedLine_.isEmpty() ) {
            wrappedLines_->push_back( WrappedStringPart{} );
        }
        else {
            WrappedStringPart lineToWrap( unwrappedLine_ );
            while ( lineToWrap.size() > width && wrappedLines_->size() < maxWrappedLines ) {
                const auto rowLength = nextRowLength( lineToWrap, width );
                wrappedLines_->push_back( lineToWrap.left( rowLength ) );
                lineToWrap = lineToWrap.mid( rowLength );
            }
            if ( lineToWrap.size() > 0 && wrappedLines_->size() < maxWrappedLines ) {
                wrappedLines_->push_back( lineToWrap );
            }
        }
    }

    size_t wrappedLinesCount() const
    {
        return wrappedLines_->size();
    }

    // Scrollbar calculations only need a count, not a stored view for every row.
    static size_t countWrappedLines( const QString& text, LineLength visibleColumns )
    {
        auto remaining = QStringView{ text };
        const auto width = std::max( visibleColumns.get(), LineLength::UnderlyingType{ 1 } );
        size_t count = 1;
        while ( remaining.size() > width ) {
            remaining = remaining.mid( nextRowLength( remaining, width ) );
            ++count;
        }
        return count;
    }

    LineLength visibleLength() const
    {
        const auto& last = wrappedLines_->back();
        return last.isEmpty() ? 0_length
                             : LineLength{ last.data() + last.size() - unwrappedLine_.constData() };
    }

    klogg::vector<WrappedStringPart> mid( LineColumn start, LineLength length ) const
    {
        auto getLength = []( const auto& view ) -> LineLength::UnderlyingType {
            return type_safe::narrow_cast<LineLength::UnderlyingType>( view.size() );
        };

        klogg::vector<WrappedStringPart> resultChunks;
        if ( start < 0_lcol || length <= 0_length || start.get() >= visibleLength().get() ) {
            return resultChunks;
        }
        if ( wrappedLines_->size() == 1 ) {
            auto& wrappedLine = wrappedLines_->front();
            auto len = std::min( length.get(), getLength( wrappedLine ) - start.get() );
            resultChunks.push_back( wrappedLine.mid( start.get(), ( len > 0 ? len : 0 ) ) );
            return resultChunks;
        }

        size_t wrappedLineIndex = 0;
        auto positionInWrappedLine = start.get();
        while ( positionInWrappedLine > getLength( (*wrappedLines_)[ wrappedLineIndex ] ) ) {
            positionInWrappedLine -= getLength( (*wrappedLines_)[ wrappedLineIndex ] );
            wrappedLineIndex++;
            if ( wrappedLineIndex >= wrappedLines_->size() ) {
                return resultChunks;
            }
        }

        auto chunkLength = length.get();
        while ( positionInWrappedLine + chunkLength
                > getLength( (*wrappedLines_)[ wrappedLineIndex ] ) ) {
            resultChunks.push_back(
                (*wrappedLines_)[ wrappedLineIndex ].mid( positionInWrappedLine ) );
            wrappedLineIndex++;
            positionInWrappedLine = 0;
            chunkLength -= getLength( resultChunks.back() );
            if ( wrappedLineIndex >= wrappedLines_->size() ) {
                return resultChunks;
            }
        }

        if ( chunkLength > 0 ) {
            auto& wrappedLine = (*wrappedLines_)[ wrappedLineIndex ];
            auto len = std::min( chunkLength, getLength( wrappedLine ) - positionInWrappedLine );
            resultChunks.push_back(
                wrappedLine.mid( positionInWrappedLine, ( len > 0 ? len : 0 ) ) );
        }

        return resultChunks;
    }

    bool isEmpty() const
    {
        return unwrappedLine_.isEmpty();
    }

    WrappedStringPart unwrappedLine() const {
        return WrappedStringPart{unwrappedLine_};
    }

    WrappedStringPart wrappedLine(size_t index) const {
        return (*wrappedLines_)[index];
    }

private:
    static LineLength::UnderlyingType nextRowLength( WrappedStringPart text,
                                                     LineLength::UnderlyingType width )
    {
        const auto row = text.left( width );
        const auto space = std::find_if( row.rbegin(), row.rend(),
                                        []( QChar c ) { return c.isSpace(); } );
        return space == row.rend() ? width : std::distance( row.begin(), space.base() );
    }

    // A row in the hit-test cache copies this object. Share the layout rather
    // than copying all wrapped rows for each cached row (quadratic memory).
    std::shared_ptr<klogg::vector<WrappedStringPart>> wrappedLines_
        = std::make_shared<klogg::vector<WrappedStringPart>>();
    QString unwrappedLine_;
};
#endif // KLOGG_WRAPPEDSTRING_H
