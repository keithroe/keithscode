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

#ifndef CCUP_BOARD_H__
#define CCUP_BOARD_H__

#include "Point.h"
#include "Util.h"

//------------------------------------------------------------------------------
//
// Game board
//
//------------------------------------------------------------------------------

class Board
{
public:
    Board();

    Board( const Board& orig );

    // No error checking for now
    void set( int x, int y, Color color ); 
    
    // No error checking for now
    void set( const Move& move, Color color ); 

    // No error checking for now
    const Point& get( int x, int y )const
    { return m_grid[ x*GRID_SIZE+y ]; } 

    Point& get( int x, int y )
    { return m_grid[ x*GRID_SIZE+y ]; } 
    
    const Point& get( int idx )const
    { return m_grid[ idx ]; } 

    Point& get( int idx )
    { return m_grid[ idx ]; } 

    void setColor( Color c )
    { m_color = c; }

    int numStones( Color color )const
    { return color == WHITE ? m_num_white_stones : m_num_black_stones; }

    int numWhiteStones()const
    { return m_num_white_stones; }

    int numBlackStones()const
    { return m_num_black_stones; }

    int score( Color c )const
    {
        return c == WHITE                                            ? 
               m_num_white_stones - GROUP_PENALTY*m_num_white_groups :
               m_num_black_stones - GROUP_PENALTY*m_num_black_groups ;
    }

    bool gameFinished()const
    {  return m_num_black_stones + m_num_white_stones == NUM_GRID_CELLS; }

    const Point* grid()const 
    { return m_grid; }
    
    Point* grid()
    { return m_grid; }

    Color winner()const;

    bool legalExploration( Color c, int idx )const;
    bool legalExploration( Color c, int x, int y )const;

    static int wrap( int x );
    static int clamp( int x );

    
private:
    void changeGroup( int old_group, int new_group );

    Color       m_color;
    int         m_next_group;
    int         m_num_white_groups;
    int         m_num_black_groups;
    int         m_num_white_stones;
    int         m_num_black_stones;
    Point       m_grid[NUM_GRID_CELLS];
};

std::ostream& operator<<( std::ostream& out, const Board& board );
bool operator==( const Board& b0, const Board& b1 );

#endif // CCUP_BOARD_H__
