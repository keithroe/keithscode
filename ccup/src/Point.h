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

#ifndef CCUP_POINT_H__
#define CCUP_POINT_H__

#include <iostream>

//------------------------------------------------------------------------------
//
// Point
//
//------------------------------------------------------------------------------

struct Point
{
    unsigned char x; //  These can be packed into a single byte if necessary
    unsigned char y; // 

    unsigned char color;
    unsigned char group; 

    unsigned char neighbors[4]; // { L, R, B, T }
};


inline
std::ostream& operator<<( std::ostream& out, const Point& p )
{
    out << "[" << static_cast<int>( p.x ) << "," 
               << static_cast<int>( p.y ) << "] c: " 
               << static_cast<int>( p.color ) << " g: " 
               << static_cast<int>( p.group );
    return out;
}

#endif // CCUP_POINT_H__
