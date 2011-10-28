
#include "AStar.h"
#include "Path.h"
#include <algorithm>

AStar::AStar( const Map& map, const Location& goal, const Location& start )
    : m_map( map ),
      m_goal( goal ),
      m_cur_depth( 0 ),
      m_max_depth( 1000 )
{
    m_open.push_back( new Node( start, NONE, 0, m_map.manhattanDistance( start, goal ), 0 ) );
}


AStar::AStar( const Map& map, const Location& goal, const std::vector<Location>& starts )
    : m_map( map ),
      m_goal( goal ),
      m_cur_depth( 0 ),
      m_max_depth( 1000 )
{
    for( std::vector<Location>::const_iterator it = starts.begin(); it != starts.end(); ++it )
        m_open.push_back( new Node( *it, NONE, 0, m_map.manhattanDistance( *it, goal ), 0 ) );

    std::make_heap( m_open.begin(), m_open.end(), NodeCompare() );
}


bool AStar::search()
{

    std::cerr << " searching ...." << std::endl;

    while( m_open.size() > 0 )
    {
      std::cerr << "   stepping...." << std::endl;
        
        if( step() ) return true;
    }

    return false;
}


bool AStar::step()
{
    Node* current = m_open.front();

    //std::cerr << "checking: " << current->loc << std::endl;
    Location f;
    //std::cerr << current->loc;
    std::cerr << f; 
    //
    // Check to see if we have reached our goal
    //
    if( current->loc == m_goal )
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

        for( NodeVec::const_iterator it = m_closed.begin(); it != m_closed.end(); ++it )
            delete *it;
        m_closed.clear();

        // Indicate search completion
        return true;
    }

    //
    // Move current from open to closed list 
    //
    std::pop_heap( m_open.begin(), m_open.end(), NodeCompare() );
    m_open.pop_back();
    m_closed.push_back( current );

    //
    // Process all neighbors
    //
    for( int i = 0; i < NUM_DIRECTIONS; ++i )
    {
        Location neighbor_loc = m_map.getLocation( current->loc, static_cast<Direction>( i ) );

        if( !m_map( neighbor_loc ).isAvailable() ) continue;

        NodeVec::iterator it = m_open.begin();
        for( ; it != m_open.end(); ++it )
            if( (*it)->loc == neighbor_loc )
                break;

        // Search through open list for this neighbor
        if( it != m_open.end() )
        {
            Node* open_node = *it;
            if( current->g + 1 < open_node->g )
            {
                // We have found a better path to this location.  update heap
                open_node->g     = current->g + 1;
                open_node->child = current;
                open_node->dir   = static_cast<Direction>( i );
                std::make_heap( m_open.begin(), m_open.end(), NodeCompare() );
            }
            continue;
        }

        // Search through closed list for this neighbor
        it = m_closed.begin();
        for( ; it != m_closed.end(); ++it )
            if( (*it)->loc == neighbor_loc )
                break;

        if( it != m_closed.end() )
            continue;

        // We need to add a new node to our open list
        if( current->g+1 < m_max_depth )
        {
            Node* neighbor_node = new Node( neighbor_loc,
                                            static_cast<Direction>( i ),
                                            current->g+1,
                                            m_map.manhattanDistance( neighbor_loc, m_goal ),
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
  
  path.assign( m_origin, m_goal, m_path.begin(), m_path.end() ); 
}
