
#ifndef MAP_H_
#define MAP_H_

#include "Direction.h"
#include "Square.h"
#include "Location.h"
#include <cassert>



class Map
{
public:
    Map();
    Map( unsigned height, unsigned width);

    ~Map();

    void resize( unsigned height, unsigned width);

    // Reset all squares to empty state.  Leave water as is
    void reset();

    // Get the location arrived at by moving in a given direction with edge wrapping
    Location getLocation( const Location &loc, Direction direction )const;

    int   manhattanDistance( const Location& loc0, const Location& loc1 )const;
    float distance         ( const Location& loc0, const Location& loc1 )const;

    void prioritize();

    void makeMove( const Location &loc, Direction direction );

    Square& operator()( const Location& loc )
    { rangeCheck( loc.row, loc.col ); return m_grid[loc.row][loc.col]; }

    const Square& operator()( const Location& loc )const
    { rangeCheck( loc.row, loc.col ); return m_grid[loc.row][loc.col]; }

    Square& operator()( unsigned row, unsigned col )
    { rangeCheck( row, col ); return m_grid[row][col]; }

    const Square& operator()( unsigned row, unsigned col )const
    { rangeCheck( row, col ); return m_grid[row][col]; }

    void rangeCheck( unsigned row, unsigned col )const
    { assert( row < m_height &&  col < m_width ); }

    
    friend std::ostream& operator<<(std::ostream &os, const Map& map);

private:
    void getDxDy( const Location& loc0, const Location& loc1, int& dx, int& dy )const;

    unsigned m_height;
    unsigned m_width;
    Square** m_grid;
};

#endif // MAP_H_
