
#include "BFS.h"
#include "Debug.h"


BFS::BFS( const Map& map, const Location& start_loc, Predicate predicate )
  : m_map( map ),
    m_predicate( predicate ),
    m_cur_depth( 0u ),
    m_max_depth( 20u )
{
    m_open.insert( std::make_pair( start_loc, new Node( start_loc ) ) );
}

bool BFS::search()
{
    Debug::stream() << "BFS searching from locations  ..." << std::endl;
    for( LocationToNode::iterator it = m_open.begin(); it != m_open.end(); ++it )
        Debug::stream() <<  "    " << it->first << std::endl;

    while( m_open.size() > 0 && m_cur_depth++ < m_max_depth )
    {
        if( step() ) return true;
    }

    return false;
}


void BFS::getPath( Path& path )const
{
    Node* current =  m_open.begin()->second;
    //Debug::stream() << "checking: " << current->loc << std::endl;

    //
    // Check to see if we have reached our goal
    //
    if( m_predicate( m_map( current->loc ) ) )
    {
        // Backtrack to create path
        m_path.clear();
        while( current->child != 0 ) 
        {
            m_path.push_back( current->dir );
            current = current->child;
        }
        m_origin = current->child ? current->child->loc : current->loc;

        std::reverse( m_path.begin(), m_path.end() );
        
        // Clean up
        for( NodeVec::const_iterator it = m_open.begin(); it != m_open.end(); ++it )
            delete *it;
        m_open.clear();

        for( LocationToNode::const_iterator it = m_closed.begin(); it != m_closed.end(); ++it )
            delete (it->second);
        m_closed.clear();

        // Indicate search completion
        return true;
    }

    //
    // Move current from open to closed list 
    //
    m_open.pop_back();
    m_closed.insert( std::make_pair( current->loc, current ) );

    //
    // Process all neighbors
    //
    for( int i = 0; i < NUM_DIRECTIONS; ++i )
    {
        Location neighbor_loc = m_map.getLocation( current->loc, static_cast<Direction>( i ) );

        // Check if the neighbor is available to be traversed
        if( !m_map( neighbor_loc ).isAvailable() ) 
            continue;

        // Check if neighbor is already in open set
        if( m_open.find( neighbor_loc ) != m_open.end() )
            continue;

        // Check to see if this neighbor is in closed set 
        if( m_closed.find( neighbor_loc ) != m_closed.end() )
            continue;

        //Debug::stream() << "     pushing " << neighbor_loc << std::endl;
        m_open.push_back( new Node( neighbor_loc, static_cast<Direction>( i ), current ) );
    }

    //
    // Indicate the search is not finished
    //
    return false;

}


bool BFS::step()
{
  return false;
}

