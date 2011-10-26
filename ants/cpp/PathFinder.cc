
#include "AStar.h"
#include "Path.h"
#include "PathFinder.h"

PathFinder::PathFinder( const Map& map )
    : m_map( map )
{
}


void PathFinder::getPath( const Location& origin,
                          const Location& destination,
                          Path& path )const
{
    AStar astar( m_map, origin, destination ); 
    astar.getPath( path );
    
    /*
    
    for( int d = 0; d < NUM_DIRECTIONS; ++d )
    {
        Location loc = m_map.getLocation( origin, static_cast<Direction>( d ) );

        if( m_map( loc.row, loc.col ).isAvailable() )
        {
            // TODO: add ability to not move. probably need to add a 5th direction NOOP
            std::cerr << " creating d: " << (int)d << std::endl;
            return  Path( origin, destination, static_cast<Direction>( d ), 10 ); 
        }
    }
    return  Path( origin, destination, NONE, 10 ); 
    */
}
