
#include "AStar.h"
#include <algorithm>

AStar::AStar( const Map& map, const Location& goal, const Location& start )
    : m_map( map ),
      m_goal( goal ),
      m_cur_depth( 0 ),
      m_max_depth( 1000 )
{
    m_open.push_back( new Node( start, m_map.manhattanDistance( start, goal ) ) );
}


AStar::AStar( const Map& map, const Location& goal, const std::vector<Location>& starts )
    : m_map( map ),
      m_goal( goal ),
      m_cur_depth( 0 ),
      m_max_depth( 1000 )
{
    for( std::vector<Location>::const_iterator it = starts.begin(); it != starts.end(); ++it )
        m_open.push_back( new Node( *it, m_map.manhattanDistance( *it, goal ) ) );

    std::make_heap( m_open.begin(), m_open.end(), NodeCompare() );
}


bool AStar::search()
{

    while( m_cur_depth++ > m_max_depth && m_open.size() > 0 )
        if( step() ) return true;

    return false;
}


bool AStar::step()
{
    Node* current = m_open.front();

    // Check to see if we have reached our goal
    if( current->loc == m_goal )
    {
        // Backtrack to create path
        m_path.clear();
        m_path.reserve( m_cur_depth );
        while( current->child != 0 ) 
        {
            m_path.push_back( current->loc );
            current = current->child;
        }
        
        // Clean up
        for( NodeHeap::const_iterator it = m_open.begin(); it != m_open.end(); ++it )
            delete *it;
        m_open.clear();

        for( NodeSet::const_iterator it = m_closed.begin(); it != m_closed.end(); ++it )
            delete *it;
        m_closed.clear();

        // Indicate search completion
        return true;
    }

    // Move current from open to closed list 
    std::pop_heap( m_open.begin(), m_open.end(), NodeCompare() );
    m_open.pop_back();
    m_closed.insert( current );


    // Process all neighbors
    for( int i = 0; i < NUM_DIRECTIONS; ++i )
    {
        Location neighbor_loc = m_map.getLocation( current->loc, static_cast<Direction>( i ) );

        if( !m_map( neighbor_loc ).isAvailable() ) continue;

        for( NodeHeap::iterator it = m_open.begin(); 
        if( inOpen( neighbor

    }
    




}


bool AStar::getPath( std::vector<Location>& path )const
{
}
