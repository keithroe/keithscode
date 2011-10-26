
#ifndef PATH_H_
#define PATH_H_

#include "Location.h"
#include "State.h"

class Path
{
public:
    Path( const Location& origin, const Location& destination, Direction next_step, int distance );

    const Location origin()             { return m_origin;          }
    const Location destination()        { return m_destination;     }
    const Direction nextStep()          { return m_next_step;       }
    int distance()                      { return m_distance;        }

private:
    Location  m_origin;
    Location  m_destination;
    Direction m_next_step;
    int       m_distance;
};


#endif // PATH_H_
