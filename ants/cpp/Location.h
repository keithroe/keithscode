
#ifndef LOCATION_H_
#define LOCATION_H_

//------------------------------------------------------------------------------
//
// struct for representing locations in the grid.
//
//------------------------------------------------------------------------------

#include <ostream>

struct Location;

//
// Utility functions
//
Location offset( const Location& loc, const int direction[ 2 ] );
Location clamp( const Location& loc, int height, int width );
std::ostream& operator<<( std::ostream& out, const Location& loc );
bool operator<( const Location& loc0, const Location& loc1 );


//
// Location class
//
struct Location
{
    int row, col;

    Location()
    {
        row = col = 0;
    };

    Location(int r, int c)
    {
        row = r;
        col = c;
    };

    void offset( const int direction[ 2 ] );
    void clamp( int height, int width );

    int  hash()const { return ( row << 16  ) + col; }

    bool operator==(const Location& other )
    { return row == other.row && col == other.col; }
};


inline void Location::offset( const int direction[ 2 ] )
{
    row += direction[ 0 ];
    col += direction[ 1 ];
}


inline void Location::clamp( int height, int width )
{
    row = ( row + height ) % height;
    col = ( col + width  ) % width;
}


inline Location offset( const Location& loc, const int direction[ 2 ] )
{
    Location temp( loc.row, loc.col );
    temp.offset( direction );
    return temp;
}


inline Location clamp( const Location& loc, int height, int width )
{
    Location temp( loc.row, loc.col );
    temp.clamp( height, width );
    return temp;
}

inline bool operator<( const Location& loc0, const Location& loc1 )
{
    return loc0.hash() < loc1.hash();
}

#endif //LOCATION_H_
