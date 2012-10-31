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

#ifndef CCUP_RANDOM_AI_H__
#define CCUP_RANDOM_AI_H__

#include "AI.h"

//------------------------------------------------------------------------------
//
// RandomAI 
//
//------------------------------------------------------------------------------


// TODO: at this point there is a loose coupling between the board passed in
//   and the exploration list.  This means the same board must be passed in each
//   time with only the moves recommended by the RandomAI being used on the
//   board.  AI should probably own board.  This should be fixed so I can sleep
//   at night.

class RandomAI : public AI
{
    
public:
    RandomAI();

    virtual ~RandomAI() {}


protected:
    virtual void doGetMove( Move& move );

    std::vector<unsigned char> m_explorations;
    std::vector<unsigned char> m_expansions;
};


#endif // CCUP_RANDOM_AI_H__

