
#ifndef DIRECTION_H_
#define DIRECTION_H_

#include <cassert>

enum Direction
{
    NORTH=0,
    EAST,
    SOUTH,
    WEST,
    NONE,
    NUM_DIRECTIONS

};

const char DIRECTION_CHAR[NUM_DIRECTIONS] = { 'N', 'E', 'S', 'W' };
const int  DIRECTION_OFFSET[4][2]         = { {-1,0}, {0,1}, {1,0}, {0,-1} };

inline Direction reverseDirection( Direction dir )
{
    assert( dir != NONE );
    return static_cast<Direction>( ( static_cast<int>( dir ) + 2 ) % 4 );
}

#endif // DIRECTION_H_
