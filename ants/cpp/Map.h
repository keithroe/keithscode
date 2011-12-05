
#ifndef MAP_H_
#define MAP_H_

#include "Direction.h"
#include "Square.h"
#include "Location.h"

#include <cassert>
#include <ostream>
#include <vector>
#include <string>



class Map
{
public:
    enum PriorityType
    {
        EXPLORE=0,         // non-visible squares, frontier, other ants(negative)
        ATTACK,            // enemy hills, battle hot spots ( distance based )
        DEFENSE,           // ally hills, battle hot spots ( distance based )
        NUM_PRIORITY_TYPES
    };

    Map();
    Map( unsigned height, unsigned width);

    ~Map();

    void resize( unsigned height, unsigned width);

    // Reset all squares to empty state.  Leave water as is
    void reset();

    unsigned width()const  { return m_width;  }
    unsigned height()const { return m_height; }

    // Get location arrived at by moving in a given direction with edge wrapping
    Location  getLocation( const Location &loc, Direction direction )const;
    Direction getDirection( const Location &loc0, const Location& loc1 )const;

    void getNeighbors( const Location& loc, std::vector<Location>& neighbors )const;
    

    template<class Predicate>
    void getNeighbors( const Location& loc, Predicate pred, std::vector<Location>& neighbors )const;

    int   manhattanDistance( const Location& loc0, const Location& loc1 )const;
    float distance         ( const Location& loc0, const Location& loc1 )const;
    int   distance2        ( const Location& loc0, const Location& loc1 )const;

    void makeMove( const Location &loc, Direction direction );
    void makeMove( const Location &loc0, const Location& loc1 );


    void setPriority( PriorityType type, const Location& loc, float priority )
    { rangeCheck( loc.row, loc.col ); m_priorities[ type ][ loc.row ][ loc.col ] = priority; }
    
    float getPriority( PriorityType type, const Location& loc )const
    { rangeCheck( loc.row, loc.col ); return m_priorities[ type ][ loc.row ][ loc.col ]; }

    void updatePriority( PriorityType type, float amount, SquarePredicate pred );
    void diffusePriority( PriorityType type, unsigned iterations );

    void setDistanceTarget( PriorityType type, const Location& loc, int max_depth );
    void computeDistanceMap( PriorityType type );

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


    Location computeCentroid( const std::vector<Location>& location )const;
    
    static std::string priorityTypeString( PriorityType type );

    friend std::ostream& operator<<(std::ostream& os, const Map& map);

private:
    void getDxDy( const Location& loc0, const Location& loc1, int& dx, int& dy )const;

    unsigned m_height;
    unsigned m_width;
    Square** m_grid;

    float**  m_priorities[ NUM_PRIORITY_TYPES ];
    float**  m_scratch;

    typedef std::vector< std::pair<Location, int> > DistanceTargets;
    DistanceTargets m_attack_targets;

};
    

template<class Predicate>
void Map::getNeighbors( const Location& loc, Predicate predicate, std::vector<Location>& neighbors )const
{
    Location nloc = getLocation( loc, NORTH );
    if( predicate( m_grid[ nloc.row ][ nloc.col ] ) ) neighbors.push_back( nloc );
    
    nloc = getLocation( loc, EAST );
    if( predicate( m_grid[ nloc.row ][ nloc.col ] ) ) neighbors.push_back( nloc );

    nloc = getLocation( loc, SOUTH );
    if( predicate( m_grid[ nloc.row ][ nloc.col ] ) ) neighbors.push_back( nloc );

    nloc = getLocation( loc, WEST );
    if( predicate( m_grid[ nloc.row ][ nloc.col ] ) ) neighbors.push_back( nloc );
}

#endif // MAP_H_
