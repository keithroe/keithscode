
#ifndef PATH_H_
#define PATH_H_

#include "Direction.h"
#include "Location.h"

#include <list>

/// TODO: need a way to represent other types of 'paths' such as 'walk in this general direction'
class Path
{
public:
    Path() {}

    template <class Iter>
    Path( const Location& destination, Iter begins, Iter ends );

    template <class Iter>
    void assign( const Location& destination, Iter begins, Iter ends );

    Location  destination()const        { return m_destination;     }
    unsigned  size()const               { return m_steps.size();    }
    unsigned  empty()const              { return m_steps.empty();    }
    Direction nextStep()const;
    Direction popNextStep();

    void      reset()                   { m_steps.clear();          }

    friend std::ostream& operator<<( std::ostream& out, const Path& path );

private:
    Location               m_destination;
    std::list<Direction>   m_steps;
};
    

template <class Iter>
Path::Path( const Location& destination, Iter begins, Iter ends )
    : m_destination( destination )
{
    m_steps.assign( begins, ends );
}


template <class Iter>
void Path::assign( const Location& destination, Iter begins, Iter ends )
{
    m_destination = destination;
    m_steps.assign( begins, ends );
}

#endif // PATH_H_
