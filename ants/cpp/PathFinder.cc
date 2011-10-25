
#include <PathFinder.h>

PathFinder::PathFinder( const Grid& grid )
    : m_grid( grid )
{
}


Direction PathFinder::getDirection( const Location& origin,
                              const Location& destination )const
{
    // TODO: For now just minimize manhattan distance.  Switch to A* soon
    return EAST;
}
