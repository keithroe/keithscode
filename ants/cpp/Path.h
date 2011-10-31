
#ifndef PATH_H_
#define PATH_H_

#include "Direction.h"
#include "Location.h"

#include <list>

class Path
{
public:
    Path() {}

    template <class Iter>
    Path( const Location& origin, const Location& destination, Iter begins, Iter ends );

    template <class Iter>
    void assign( const Location& origin, const Location& destination, Iter begins, Iter ends );

    Location  origin()const             { return m_origin;          }
    Location  destination()const        { return m_destination;     }
    unsigned  size()const               { return m_steps.size();    }
    Direction nextStep();
    Direction popNextStep();

private:
    Location               m_origin;
    Location               m_destination;
    std::list<Direction>   m_steps;
};
    

template <class Iter>
Path::Path( const Location& origin, const Location& destination, Iter begins, Iter ends )
    : m_origin( origin ),
      m_destination( destination )
{
    m_steps.assign( begins, ends );
}

template <class Iter>
void Path::assign( const Location& origin, const Location& destination, Iter begins, Iter ends )
{
    m_origin      = origin;
    m_destination = destination;
    m_steps.assign( begins, ends );
}

#endif // PATH_H_
