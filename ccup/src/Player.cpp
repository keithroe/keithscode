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

#include "Logger.h"
#include "MCTSAI.h"
#include "RandomAI.h"

#include <sstream>
#include <iostream>

Player::Player()
    : m_move_number( 0 ),
      //m_ai( new RandomAI )
      m_ai( new MCTSAI )
{
}


std::string Player::doMove( const std::string& opponent_move )
{

    m_move_number++;

    if( opponent_move != "Start" )
    {
        Move opp_move; 
        toMove( opponent_move, opp_move );
        m_ai->opponentMove( opp_move );
    }

    Move move; 
    m_ai->getMove( move );

    // TODO: ostream iterator and copy
    std::ostringstream oss;
    for( Move::iterator it = move.begin(); it != move.end(); ++it )
    {
        oss << toString( it->first, it->second );
        if( it + 1 != move.end() )
            oss << "-";
    }
    return oss.str(); 
}

