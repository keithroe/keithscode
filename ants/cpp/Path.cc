
#include "Path.h"


Direction Path::nextStep()
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
