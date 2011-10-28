
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
Location wrap( const Location& loc, int max_row, int max_col );
std::ostream& operator<<( std::ostream& out, const Location& loc );

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
    void wrap( int max_row, int max_col );

    bool operator==(const Location& other )
    { return row == other.row && col == other.col; }
};


inline void Location::offset( const int direction[ 2 ] )
{
    row += direction[ 0 ];
    col += direction[ 1 ];
}


inline void Location::wrap( int max_row, int max_col )
{
    row = ( row + max_row ) % max_row;
    col = ( col + max_col ) % max_col;
}


inline Location offset( const Location& loc, const int direction[ 2 ] )
{
    Location temp( loc.row, loc.col );
    temp.offset( direction );
    return temp;
}


inline Location wrap( const Location& loc, int max_row, int max_col )
{
    Location temp( loc.row, loc.col );
    temp.wrap( max_row, max_col );
    return temp;
}


#endif //LOCATION_H_
