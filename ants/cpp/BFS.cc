
#include "BFS.h"


BFS::BFS( const Map& map, const Location& start_loc, Predicate predicate )
  : m_map( map ),
    m_predicate( predicate ),
    m_max_depth( 20 )
{
    m_open.push_back( new Node( start_loc ) );
}

bool BFS::search()
{
    return false;
}


void BFS::getPath( Path& path )const
{
}


bool BFS::step()
{
  return false;
}

