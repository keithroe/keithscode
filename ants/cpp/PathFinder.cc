
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
    astar.search();
    astar.getPath( path );
}
