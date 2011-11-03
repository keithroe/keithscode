
#include "AStar.h"
#include "Path.h"
#include "Debug.h"
#include "Map.h"

#include <algorithm>



AStar::AStar( const Map& map, const Location& start, const Location& destination )
    : m_map( map ),
      m_destination( destination ),
      m_max_depth( 20 )
{
    m_open.push_back( new Node( start, NONE, 0, m_map.manhattanDistance( start, destination ), 0 ) );
}


bool AStar::search()
{
    //Debug::stream() << " A* searching for " << m_open.front()->loc << " to " << m_destination << " ...." << std::endl;
    while( m_open.size() > 0 )
    {
        if( step() ) return true;
    }

    return false;
}


bool AStar::step()
{
    Node* current = m_open.front();

    //
    // Check to see if we have reached our destination
    //
    if( current->loc == m_destination )
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
    std::pop_heap( m_open.begin(), m_open.end(), NodeCompare() );
    m_open.pop_back();
    m_closed.insert( std::make_pair( current->loc, current ) );

    //
    // Process all neighbors
    //
    if( current->g+1 < m_max_depth )
    {
        for( int i = 0; i < NUM_DIRECTIONS; ++i )
        {
            Location neighbor_loc = m_map.getLocation( current->loc, static_cast<Direction>( i ) );

            // Check to see if this neighbor is traversable Do not rule out
            // temporarily unavailable nodes
            if( ( current->g == 0 && !m_map( neighbor_loc ).isAvailable() ) ||
                m_map( neighbor_loc ).isWater() )  
                continue;

            // Search through open list for this neighbor
            NodeVec::iterator it = std::find_if( m_open.begin(), m_open.end(), HasLocation( neighbor_loc ) );
            if( it != m_open.end() )
            {
                Node* open_node = *it;
                if( current->g+1 < open_node->g )
                {
                    // We have found a better path to this location.  update heap
                    open_node->g     = current->g+1;
                    open_node->child = current;
                    open_node->dir   = static_cast<Direction>( i );
                    std::make_heap( m_open.begin(), m_open.end(), NodeCompare() );
                }
                continue;
            }
            
            // Check to see if this neighbor is in closed set 
            if( m_closed.find( neighbor_loc ) != m_closed.end() )
                continue;

            // We need to add a new node to our open list
            Node* neighbor_node = new Node( neighbor_loc,
                                            static_cast<Direction>( i ),
                                            current->g+1,
                                            m_map.manhattanDistance( neighbor_loc, m_destination ),
                                            current ); 
            m_open.push_back( neighbor_node );
            std::push_heap( m_open.begin(), m_open.end(), NodeCompare() );
        }
    }

    //
    // Indicate the search is not finished
    //
    return false;
}


void AStar::getPath( Path& path )const
{
  
    path.assign( m_destination, m_path.begin(), m_path.end() ); 
}
