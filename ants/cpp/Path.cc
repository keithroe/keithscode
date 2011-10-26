
#include "Path.h"



Direction Path::nextStep()
{ 
    Direction dir = m_steps.front();
    m_steps.pop_front();
    return dir;     
}
