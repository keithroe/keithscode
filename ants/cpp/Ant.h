
#ifndef ANT_H_
#define ANT_H_

#include "Path.h"
#include "Location.h"

#include <ostream>

class Ant
{
public:
    Ant( const Location& location )
        : location( location ) {}

    bool     available()const;

    Location location;
    Path     path;

private:
    Ant& operator=( Ant& );
    Ant( Ant& );
};


inline bool Ant::available()const
{
    return path.empty();
}

inline std::ostream& operator<<( std::ostream& out, const Ant& ant )
{
    out << ant.location << ": " << ant.path;
    return out;
}

#endif // ANT_H_
