
#include "Map.h"
#include "Location.h"
#include <cstdlib>
#include <ostream>
#include <cmath>
    
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



std::ostream& operator<<( std::ostream &os, const Map& map )
{
    os << "------------------------------------------------------------------" << std::endl;
    for( unsigned i = 0u; i < map.m_height; ++i )
    {
        for( unsigned j = 0u; j < map.m_width; ++j )
        {
            const Square& square = map.m_grid[ i ][ j ];
            os << ' ';
            switch( square.content )
            {
                case Square::WATER:
                {
                    os << 'w';
                    break;
                }
                case Square::FOOD:
                {
                    os << 'f';
                    break;
                }
                case Square::HILL:
                {
                    os << static_cast<char>( 'A' + square.hill_id );
                    break;
                }
                case Square::EMPTY:
                {
                    if( square.ant_id >= 0 )
                    {
                        os << static_cast<char>( 'a' + square.ant_id );
                    }
                    else 
                    {
                      os << (square.isVisible ? ' ' : '.');
                    }
                    break;

                }
                case Square::UNKNOWN:
                {
                    os << '?';
                    break;
                }
            }
        }
        os << std::endl;
    }
    os << "------------------------------------------------------------------" << std::endl;
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
