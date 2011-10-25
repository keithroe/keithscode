
#ifndef PATH_FINDER_H_
#define PATH_FINDER_H_

#include <State.h>

class Grid;
class Location;

class PathFinder
{
public:
    PathFinder( const Grid& grid );

    Direction getDirection( const Location& origin, const Location& destination )const;

private:
    const Grid& m_grid;
};

#endif // PATH_FINDER_H_
