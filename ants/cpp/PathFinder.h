
#ifndef PATH_FINDER_H_
#define PATH_FINDER_H_

#include "State.h"
#include "Location.h"

class Map;
class Path;

class PathFinder
{
public:
    PathFinder( const Map& map );

    Path getPath( const Location& origin, const Location& destination )const;
    
private:
    const Map& m_map;
};

#endif // PATH_FINDER_H_
