//
// MIT License
//
// Copyright (c) 2008 r. keith morley 
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE
//

#include "Util.h"
#include "Board.h"
#include "Logger.h"

#include <cassert>
#include <set>

bool chooseExploration(
        Color color,
        const Board& board,
        std::vector<unsigned char>& potential_explorations,
        Move& move 
        )
{
    LDEBUG << "Choosing exploration *************************";
    move.clear();

    // Walk the vector till we find the first legal move
    while( !potential_explorations.empty() )
    {
        const int idx = potential_explorations.back();
        potential_explorations.pop_back();


        if( board.legalExploration( color, idx ) )
        {
            int x, y;
            to2D( idx, x, y );
            move.push_back( std::make_pair( x, y ) );
            return true;
        }
    }

    // No legal exploration found
    return false;

}

bool chooseExpansion(
        Color color,
        const Board& board,
        const std::vector<unsigned char>& random_points,
        Move& move 
        )
{
    LDEBUG << "Choosing expansion *************************";
    assert( board.numStones( color ) != 0 );
    
    move.clear();
    std::set< int > expansions;

    int adjacent_groups[4];

    // Iterate over points in random order
    for( std::vector<unsigned char>::const_iterator it = random_points.begin();
         it != random_points.end();
         ++it )
    {
        const int idx = *it;
        const Point& p = board.get( idx );

        int x, y;
        to2D( idx, x, y );
        if( p.color == NONE ) // empty point
        {
            LDEBUG1 << "considering " << x << "," << y;

            // Find this points adjacent groups
            int num_adjacent = 0;
            for( int i = 0; i < 4; ++i )
            {
                int nidx = p.neighbors[ i ];
                if( nidx == INVALID_IDX )
                    continue;

                const Point& neighbor_p = board.get( nidx );
                if( neighbor_p.color == color )
                {
                    int ngroup = neighbor_p.group;

                    // Avoid redundant group entries
                    if( ( num_adjacent < 1 || adjacent_groups[0] != ngroup ) &&
                        ( num_adjacent < 2 || adjacent_groups[1] != ngroup ) && 
                        ( num_adjacent < 3 || adjacent_groups[2] != ngroup ) ) 
                        adjacent_groups[ num_adjacent++ ] = neighbor_p.group;
                }
            }

            // Not on border of any group -- skip it
            if( num_adjacent == 0 )
            {
                LDEBUG1 << "       no adj groups, skipping";
                continue;
            }

            int x, y;
            to2D( idx, x, y );
            const std::pair<int, int> coord = std::make_pair( x, y );

            // If we have a single adjacent group
            if( num_adjacent == 1 )
            {
                int group = adjacent_groups[0];
                if( expansions.find( group ) != expansions.end() )
                {
                    LDEBUG1 << "         group already expanded, skipping";
                    continue;
                }

                LDEBUG1 << "        inserting ";
                expansions.insert( group ); 
                move.push_back( coord );
                continue;
            }

            // If we have multiple adjacent friendly groups, add this move if
            // none of the adjacent groups have been expanded yet
            if( num_adjacent > 1 )
            {
                bool ok_to_join = true;
                for( int i = 0; i < num_adjacent; ++i )
                {
                    if( expansions.count( adjacent_groups[ i ] ) > 0 )
                    {
                        ok_to_join = false;
                        break;
                    }
                }
                if( ok_to_join )
                {
                    for( int i = 0; i < num_adjacent; ++i )
                        expansions.insert( adjacent_groups[ i ] ); 

                    move.push_back( coord );
                    LDEBUG1 << "       >1 adj groups, works!!";
                }
                continue;
            }

            LDEBUG1 << "       >1 adj groups, skipping";
        }
    }

    return( !move.empty() );
}

