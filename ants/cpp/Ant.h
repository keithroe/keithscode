
#ifndef ANT_H_
#define ANT_H_

#include "Path.h"
#include "Location.h"

struct Ant
{
    Ant( const Location& location )
        : location( location ), available( true ) {}

    Location location;
    Path     path;
    bool     available;
};


inline void resetAvailable( Ant* ant )
{
    ant->available = true;
}

#endif // ANT_H_
