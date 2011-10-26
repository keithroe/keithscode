
#include "Path.h"
#include "PathFinder.h"

PathFinder::PathFinder( const Map& map )
    : m_map( map )
{
}


Path PathFinder::getPath( const Location& origin,
                          const Location& destination )const
{
    // TODO: For now just minimize manhattan distance.  Switch to A* soon
    float dx = static_cast<float>( destination.col - origin.col );
    float dy = static_cast<float>( destination.row - origin.row );

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
}
