
#include "Map.h"
#include "Location.h"
#include <cstdlib>
#include <iomanip>
#include <ostream>
#include <cmath>
#include <cstring>
    
Map::Map()
    : m_height( 0u ),
      m_width( 0u  ),
      m_grid( 0u ),
      m_priorities0( 0u ),
      m_priorities1( 0u )
{
}


Map::Map( unsigned height, unsigned width)
    : m_height( height ),
      m_width( width )
{
    // TODO: should probably make these by flat arrays that are tiled for memory coherency
    m_grid        = new Square*[ m_height ];
    m_priorities0 = new float*[ m_height ];
    m_priorities1 = new float*[ m_height ];
    for( unsigned int i = 0; i < m_height; ++i )
    {
          m_grid[ i ]        = new Square[ m_width ];
          m_priorities0[ i ] = new float[ m_width ];
          m_priorities1[ i ] = new float[ m_width ];
          memset( m_priorities0[ i ], 0, m_width*sizeof( float ) );
    }

}


void Map::resize( unsigned height, unsigned width )
{

    if( m_grid )
    {
        for( unsigned int i = 0; i < m_height; ++i )
        {
            delete [] m_grid[i];
            delete [] m_priorities0;
            delete [] m_priorities1;
        }
        delete m_grid;
        delete m_priorities0;
        delete m_priorities1;
    }

    m_height      = height;
    m_width       = width;
    m_grid        = new Square*[ m_height ];
    m_priorities0 = new float*[ m_height ];
    m_priorities1 = new float*[ m_height ];
    for( unsigned int i = 0; i < m_height; ++i )
    {
          m_grid[ i ]        = new Square[ m_width ];
          m_priorities0[ i ] = new float[ m_width ];
          m_priorities1[ i ] = new float[ m_width ];
          memset( m_priorities0[ i ], 0, m_width*sizeof( float ) );
    }
}



Map::~Map()
{
    for( unsigned i = 0; i < m_height; ++i )
    {
        delete [] m_grid[i];
        delete [] m_priorities0[i];
        delete [] m_priorities1[i];
    }
    delete m_grid;
    delete m_priorities0;
    delete m_priorities1;

    m_grid = 0u;
    m_priorities0 = m_priorities1 = 0u;
}


void Map::reset()
{
    for( unsigned i = 0u; i < m_height; ++i )
        for( unsigned j = 0u; j < m_width; ++j )
        {
            m_grid[ i ][ j ].reset();
            m_priorities0[ i ][ j ] = 0u;
        }

}


Location Map::getLocation( const Location &loc, Direction direction )const
{
    return wrap( offset( loc, DIRECTION_OFFSET[direction] ), m_height, m_width );
}
   
    
Direction Map::getDirection( const Location &loc0, const Location& loc1 )const
{
    int dx = loc1.col - loc0.col;
    if( dx ==  1 || dx + m_width == 1 ) return EAST;
    if( dx == -1 || dx - m_width == 1 ) return WEST;

    int dy = loc1.row - loc0.row; 
    if( dy ==  1 || dy + m_height == 1 ) return SOUTH;
    return NORTH;
}


void Map::getNeighbors( const Location& loc, std::vector<Location>& neighbors )const
{
    neighbors.push_back( getLocation( loc, NORTH ) );
    neighbors.push_back( getLocation( loc, EAST  ) );
    neighbors.push_back( getLocation( loc, SOUTH ) );
    neighbors.push_back( getLocation( loc, WEST  ) );
}


void Map::getDxDy( const Location& loc0, const Location& loc1, int& dx, int& dy )const
{

    int direct_dist_x = abs( loc0.col - loc1.col );
    int direct_dist_y = abs( loc0.row - loc1.row );

    int wrap_dist_x = m_width  - direct_dist_x;
    int wrap_dist_y = m_height - direct_dist_y;

    dx = std::min( direct_dist_x, wrap_dist_x );
    dy = std::min( direct_dist_y, wrap_dist_y );
}


int Map::manhattanDistance( const Location& loc0, const Location& loc1 )const
{
    int dx, dy;
    getDxDy( loc0, loc1, dx, dy );
    return dx + dy; 
}


float Map::distance( const Location& loc0, const Location& loc1 )const
{
    int dx, dy;
    getDxDy( loc0, loc1, dx, dy );

    return sqrtf( static_cast<float>( dx*dx ) + static_cast<float>( dy*dy ) );
}


void Map::makeMove( const Location &loc, Direction direction )
{
    Location new_loc = getLocation( loc, direction );
    m_grid[ new_loc.row ][ new_loc.col ].ant_id = m_grid[loc.row][loc.col].ant_id;
    m_grid[ new_loc.row ][ new_loc.col ].ant    = m_grid[loc.row][loc.col].ant;
    m_grid[ loc.row     ][ loc.col     ].ant_id = -1;
    m_grid[ loc.row     ][ loc.col     ].ant    = NULL;
}


void Map::makeMove( const Location &loc0, const Location& loc1 )
{
    m_grid[ loc1.row ][ loc1.col ].ant_id = m_grid[ loc0.row ][ loc0.col ].ant_id;
    m_grid[ loc1.row ][ loc1.col ].ant    = m_grid[ loc0.row ][ loc0.col ].ant;
    m_grid[ loc0.row ][ loc0.col ].ant_id = -1;
    m_grid[ loc0.row ][ loc0.col ].ant    = NULL;
}


void Map::diffusePriority( unsigned iterations )
{
    for( unsigned int k = 0; k < iterations; ++k )
    {
        for( unsigned i = 0u; i < m_height; ++i )
        {
            for( unsigned j = 0u; j < m_width; ++j )
            {
                // TODO: can optimize this.  Create ring buffer around priorities so no wrapping necessary;
                Location loc( i, j );
                const Square& sqr = m_grid[ i ][ j ];
                if( sqr.isUnknown() || sqr.isWater() ) continue;
                float val = m_priorities0[ i ][ j ];
                Location n = getLocation( loc, NORTH ); 
                Location s = getLocation( loc, SOUTH ); 
                Location e = getLocation( loc, EAST  ); 
                Location w = getLocation( loc, WEST  ); 
                float val_n = m_grid[ n.row ][ n.col ].isWater() ? val : m_priorities0[ n.row ][ n.col ];
                float val_s = m_grid[ s.row ][ s.col ].isWater() ? val : m_priorities0[ s.row ][ s.col ]; 
                float val_e = m_grid[ e.row ][ e.col ].isWater() ? val : m_priorities0[ e.row ][ e.col ];
                float val_w = m_grid[ w.row ][ w.col ].isWater() ? val : m_priorities0[ w.row ][ w.col ];
                m_priorities1[ i ][ j ] = 0.2f * ( m_priorities0[ i ][ j ] + val_n + val_s + val_e + val_w ); 
            }
        }
        float** temp = m_priorities0;
        m_priorities0 = m_priorities1;
        m_priorities1 = temp;
    }
}


std::ostream& operator<<( std::ostream &os, const Map& map )
{
    os << "------------------------------------------------------------------" << std::endl;
    for( unsigned i = 0u; i < map.m_height; ++i )
    {
        for( unsigned j = 0u; j < map.m_width; ++j )
        {
            const Square& square = map.m_grid[ i ][ j ];
            os << ' ';
            if     ( square.food         ) os << 'f';
            else if( square.ant_id >=0   ) os << static_cast<char>( 'a' + square.ant_id );
            else if( square.hill_id >= 0 ) os << static_cast<char>( 'A' + square.hill_id );
            else if( square.isWater()    ) os << 'w';
            else if( square.isUnknown()  ) os << '?';
            else if( square.isLand()     ) os << (square.visible ? ' ' : '.');
        }
        os << std::endl;
    }
    os << "------------------------------------------------------------------" << std::endl;

    for( unsigned i = 0u; i < map.m_height; ++i )
    {
        for( unsigned j = 0u; j < map.m_width; ++j )
        {
            const Square& square = map.m_grid[ i ][ j ];
            os << ' ';
            os << std::setw( 8 ) << std::setprecision( 2 ) << map.m_priorities0[ i ][ j ]; 
            if     ( square.food         ) os << 'f';
            else if( square.ant_id >=0   ) os << static_cast<char>( 'a' + square.ant_id );
            else if( square.hill_id >= 0 ) os << static_cast<char>( 'A' + square.hill_id );
            else if( square.isWater()    ) os << 'w';
            else if( square.isUnknown()  ) os << '?';
            else if( square.isLand()     ) os << (square.visible ? ' ' : '.');
        }
        os << std::endl;
    }

    /*
    
    for( unsigned i = 0u; i < map.m_height; ++i )
    {
        for( unsigned j = 0u; j < map.m_width; ++j )
        {
            os << " " << map.m_grid[ i ][ j ].priority; 
        }
        os << std::endl;
    }

    os << "------------------------------------------------------------------" << std::endl;
    */
    return os;
}


