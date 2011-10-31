
#ifndef ANT_H_
#define ANT_H_

#include "Path.h"
#include "Location.h"

struct Ant
{
    Ant( const Location& location )
        : location( location ) {}
    Location location;
    Path     path;
};

#endif // ANT_H_
