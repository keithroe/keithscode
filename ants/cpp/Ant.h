
#ifndef ANT_H_
#define ANT_H_

#include "Path.h"
#include "Location.h"

#include <string>
#include <ostream>

class Ant
{
public:
    enum Assignment
    {
        EXPLORE=0,
        ATTACK,
        DEFENSE,
        STATIC_DEFENSE,
        NONE,
        NUM_ASSIGNMENTS
    };

    Ant( const Location& location );

    Location   location;   //< Current location of ant
    Assignment assignment; //< What is this ants job
    Location   goal;       //< Destination for this ant
    Path       path;       //< Path to destination

private:
    Ant& operator=( Ant& );
    Ant( Ant& );
};


inline Ant::Ant( const Location& location )
    : location( location ),
      assignment( EXPLORE ),
      goal( location )
{
}



inline std::string assignmentString( Ant::Assignment assignment )
{
    static const char* assign2string [ Ant::NUM_ASSIGNMENTS ] = 
    {
        "EXPLORE",
        "ATTACK",
        "DEFENSE",
        "STATIC_DEFENSE",
        "NONE"
    };

    return assign2string[ static_cast<int>( assignment ) ];
}


inline std::ostream& operator<<( std::ostream& out, const Ant& ant )
{
    out << ant.location << ": " << assignmentString( ant.assignment ) << " path: " << ant.path;
    return out;
}

#endif // ANT_H_
