
#include "BFS.h"
#include "Debug.h"
#include "Map.h"
#include "Path.h"

#include <algorithm>


BFS::BFS( const Map& map, const Location& start_loc, Predicate predicate )
  : m_map( map ),
    m_predicate( predicate ),
    m_max_depth( 25u )
{
    m_open.push_back( new Node( start_loc ) );
}


bool BFS::search()
{
    //Debug::stream() << "BFS searching from locations  ..." << std::endl;
    //for( NodeQueue::iterator it = m_open.begin(); it != m_open.end(); ++it )
    //    Debug::stream() <<  "    " << (*it)->loc << std::endl;

    while( !m_open.empty() )
    {
        if( step() ) return true;
    }

    return false;
}


bool BFS::step()
{
    Node* current =  m_open.front();

    //Debug::stream() << "  Checking " << current->loc << std::endl;
    //
    // Check to see if we have reached our goal
    //
    if( m_predicate( m_map( current->loc ) ) )
    {
        m_destination = current->loc;
        // Backtrack to create path
        m_path.clear();
        while( current->child != 0 ) 
        {
            m_path.push_back( current->dir );
            current = current->child;
        }
        m_origin = current->child ? current->child->loc : current->loc;

        // Clean up
        for( NodeQueue::const_iterator it = m_open.begin(); it != m_open.end(); ++it )
            delete *it;
        m_open.clear();

        for( LocationToNode::const_iterator it = m_closed.begin(); it != m_closed.end(); ++it )
            delete it->second;
        m_closed.clear();

        // Indicate search completion
        return true;
    }

    //
    // Move current from open to closed list 
    //
    m_open.pop_front();
    m_closed.insert( std::make_pair( current->loc, current ) );

    //
    // Process all neighbors
    //
    if( current->depth < m_max_depth )
    {
        for( int i = 0; i < NUM_DIRECTIONS; ++i )
        {
            const Location neighbor_loc = m_map.getLocation( current->loc, static_cast<Direction>( i ) );
            
            /*
            // Check if the neighbor is available to be traversed
            if( !m_map( neighbor_loc ).isAvailable() ) 
            */

            if( ( current->depth == 0 && !m_map( neighbor_loc ).isAvailable() ) ||
                m_map( neighbor_loc ).isWater() )                    
                
            {
                //Debug::stream() << "  not avail " << std::endl;
                continue;
            }

            // Check if neighbor is already in open set
            if( std::find_if( m_open.begin(), m_open.end(), HasLocation( neighbor_loc ) ) != m_open.end() )
            {
                //Debug::stream() << "  in open set" << std::endl;
                continue;
            }

            // Check to see if this neighbor is in closed set 
            if( m_closed.find( neighbor_loc ) != m_closed.end() )
            {
                //Debug::stream() << "  in closed set" << std::endl;
                continue;
            }

            // Insert this location into open set
            //Debug::stream() << "   pushing " << std::endl;
            Node* neighbor_node = new Node( neighbor_loc,
                                            static_cast<Direction>( i ),
                                            current->depth+1,
                                            current );
            m_open.push_back( neighbor_node  );
        }
    }

    //
    // Indicate the search is not finished
    //
    return false;

}


void BFS::getPath( Path& path )const
{
    path.assign( m_destination, m_path.begin(), m_path.end() ); 
}


void BFS::getReversePath( Path& path )const
{
    std::deque<Direction> rpath;
    std::transform( m_path.begin(), m_path.end(), 
                    std::front_insert_iterator<std::deque<Direction> >( rpath ),
                    reverseDirection );
    path.assign( m_origin, rpath.begin(), rpath.end() ); 
}

