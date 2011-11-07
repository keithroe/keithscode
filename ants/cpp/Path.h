
#ifndef PATH_H_
#define PATH_H_

#include "Direction.h"
#include "Location.h"

#include <list>

class Path
{
public:
    enum Goal
    {
        HILL=0,
        FOOD, 
        EXPLORE,
        OTHER 
    };

    //Path() : m_goal( OTHER ) {}
    Path()  {}

    template <class Iter>
    Path( const Location& destination, Iter begins, Iter ends );

    template <class Iter>
    void assign( const Location& destination, Iter begins, Iter ends );

    Location  destination()const        { return m_destination;     }
    unsigned  size()const               { return m_steps.size();    }
    unsigned  empty()const              { return m_steps.empty();   }
    Direction nextStep()const;

    Goal      goal()const               { return m_goal;            }
    void      setGoal( Goal goal )      { m_goal = goal;            }
    Direction popNextStep();

    void      reset()                   { m_steps.clear();          }

    friend std::ostream& operator<<( std::ostream& out, const Path& path );

private:
    Location               m_destination;
    Goal                   m_goal;
    std::list<Direction>   m_steps;

};
    

template <class Iter>
Path::Path( const Location& destination, Iter begin, Iter end )
    : m_destination( destination ),
      m_goal( OTHER )
{
    m_steps.assign( begin, end );
}


template <class Iter>
void Path::assign( const Location& destination, Iter begin, Iter end )
{
    m_destination = destination;
    m_goal        = OTHER;
    m_steps.assign( begin, end );
}

#endif // PATH_H_
