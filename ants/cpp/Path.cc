
#include "Path.h"


Path::Path( const Location& origin, const Location& destination, Direction next_step, int distance )
    : m_origin( origin ),
      m_destination( destination ),
      m_next_step( next_step ),
      m_distance( distance )
{
}

