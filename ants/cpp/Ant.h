
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
        NONE=0,
        EXPLORE,
        DEFEND,
        ATTACK,
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
      assignment( NONE ),
      goal( location )
{
}



inline std::string assignmentString( Ant::Assignment assignment )
{
    static const char* assign2string [ Ant::NUM_ASSIGNMENTS ] = 
    {
        "NONE",
        "EXPLORE",
        "DEFEND",
        "ATTACK"
    };

    return assign2string[ static_cast<int>( assignment ) ];
}


inline std::ostream& operator<<( std::ostream& out, const Ant& ant )
{
    out << ant.location << ": " << ant.path;
    return out;
}

#endif // ANT_H_
