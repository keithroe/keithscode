
#include "Debug.h"
#include "Location.h"
#include "Map.h"
#include "BF.h"

#include <cmath>
#include <cstdlib>
#include <cstring>
#include <iomanip>
#include <ostream>
    

namespace
{
    
    struct CompareMaxDist
    {
        bool operator()( const std::pair<Location, int>& t0, const std::pair<Location, int>& t1 ) 
        {
            return t0.second != 0 && t0.second < t1.second;
        }
    };


    struct MarkDistance 
    {
        MarkDistance( float** map ) : map( map ) {}

        bool operator()( const BFNode* node )
        {
            Location node_loc = node->loc;
            if( map[ node_loc.row ][ node_loc.col ] == 0.0f ) 
                map[ node_loc.row ][ node_loc.col ] = node->depth;

            return true;
        }

        float** map;
    };
    

    struct WithinDistance 
    {
        WithinDistance( unsigned distance ) : distance( distance ) {}

        bool operator()( const BFNode* current, const Location& neighbor_location, const Square& neighbor_square )
        {
            return ( distance == 0 || current->depth+1 <= distance ) && neighbor_square.isLand();
        }

        const unsigned distance;
    };

    typedef BF<MarkDistance, WithinDistance> ComputeDistance;
}


Map::Map()
    : m_height( 0u ),
      m_width( 0u  ),
      m_grid( 0u ),
      m_scratch( 0u )
{
    for( int j = 0; j < NUM_PRIORITY_TYPES; ++j )
        m_priorities[ j ] = 0u;
}


Map::Map( unsigned height, unsigned width)
    : m_height( height ),
      m_width( width ),
      m_grid( 0u )
{
    resize( m_height, m_width );
}


void Map::resize( unsigned height, unsigned width )
{

    if( m_grid )
    {
        for( unsigned int i = 0; i < m_height; ++i )
        {
            delete [] m_grid[i];
            delete [] m_scratch[i];
            for( int j = 0; j < NUM_PRIORITY_TYPES; ++j )
                delete [] m_priorities[j][i];
        }
        delete m_grid;
        delete m_scratch;
        for( int j = 0; j < NUM_PRIORITY_TYPES; ++j )
            delete m_priorities[ j ];
    }

    m_height = height;
    m_width  = width;
    
    m_grid        = new Square*[ m_height ];
    m_scratch     = new float*[ m_height ];
    for( int j = 0; j < NUM_PRIORITY_TYPES; ++j )
        m_priorities[ j ]  = new float*[ m_height ];

    for( unsigned int i = 0; i < m_height; ++i )
    {
          m_grid[ i ]       = new Square[ m_width ];
          m_scratch[ i ]    = new float[ m_width ];
          memset( m_scratch[ i ], 0, m_width*sizeof( float ) );

          for( int j = 0; j < NUM_PRIORITY_TYPES; ++j )
          {
              m_priorities[ j ][ i ] = new float[ m_width ];
              memset( m_priorities[ j ][ i ], 0, m_width*sizeof( float ) );
          }
    }
}



Map::~Map()
{
    for( unsigned i = 0; i < m_height; ++i )
    {
        delete [] m_grid[i];
        delete [] m_scratch[i];
        for( int j = 0; j < NUM_PRIORITY_TYPES; ++j )
            delete [] ( m_priorities[j][i] );
    }
    delete [] m_grid;
    delete [] m_scratch;
    for( int j = 0; j < NUM_PRIORITY_TYPES; ++j )
        delete [] m_priorities[ j ];

    m_grid = 0u;
    m_scratch = 0u;
    for( int j = 0; j < NUM_PRIORITY_TYPES; ++j )
        m_priorities[ j ] = 0u;
}


void Map::reset()
{
    for( unsigned i = 0u; i < m_height; ++i )
        for( unsigned j = 0u; j < m_width; ++j )
        {
            m_grid[ i ][ j ].reset();
            for( int p = 0; p < NUM_PRIORITY_TYPES; ++p )
                m_priorities[ p ][ i ][ j ] = 0u;
        }

    m_attack_targets.clear();
}


Location Map::getLocation( const Location &loc, Direction direction )const
{
    return clamp( offset( loc, DIRECTION_OFFSET[direction] ), m_height, m_width );
}
   
    
Direction Map::getDirection( const Location &loc0, const Location& loc1 )const
{
    if( loc0 == loc1 ) return NONE;

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


void Map::setDistanceTarget( PriorityType type, const Location& loc, int max_depth )
{ 
    rangeCheck( loc.row, loc.col );
    assert( type == ATTACK );
    m_attack_targets.push_back( std::make_pair( loc, max_depth ) );
}


void Map::computeDistanceMap( PriorityType type )
{
    assert( type == ATTACK );

    if( m_attack_targets.empty() ) return;

    // sort by max_distance
    std::sort( m_attack_targets.begin(), m_attack_targets.end(), CompareMaxDist() );

    for( DistanceTargets::iterator it = m_attack_targets.begin(); it != m_attack_targets.end(); ++it )
    {
        MarkDistance mark_distance( m_priorities[type] );
        WithinDistance within_distance( it->second );
        ComputeDistance compute_distance( *this, it->first, mark_distance, within_distance );
        compute_distance.setMaxDepth( 1000 );
        compute_distance.traverse();

    }
}


void Map::updatePriority( PriorityType type, float amount, SquarePredicate pred )
{
    assert( type == EXPLORE );

    for( unsigned int i = 0; i < m_height; ++i )
        for( unsigned int j = 0; j < m_width; ++j )
            if( pred( m_grid[i][j] ) ) m_priorities[ type ][i][j] += amount;
}


void Map::diffusePriority( PriorityType type, unsigned iterations )
{
    assert( type == EXPLORE );

    const unsigned w = m_width;
    const unsigned h = m_height;

    for( unsigned int k = 0; k < iterations; ++k )
    {
        float** priorities = m_priorities[ type ];

        // Diffuse the internal nodes first
        for( unsigned i = 1u; i < h-1; ++i )
        {
            for( unsigned j = 1u; j < w-1; ++j )
            {
                if( m_grid[ i ][ j ].isLand() )
                {
                    float sum       = priorities[ i ][ j ];
                    float num_nodes = 1.0f;
                    if( m_grid[ i+0 ][ j-1 ].isLand() ) { sum += priorities[ i+0 ][ j-1 ]; num_nodes += 1.0f; }
                    if( m_grid[ i+0 ][ j+1 ].isLand() ) { sum += priorities[ i+0 ][ j+1 ]; num_nodes += 1.0f; }
                    if( m_grid[ i+1 ][ j+0 ].isLand() ) { sum += priorities[ i+1 ][ j+0 ]; num_nodes += 1.0f; }
                    if( m_grid[ i-1 ][ j+0 ].isLand() ) { sum += priorities[ i-1 ][ j+0 ]; num_nodes += 1.0f; }
                    m_scratch[ i ][ j ] = sum / num_nodes; 
                }
            }
        }

        // Now do outside ring
        for( unsigned i = 1u; i < h-1; ++i )
        {
            if( m_grid[ i ][ 0 ].isLand() )
            {
                float sum         = priorities[ i ][ 0 ];
                float num_nodes   = 1.0f;
                if( m_grid[ i+0 ][ w-1 ].isLand() ) { sum += priorities[ i+0 ][ w-1 ]; num_nodes += 1.0f; }
                if( m_grid[ i+0 ][ 1   ].isLand() ) { sum += priorities[ i+0 ][ 1   ]; num_nodes += 1.0f; }
                if( m_grid[ i+1 ][ 0   ].isLand() ) { sum += priorities[ i+1 ][ 0   ]; num_nodes += 1.0f; }
                if( m_grid[ i-1 ][ 0   ].isLand() ) { sum += priorities[ i-1 ][ 0   ]; num_nodes += 1.0f; }
                m_scratch[ i ][ 0 ] = sum / num_nodes; 
            }
            
            if( m_grid[ i ][ w-1 ].isLand() )
            {
                float sum       = priorities[ i ][ w-1 ];
                float num_nodes = 1.0f;
                if( m_grid[ i+0 ][ w-2 ].isLand() ) { sum += priorities[ i+0 ][ w-2 ]; num_nodes += 1.0f; }
                if( m_grid[ i+0 ][ 0   ].isLand() ) { sum += priorities[ i+0 ][ 0   ]; num_nodes += 1.0f; }
                if( m_grid[ i+1 ][ w-1 ].isLand() ) { sum += priorities[ i+1 ][ w-1 ]; num_nodes += 1.0f; }
                if( m_grid[ i-1 ][ w-1 ].isLand() ) { sum += priorities[ i-1 ][ w-1 ]; num_nodes += 1.0f; }
                m_scratch[ i ][ w-1 ] = sum / num_nodes; 
            }

        }

        for( unsigned j = 1u; j < w-1; ++j )
        {
            if( m_grid[ 0 ][ j ].isLand() )
            {
                float sum         = priorities[ 0 ][ j ];
                float num_nodes   = 1.0f;
                if( m_grid[ 0   ][ j-1 ].isLand() ) { sum += priorities[ 0   ][ j-1 ]; num_nodes += 1.0f; }
                if( m_grid[ 0   ][ j+1 ].isLand() ) { sum += priorities[ 0   ][ j+1 ]; num_nodes += 1.0f; }
                if( m_grid[ h-1 ][ j+0 ].isLand() ) { sum += priorities[ h-1 ][ j+0 ]; num_nodes += 1.0f; }
                if( m_grid[ 1   ][ j+0 ].isLand() ) { sum += priorities[ 1   ][ j+0 ]; num_nodes += 1.0f; }
                m_scratch[ 0 ][ j ] = sum / num_nodes; 
            }
            
            if( m_grid[ h-1 ][ j ].isLand() )
            {
                float sum       = priorities[  h-1 ][ j ];
                float num_nodes = 1.0f;
                if( m_grid[ h-1 ][ j-1 ].isLand() ) { sum += priorities[ h-1 ][ j-1 ]; num_nodes += 1.0f; }
                if( m_grid[ h-1 ][ j+1 ].isLand() ) { sum += priorities[ h-1 ][ j+1 ]; num_nodes += 1.0f; }
                if( m_grid[ h-2 ][ j+0 ].isLand() ) { sum += priorities[ h-2 ][ j+0 ]; num_nodes += 1.0f; }
                if( m_grid[ 0   ][ j+0 ].isLand() ) { sum += priorities[ 0   ][ j+0 ]; num_nodes += 1.0f; }
                m_scratch[  h-1 ][ j ] = sum / num_nodes; 
            }
        }


        // four corners
        if( m_grid[ 0 ][ 0 ].isLand() )
        {
            float sum         = priorities[ 0 ][ 0 ];
            float num_nodes   = 1.0f;
            if( m_grid[ 0   ][ w-1 ].isLand() ) { sum += priorities[ 0   ][ w-1 ]; num_nodes += 1.0f; }
            if( m_grid[ 0   ][ 1   ].isLand() ) { sum += priorities[ 0   ][ 1   ]; num_nodes += 1.0f; }
            if( m_grid[ h-1 ][ 0   ].isLand() ) { sum += priorities[ h-1 ][ 0   ]; num_nodes += 1.0f; }
            if( m_grid[ 1   ][ 0   ].isLand() ) { sum += priorities[ 1   ][ 0   ]; num_nodes += 1.0f; }
            m_scratch[ 0 ][ 0 ] = sum / num_nodes; 
        }
        
        if( m_grid[ 0 ][ w-1 ].isLand() )
        {
            float sum         = priorities[ 0 ][ w-1 ];
            float num_nodes   = 1.0f;
            if( m_grid[ 0   ][ w-2 ].isLand() ) { sum += priorities[ 0   ][ w-2 ]; num_nodes += 1.0f; }
            if( m_grid[ 0   ][ 0   ].isLand() ) { sum += priorities[ 0   ][ 0   ]; num_nodes += 1.0f; }
            if( m_grid[ h-1 ][ w   ].isLand() ) { sum += priorities[ h-1 ][ w   ]; num_nodes += 1.0f; }
            if( m_grid[ 1   ][ w   ].isLand() ) { sum += priorities[ 1   ][ w   ]; num_nodes += 1.0f; }
            m_scratch[ 0 ][ w-1 ] = sum / num_nodes; 
        }

        if( m_grid[ h-1 ][ 0 ].isLand() )
        {
            float sum         = priorities[ h-1 ][ 0 ];
            float num_nodes   = 1.0f;
            if( m_grid[ h-1 ][ w-1 ].isLand() ) { sum += priorities[ h-1 ][ w-1 ]; num_nodes += 1.0f; }
            if( m_grid[ h-1 ][ 1   ].isLand() ) { sum += priorities[ h-1 ][ 1   ]; num_nodes += 1.0f; }
            if( m_grid[ h-2 ][ 0   ].isLand() ) { sum += priorities[ h-2 ][ 0   ]; num_nodes += 1.0f; }
            if( m_grid[ 0   ][ 0   ].isLand() ) { sum += priorities[ 0   ][ 0   ]; num_nodes += 1.0f; }
            m_scratch[ h-1 ][ 0 ] = sum / num_nodes; 
        }

        if( m_grid[ h-1 ][ w-1 ].isLand() )
        {
            float sum         = priorities[ h-1 ][ w-1 ];
            float num_nodes   = 1.0f;
            if( m_grid[ h-1 ][ w-2 ].isLand() ) { sum += priorities[ h-1 ][ w-2 ]; num_nodes += 1.0f; }
            if( m_grid[ h-1 ][ 0   ].isLand() ) { sum += priorities[ h-1 ][ 0   ]; num_nodes += 1.0f; }
            if( m_grid[ h-2 ][ w   ].isLand() ) { sum += priorities[ h-2 ][ w   ]; num_nodes += 1.0f; }
            if( m_grid[ 0   ][ w   ].isLand() ) { sum += priorities[ 0   ][ w   ]; num_nodes += 1.0f; }
            m_scratch[ h-1 ][ w-1 ] = sum / num_nodes; 
        }

        // Swap buffers
        m_priorities[ type ] = m_scratch;
        m_scratch            = priorities;
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
            if     ( square.food           ) os << 'f';
            else if( square.ant_id >=0     ) os << static_cast<char>( 'a' + square.ant_id );
            else if( square.new_ant_id >=0 ) os << static_cast<char>( 'a' + square.new_ant_id );
            else if( square.hill_id >= 0   ) os << static_cast<char>( 'A' + square.hill_id );
            else if( square.isWater()      ) os << 'w';
            else if( square.isUnknown()    ) os << '?';
            else if( square.isLand()       ) os << (square.visible ? ' ' : '.');
        }
        os << std::endl;
    }
    for( int p = 0; p < Map::NUM_PRIORITY_TYPES; ++p )
    {
        os << Map::priorityTypeString( static_cast<Map::PriorityType>( p ) ) << std::endl
           << "------------------------------------------------------------------" << std::endl;

        float** priorities = map.m_priorities[ p ];

        for( unsigned i = 0u; i < map.m_height; ++i )
        {
            for( unsigned j = 0u; j < map.m_width; ++j )
            {
                const Square& square = map.m_grid[ i ][ j ];
                os << ' ';
                os << std::fixed << std::setw( 8 ) << std::setprecision( 2 ) << priorities[ i ][ j ]; 
                if     ( square.food           ) os << 'f';
                else if( square.ant_id >=0     ) os << static_cast<char>( 'a' + square.ant_id );
                else if( square.new_ant_id >=0 ) os << static_cast<char>( 'a' + square.new_ant_id );
                else if( square.hill_id >= 0   ) os << static_cast<char>( 'A' + square.hill_id );
                else if( square.isWater()      ) os << 'w';
                else if( square.isUnknown()    ) os << '?';
                else if( square.isLand()       ) os << (square.visible ? ' ' : '.');
            }
            os << std::endl;
        }
    }

    return os;
}

std::string Map::priorityTypeString( PriorityType type )
{
    static const char* type2string[ NUM_PRIORITY_TYPES ] =
    {
        "EXPLORE",
        "ATTACK",
        "DEFENSE"
    };

    return type2string[ type ];
}

