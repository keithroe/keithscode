
#include "Location.h"


std::ostream& operator<<( std::ostream& out, const Location& loc ) 
{
  out << "[" << loc.row << "," << loc.col << "]";
  return out;
}


