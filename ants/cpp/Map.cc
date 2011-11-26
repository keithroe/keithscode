
#include "Debug.h"
#include "Location.h"
#include "Map.h"

#include <cmath>
#include <cstdlib>
#include <cstring>
#include <iomanip>
#include <ostream>
    
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
          memset( m_priorities1[ i ], 0, m_width*sizeof( float ) );
    }

}


void Map::resize( unsigned height, unsigned width )
{

    if( m_grid )
    {
        for( unsigned int i = 0; i < m_height; ++i )
        {
            delete [] m_grid[i];
            delete [] m_priorities0[i];
            delete [] m_priorities1[i];
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
          memset( m_priorities1[ i ], 0, m_width*sizeof( float ) );
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
    return clamp( offset( loc, DIRECTION_OFFSET[direction] ), m_height, m_width );
}
   
    
Direction Map::getDirection( const Location &loc0, const Location& loc1 )const
{
    int dx = loc1.col - loc0.col;
    //if( dx ==  1 || dx + m_width == 1 ) return EAST;
    //if( dx == -1 || dx - m_width == 1 ) return WEST;

    int dy = loc1.row - loc0.row; 
    //if( dy ==  1 || dy + m_height == 1 ) return SOUTH;
    //if( dy == -1 || dy - m_height == 1 ) return NORTH;

    int half_width  = m_width  / 2;
    if( dx >  half_width ) dx -= m_width;
    if( dx < -half_width ) dx += m_width;
    
    int half_height = m_height / 2;
    if( dy >  half_height ) dy -= m_height;
    if( dy < -half_height ) dy += m_height;

    Direction dir;
    if( abs( dx ) > abs( dy ) )
    {
        dir = dx > 0 ? EAST : WEST;
    }
    else
    {
        dir = dy > 0 ? SOUTH : NORTH;
    }

    Debug::stream() << "   getDirection( " << loc0 << "," << loc1 << " ): returning " << DIRECTION_CHAR[dir] << std::endl;
    return dir;
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

int Map::distance2( const Location& loc0, const Location& loc1 )const
{
    int dx, dy;
    getDxDy( loc0, loc1, dx, dy );

    return  dx*dx + dy*dy;
}


void Map::makeMove( const Location &loc, Direction direction )
{
    Location new_loc = getLocation( loc, direction );
    m_grid[ new_loc.row ][ new_loc.col ].new_ant_id = m_grid[loc.row][loc.col].ant_id;
    m_grid[ new_loc.row ][ new_loc.col ].new_ant    = m_grid[loc.row][loc.col].ant;
    m_grid[ loc.row     ][ loc.col     ].ant_id = -1;
    m_grid[ loc.row     ][ loc.col     ].ant    = NULL;
}


void Map::makeMove( const Location &loc0, const Location& loc1 )
{
    m_grid[ loc1.row ][ loc1.col ].new_ant_id = m_grid[ loc0.row ][ loc0.col ].ant_id;
    m_grid[ loc1.row ][ loc1.col ].new_ant    = m_grid[ loc0.row ][ loc0.col ].ant;
    m_grid[ loc0.row ][ loc0.col ].ant_id = -1;
    m_grid[ loc0.row ][ loc0.col ].ant    = NULL;
}


Location Map::computeCentroid( const std::vector<Location>& locations )const
{
    assert( !locations.empty() );
    float x = locations.begin()->col;
    float y = locations.begin()->row;
    const float half_width  = static_cast<float>( m_width  ) * 0.5f;
    const float half_height = static_cast<float>( m_height ) * 0.5f;

    float num_points = 1.0f;
    for( std::vector<Location>::const_iterator it = locations.begin()+1; it != locations.end(); ++it )
    {
        Location cur = *it;
        float dx = static_cast<float>( cur.col ) - x;
        if( dx >  half_width ) dx -= m_width;
        if( dx < -half_width ) dx += m_width;
        
        float dy = static_cast<float>( cur.row ) - y;
        if( dy >  half_height ) dy -= m_height;
        if( dy < -half_height ) dy += m_height;
        
        x = ( ( x + dx ) + num_points*x ) / ( num_points + 1.0f );
        y = ( ( y + dy ) + num_points*y ) / ( num_points + 1.0f );
        num_points += 1.0f;
    }
    if( x > (m_width-1)  ) x -= m_width;
    if( x < 0            ) x += m_width;
    if( y > (m_height-1) ) y -= m_height;
    if( y < 0            ) y += m_height;
    
    return Location( static_cast<int>( y+0.5f ), static_cast<int>( x+0.5f ) );
}

void Map::updatePriority( float amount, SquarePredicate pred )
{
    for( unsigned int i = 0; i < m_height; ++i )
        for( unsigned int j = 0; j < m_width; ++j )
            if( pred( m_grid[i][j] ) ) m_priorities0[i][j] += amount;
}


void Map::diffusePriority( unsigned iterations )
{
    for( unsigned int k = 0; k < iterations; ++k )
    {
        for( unsigned i = 0u; i < m_height; ++i )
        {
            for( unsigned j = 0u; j < m_width; ++j )
            {
                const Square& sqr = m_grid[ i ][ j ];
                if( !sqr.isLand() ) continue;
                float sum         = m_priorities0[ i ][ j ];
                float num_nodes   = 1.0f;
                for( int d = 0; d < 4; ++d )
                {
                    Location neighbor = getLocation( Location( i, j ), static_cast<Direction>( d ) ); 
                    if( m_grid[ neighbor.row ][ neighbor.col ].isLand() )
                    {
                        sum       += m_priorities0[ neighbor.row ][ neighbor.col ]; 
                        num_nodes += 1.0f;
                    }
                }
                m_priorities1[ i ][ j ] = 1.0f / num_nodes  * ( sum ); 
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
            os << std::fixed << std::setw( 8 ) << std::setprecision( 2 ) << map.m_priorities0[ i ][ j ]; 
            if     ( square.food         ) os << 'f';
            else if( square.ant_id >=0   ) os << static_cast<char>( 'a' + square.ant_id );
            else if( square.hill_id >= 0 ) os << static_cast<char>( 'A' + square.hill_id );
            else if( square.isWater()    ) os << 'w';
            else if( square.isUnknown()  ) os << '?';
            else if( square.isLand()     ) os << (square.visible ? ' ' : '.');
        }
        os << std::endl;
    }

    return os;
}


