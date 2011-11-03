
#include "Path.h"


Direction Path::nextStep()const
{ 
    if( m_steps.empty() ) return NONE;
    return m_steps.front();
}

Direction Path::popNextStep()
{ 
    if( m_steps.empty() ) return NONE;

    Direction dir = m_steps.front();
    m_steps.pop_front();
    return dir;     
}


std::ostream& operator<<( std::ostream& out, const Path& path )
{
    out << path.m_destination << ": ";
    for( std::list<Direction>::const_iterator it = path.m_steps.begin();
         it != path.m_steps.end();
         ++it )
        out << DIRECTION_CHAR[ *it ] << " ";
}
