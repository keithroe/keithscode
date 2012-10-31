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


#ifndef CCUP_AI_H__
#define CCUP_AI_H__

#include "Util.h"
#include "Board.h"


//------------------------------------------------------------------------------
//
// AI base class
//   
//------------------------------------------------------------------------------

class AI
{
public:
    AI();

    virtual ~AI() {}

    void opponentMove( const Move& move );
    void getMove( Move& move );
    
    const Board& board()const;

protected:
    virtual void doGetMove( Move& move )=0;

    Board               m_board;         // Current game state
    int                 m_move_number;   // Move number
    Color               m_color;         // AI player's color
    Color               m_opp_color;     // Opponent's color
    std::vector<Move>   m_moves;         // AI player's moves
    std::vector<Move>   m_opp_moves;     // Opponent's moves
};


inline
AI::AI()
    : m_move_number( 1 ),
      m_color( NONE ),
      m_opp_color( NONE )
{
}


inline 
void AI::opponentMove( const Move& move )
{
    if( m_color == NONE )
    {
        m_color     = BLACK;
        m_opp_color = WHITE;
    }
    m_board.set( move, m_opp_color );
    m_opp_moves.push_back( move );
    
    if( m_opp_color == BLACK )
        m_move_number++;
}


inline
void AI::getMove( Move& move )
{
    if( m_color == NONE )
    {
        m_color     = WHITE;
        m_opp_color = BLACK;
    }

    doGetMove( move );
    m_board.set( move, m_color );
    m_moves.push_back( move );

    if( m_color == BLACK )
        m_move_number++;
}


inline
const Board& AI::board()const
{
    return m_board;
}

#endif // CCUP_AI_H__

