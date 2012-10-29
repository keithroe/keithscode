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

#include "Player.h"
#include "MCTSAI.h"

#include <sstream>
#include <iostream>
#include "Logger.h"

Player::Player()
    : m_color( NONE ),
      m_opp_color( NONE ),
      m_move_number( 0 ),
      m_ai( new MCTSAI )

{
}


std::string Player::doMove( const std::string& opponent_move )
{

    m_move_number++;

    if( m_color == NONE ) // First move
    {
        if( opponent_move == "Start" )
        {
            m_color     = WHITE;
            m_opp_color = BLACK;
        }
        else
        {
            m_color     = BLACK;
            m_opp_color = WHITE;
        }

        m_board.setColor( m_color );
    }

    placeOpponentStones( opponent_move );

    Move move; // TODO: persistent
    m_ai->chooseMove( m_color, m_board, move );

    std::cerr << "Making move: " << std::endl;
    for( Move::iterator it = move.begin(); it != move.end(); ++it )
    {
        std::cerr << "  "  << toString( it->first, it->second ) << std::endl;
    }
    std::cerr << "   on board\n" << m_board << std::endl;

    std::ostringstream oss;
    for( Move::iterator it = move.begin(); it != move.end(); ++it )
    {
        m_board.set( it->first, it->second, m_color );
        oss << toString( it->first, it->second );
        if( it + 1 != move.end() )
            oss << "-";
    }
    return oss.str(); 
}


void Player::placeOpponentStones( const std::string& opponent_move )
{
    if( opponent_move != "Start" )
    {
        std::istringstream iss( opponent_move );
        std::string position;
        while( std::getline( iss, position, '-' ) )
        {
            int x, y;
            toCoord( position, x, y );
            m_board.set( x, y, m_opp_color );
        }
    }
}


