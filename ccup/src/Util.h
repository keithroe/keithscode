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

#ifndef CCUP_UTIL_H__
#define CCUP_UTIL_H__


#include <vector>
#include <string>
#include <cmath>


//------------------------------------------------------------------------------
//
//  Helper decls
//
//------------------------------------------------------------------------------

class Board;


const int GRID_SIZE      = 15;
const int NUM_GRID_CELLS = GRID_SIZE*GRID_SIZE;
const int INVALID_IDX    = 255;
const int GROUP_PENALTY  = 6;


typedef std::vector< std::pair<int, int> > Move;

enum Color { NONE=0, WHITE, BLACK, };

char toChar( Color c );

// Currently no validation
int extractRow( const std::string& position );

// Currently no validation
int extractCol( const std::string& position );

void toCoord( const std::string& position, int& x, int& y );

std::string toString( int x, int y );

int to1D( int x, int y );
void to2D( int i, int& x, int& y );

bool chooseExpansion(
        Color color,
        const Board& board,
        const std::vector<unsigned char>& random_points,
        Move& move 
        );

bool chooseExploration(
        Color color,
        const Board& board,
        std::vector<unsigned char>& potential_expolorations,
        Move& move 
        );

float uct( float score, float num_visits, float num_visits_parent );



//------------------------------------------------------------------------------
//
//  Helper inline defs 
//
//------------------------------------------------------------------------------



inline char toChar( Color c )
{
    return c == WHITE ? 'W' :
           c == BLACK ? 'B' :
                        '.';
}

// Currently no validation
inline int extractRow( const std::string& position )
{
    const int offset = 1u; // Index is 1 based (A1 maps to index 0,0)

    if( position.size() == 2 )
        return position[1] - '0' - offset;
    else
        return ( position[1] - '0' ) * 10u + position[2] - '0' - offset;
}


// Currently no validation
inline int extractCol( const std::string& position )
{
    return position[0] - 'A';
}


inline void toCoord( const std::string& position, int& x, int& y )
{
    x = extractCol( position );
    y = extractRow( position );
}


inline std::string toString( int x, int y )
{
    char buf[ 4 ];
    buf[0] = 'A' + x;

    y += 1; // One based index
    if( y < 10 )
    {
        buf[1] = '0' + y;
        buf[2] = '\0';
    }
    else
    {
        buf[1] = '1';
        buf[2] = '0' + ( y - 10 );
        buf[3] = '\0';
    }
    return buf;
}




inline int to1D( int x, int y )
{
    return x*GRID_SIZE+y;
}


inline void to2D( int i, int& x, int& y )
{
    x = i / GRID_SIZE;
    y = i % GRID_SIZE;
}


inline float uct( float score, float num_visits, float num_visits_parent )
{
    //const float C = 0.1f;
    const float C = 0.01f;
    return score + C * sqrtf( ( logf( num_visits_parent+1.0f ) ) / 
                              ( num_visits+1.0f ) );
}

#endif // CCUP_UTIL_H__

