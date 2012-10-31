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

#include "Board.h"

#include <cassert>
#include <cstring>
#include <iostream>
#include <iomanip>


Board::Board()
    : m_color( WHITE ),
      m_next_group( 0 ),
      m_num_white_groups( 0 ),
      m_num_black_groups( 0 ),
      m_num_white_stones( 0 ),
      m_num_black_stones( 0 )
{
    int idx = 0;
    for( int i = 0; i < GRID_SIZE; ++i )
    {
        for( int j = 0; j < GRID_SIZE; ++j )
        {
            Point& p = m_grid[ idx++ ];
            p.x       = static_cast<unsigned char>( i );
            p.y       = static_cast<unsigned char>( j );
            p.color   = static_cast<unsigned char>( NONE );
            p.group   = 0;
            p.neighbors[0] = ( i == 0           ? INVALID_IDX : to1D(i-1, j) );
            p.neighbors[1] = ( i == GRID_SIZE-1 ? INVALID_IDX : to1D(i+1, j) );
            p.neighbors[2] = ( j == 0           ? INVALID_IDX : to1D(i, j-1) );
            p.neighbors[3] = ( j == GRID_SIZE-1 ? INVALID_IDX : to1D(i, j+1) );
        }
    }
}


Board::Board( const Board& orig )
    : m_color( orig.m_color ),
      m_next_group( orig.m_next_group ),
      m_num_white_groups( orig.m_num_white_groups ),
      m_num_black_groups( orig.m_num_black_groups ),
      m_num_white_stones( orig.m_num_white_stones ),
      m_num_black_stones( orig.m_num_black_stones )
{
    memcpy( m_grid, orig.m_grid, sizeof( m_grid ) );
}


void Board::set( int x, int y, Color color ) 
{ 
    assert( x >= 0 && x < GRID_SIZE ); 
    assert( y >= 0 && y < GRID_SIZE ); 
    assert( m_grid[ x*GRID_SIZE+y ].color == NONE );

    if( color == WHITE )
        m_num_white_stones++;
    else
        m_num_black_stones++;

    const int idx = to1D( x, y );

    Point& p = m_grid[ idx ];
    p.color = static_cast<unsigned char>( color ); 

    // Assign group to first neighbor group we find
    for( int i = 0; i < 4; ++i )
    {
        int nidx = p.neighbors[ i ];
        if( nidx == INVALID_IDX )
            continue;

        if( m_grid[ nidx ].color == color && m_grid[ nidx ].group != 0  )
        {
            p.group = m_grid[ nidx ].group;
            break;
        }
    }
    
    // All neighbors are empty, create new group
    if( p.group == 0 )
    {
        p.group = ++m_next_group; 
        if( color == WHITE )
            m_num_white_groups++;
        else
            m_num_black_groups++;
        return;
    }

    // Collapse groups if necessary 
    for( int i = 0; i < 4; ++i )
    {
        int nidx = p.neighbors[ i ];
        if( nidx != INVALID_IDX             &&
            m_grid[ nidx ].color == color   &&
            m_grid[ nidx ].group != 0       &&
            m_grid[ nidx ].group != p.group )
        {
            changeGroup( m_grid[ nidx ].group, p.group);
            if( color == WHITE )
                m_num_white_groups--;
            else
                m_num_black_groups--;
        }
    }
}

    
void Board::set( const Move& move, Color color )
{
    for( Move::const_iterator it = move.begin(); it != move.end(); ++it )
        set( it->first, it->second, color );
}


Color Board::winner()const
{
    const int wscore = m_num_white_stones - GROUP_PENALTY*m_num_white_groups;
    const int bscore = m_num_black_stones - GROUP_PENALTY*m_num_black_groups;

    if( wscore > bscore )
        return WHITE;
    else
        return BLACK;
}


void Board::changeGroup( int old_group, int new_group )
{
    for( int i = 0; i < NUM_GRID_CELLS; ++i )
        if( m_grid[i].group == old_group )
            m_grid[i].group = new_group;
}


bool Board::legalExploration( Color c, int idx )const
{
    assert( idx < NUM_GRID_CELLS );
    const Point* g = m_grid; 
    const Point& p = g[idx];

    return( p.color == NONE                                                   &&
          ( p.neighbors[0] == INVALID_IDX || g[ p.neighbors[0] ].color != c ) &&
          ( p.neighbors[1] == INVALID_IDX || g[ p.neighbors[1] ].color != c ) &&
          ( p.neighbors[2] == INVALID_IDX || g[ p.neighbors[2] ].color != c ) &&
          ( p.neighbors[3] == INVALID_IDX || g[ p.neighbors[3] ].color != c ) );
}


bool Board::legalExploration( Color c, int x, int y )const
{
    return legalExploration( c, to1D( x, y ) );
}


int Board::wrap( int x )
{ 
    const int s = GRID_SIZE;
    return x < 0   ? s-1 : 
           x > s-1 ? 0   :
           x;
}


int Board::clamp( int x )
{ 
    const int s = GRID_SIZE;
    return x < 0   ? 0   : 
           x > s-1 ? s-1 :
           x;
}


std::ostream& operator<<( std::ostream& out, const Board& board )
{
    out << " -";
    for( int i = 0; i < GRID_SIZE; ++i )
        out << "--";
    out << "\n";

    for( int i = GRID_SIZE - 1; i >= 0; --i ) // row
    {
        out << "| "; 
        for( int j = 0; j < GRID_SIZE; ++j ) // col
        {
            out << toChar( static_cast<Color>( board.get( j, i ).color ) ) 
                << ' ';
        }
        out << "| " << i + 1 << "\n";
    }

    out << " -";
    for( int i = 0; i < GRID_SIZE; ++i )
        out << "--";
    out << std::endl << " "; //flush
    for( int i = 0; i < GRID_SIZE; ++i )
        out << " " << static_cast<char>( 'A' + i );
    out << std::endl; //flush




    out << " -";
    for( int i = 0; i < GRID_SIZE; ++i )
        out << "----";
    out << "\n";

    for( int i = GRID_SIZE - 1; i >= 0; --i ) // row
    {
        out << "|"; 
        for( int j = 0; j < GRID_SIZE; ++j ) // col
        {
            int g = static_cast<int>( board.get( j, i ).group );
            if( g )
                out << std::setw( 4 ) << g ;
            else
                out << std::setw( 4 ) << '.';
        }
        out << " | " << i + 1 << "\n";
    }

    out << " -";
    for( int i = 0; i < GRID_SIZE; ++i )
        out << "----";
    out << std::endl << " "; //flush
    for( int i = 0; i < GRID_SIZE; ++i )
        out << "   " << static_cast<char>( 'A' + i );
    out << std::endl; //flush

    return out;
}

bool operator==( const Board& b0, const Board& b1 )
{
    //return !memcmp( b0.grid(), b1.grid() );
    const Point* g0 = b0.grid();
    const Point* g1 = b1.grid();
    for( int i = 0; i < NUM_GRID_CELLS; ++i )
        if( g0[i].color != g1[i].color )
            return false;
    return true;
}


