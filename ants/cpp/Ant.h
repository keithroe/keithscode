
#ifndef ANT_H_
#define ANT_H_

#include "Path.h"
#include "Location.h"

struct Ant
{
    Ant( const Location& location )
        : location( location ) {}

    bool     available()const;

    Location location;
    Location goal;
    Path     path;

};


inline bool Ant::available()const
{
    return path.empty();
}

#endif // ANT_H_
