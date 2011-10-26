
#ifndef DIRECTION_H_
#define DIRECTION_H_
/*
    constants
*/

enum Direction
{
    NORTH=0,
    EAST,
    SOUTH,
    WEST,
    NUM_DIRECTIONS,
    NONE

};

const char DIRECTION_CHAR[NUM_DIRECTIONS] = { 'N', 'E', 'S', 'W' };
const int  DIRECTION_OFFSET[4][2]         = { {-1,0}, {0,1}, {1,0}, {0,-1} };


#endif // DIRECTION_H_
