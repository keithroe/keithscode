
#include "Path.h"
#include "Debug.h"
#include "Map.h"



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


void Path::visualize( const Location& start, const Map& map )const
{
#ifdef VISUALIZER
    if( m_steps.empty() ) return;
    
    setLineWidth( 2 );
    switch( m_goal )
    {
        case ATTACK:  setColor( 255,  80,  80, 0.5f ); break;
        case HILL:    setColor( 255, 255,  80, 0.5f ); break;
        case FOOD:    setColor(  80, 255,  80, 0.5f ); break;
        case EXPLORE: setColor(  80,  80, 255, 0.5f ); break;
        case OTHER:   setColor(  80, 255, 255, 0.5f ); break;
    }

    Location cur_location = start;
    for( std::list<Direction>::const_iterator it = m_steps.begin(); it != m_steps.end(); ++it )
    {
        Location next_location = map.getLocation( cur_location, *it );
        int d = abs( next_location.row - cur_location.row + next_location.col - cur_location.col );
        if(  d == 1 )
            line( cur_location, next_location );
        cur_location = next_location;
    }
#endif
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
