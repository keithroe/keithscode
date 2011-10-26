
#include "Map.h"
#include "Location.h"
#include <cstdlib>
    
Map::Map()
    : m_height( 0u ),
      m_width( 0u  ),
      m_grid( 0u )
{
}

Map::Map( unsigned height, unsigned width)
    : m_height( height ),
      m_width( width )
{
    m_grid = new Square*[ m_height ];
    for( int i = 0; i < m_height; ++i )
        m_grid[ i ] = new Square[ m_width ];
}


void Map::resize( unsigned height, unsigned width )
{
    if( m_grid )
    {
        for( int i = 0; i < m_height; ++i )
            delete [] m_grid[i];
        delete m_grid;
    }

    m_height = height;
    m_width  = width;
    m_grid = new Square*[ m_height ];
    for( int i = 0; i < m_height; ++i )
        m_grid[ i ] = new Square[ m_width ];
}



Map::~Map()
{
    for( int i = 0; i < m_height; ++i )
        delete [] m_grid[i];
    delete m_grid;
    m_grid = 0u;
}


void Map::reset()
{
    for( unsigned i = 0u; i < m_height; ++i )
        for( unsigned j = 0u; j < m_width; ++j )
            m_grid[ i ][ j ].reset();

}


Location Map::getLocation( const Location &loc, Direction direction )const
{
    return wrap( offset( loc, DIRECTION_OFFSET[direction] ), m_height, m_width );
}
    

int Map::manhattanDistance( const Location& loc0, const Location& loc1 )const
{

    int direct_dist_x = abs( loc0.col - loc1.col );
    int direct_dist_y = abs( loc0.row - loc1.row );

    int wrap_dist_x = m_width  - direct_dist_x;
    int wrap_dist_y = m_height - direct_dist_y;

    return std::min( direct_dist_x, wrap_dist_x ) + std::min( direct_dist_y, wrap_dist_y );
}


void Map::makeMove( const Location &loc, Direction direction )
{
    Location new_loc = getLocation( loc, direction );
    m_grid[ new_loc.row ][ new_loc.col ].newAnt = m_grid[loc.row][loc.col].ant;
}

