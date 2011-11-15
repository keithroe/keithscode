
#include "Path.h"
#include "Debug.h"



void Path::assign( const Location& destination, Direction step, Goal goal )
{
    m_destination = destination;
    m_goal        = goal;
    m_steps.clear();
    m_steps.push_back( step );
}


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
    
    if( m_steps.empty() )
    {
        Debug::stream() <<"      resetting path to " << m_destination << std::endl;
        m_goal = OTHER;
    }
    return dir;     
}


std::ostream& operator<<( std::ostream& out, const Path& path )
{
    static const char* lookup[5] = { "ATTACK", "HILL", "FOOD", "EXPLORE", "OTHER" };
    out << path.m_destination << " - " << lookup[ path.m_goal ] << ": ";
    for( std::list<Direction>::const_iterator it = path.m_steps.begin();
         it != path.m_steps.end();
         ++it )
        out << DIRECTION_CHAR[ *it ] << " ";

    return out;
}
