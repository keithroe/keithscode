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

#include "RandomAI.h"
#include "Board.h"

RandomAI::RandomAI()
    : AI(),
      m_explorations( NUM_GRID_CELLS ),
      m_expansions( NUM_GRID_CELLS )
{
    for( int i = 0; i < NUM_GRID_CELLS; ++i )
        m_explorations[i] = m_expansions[i] = i;
    
    std::random_shuffle( m_explorations.begin(), m_explorations.end() ); 
    std::random_shuffle( m_expansions.begin(), m_expansions.end() ); 
}


void RandomAI::doGetMove( Move& move )
{
    chooseRandomMove( m_color, m_board, 0.5f, m_expansions, m_explorations,
                      move );
}


