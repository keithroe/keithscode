
#ifndef BF_H_
#define BF_H_

#include "Debug.h"
#include "Direction.h"
#include "Location.h"
#include "Map.h"

#include <algorithm>
#include <deque>
#include <map>
#include <vector>

class Map;

//------------------------------------------------------------------------------
//
// BF::BFNode methods
//
//------------------------------------------------------------------------------

struct BFNode
{
    BFNode( const Location& loc,
            Square* square,
            Direction dir,
            unsigned depth,
            BFNode* child ) 
        : loc( loc ),
          square( square ),
          dir( dir ),
          depth( depth ),
          child( child )
          
        {}

    void getPath( Path& path )const;
    void getRPath( Path& path )const;

    Location  loc;
    Square*   square;
    Direction dir;
    unsigned  depth;
    BFNode*   child;
};

inline void BFNode::getPath( Path& path )const
{
    // Backtrack to create path
    path.reset();

    std::deque<Direction> dirs;     // TODO: switch to vec once correct
    for( const BFNode* current = this; current->child != 0; current = current->child )
    {
        dirs.push_front( current->dir );
    }
    path.assign( loc, dirs.begin(), dirs.end() );
}


inline void BFNode::getRPath( Path& path )const
{
    // Backtrack to create path
    path.reset();

    std::vector<Direction> dirs;
    Location destination;
    for( const BFNode* current = this; current->child != 0; current = current->child )
    {
        dirs.push_back( reverseDirection( current->dir ) );
        destination = current->child->loc;
    }
    path.assign( destination, dirs.begin(), dirs.end() );
}

///
/// Action: bool (*Action)( const BFNode* node )
///    - returns true to continue search, false to stop
///    - stores any results of search (such as path or list of found nodes)
///
/// ValidNeighbor: bool (*ValidNeighbor)( const BFNode* current, const Location& location, const Square& neighbor )
///    - returns true if this square is traversable for this search
///
template< class Action, class ValidNeighbor >
class BF
{
public:
    BF( Map& map, const Location& start_loc, Action& action, ValidNeighbor& valid_neighbor );
    ~BF();

    void setMaxDepth( unsigned depth ) { m_max_depth = depth; }

    void traverse();

private:
    //
    // Uncopyable
    //
    BF( const BF& );
    BF& operator=( const BF& );
    

    /// For searching stl containers for node with a given location
    struct HasLocation
    {
        HasLocation( const Location& location ) : location( location ) {}
        bool operator()( BFNode* node )const { return node->loc == location; }
        Location location;
    };

    bool step();


    typedef std::deque<BFNode*>            BFNodeQueue;
    typedef std::map<Location, BFNode*>    LocationToBFNode;

    BFNodeQueue              m_open;
    LocationToBFNode         m_closed;

    Map&                   m_map;
    Action&                m_action;
    ValidNeighbor&         m_valid_neighbor;

    unsigned               m_max_depth;
};




//------------------------------------------------------------------------------
//
// BF methods
//
//------------------------------------------------------------------------------

template< class Action, class ValidNeighbor >
BF<Action, ValidNeighbor>::BF( Map& map,
                             const Location& start_loc,
                             Action& action,
                             ValidNeighbor& valid_neighbor )
  : m_map( map ),
    m_action( action ),
    m_valid_neighbor( valid_neighbor ),
    m_max_depth( 25u )
{
    m_open.push_back( new BFNode( start_loc, &m_map( start_loc ), NONE, 0u, NULL ) );
}


template< class Action, class ValidNeighbor >
BF<Action, ValidNeighbor>::~BF()
{
    for( BFNodeQueue::iterator it = m_open.begin(); it != m_open.end(); ++it )
        delete *it;
    m_open.clear();

    for( LocationToBFNode::iterator it = m_closed.begin(); it != m_closed.end(); ++it )
        delete it->second;
    m_closed.clear();
}


template< class Action, class ValidNeighbor >
void BF<Action, ValidNeighbor>::traverse()
{
    while( !m_open.empty() && step() ) ;
}


template< class Action, class ValidNeighbor >
bool BF<Action, ValidNeighbor>::step()
{
    BFNode* current =  m_open.front();

    //Debugstream() << "  Checking " << current->loc << std::endl;
    //
    // Check to see if we have reached our goal
    //
    if( !m_action( current ) ) return false;

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
            //Debugstream() << "    Checking  neighbor " << neighbor_loc  << std::endl;
            
            /*
            // Check if the neighbor is available to be traversed
            if( !m_map( neighbor_loc ).isAvailable() ) 
            */

            if( !m_map( neighbor_loc ).isLand() || !m_valid_neighbor( current, neighbor_loc, m_map( neighbor_loc ) ) )
            {
                //Debugstream() << "      not avail " << std::endl;
                continue;
            }

            // Check if neighbor is already in open set
            if( std::find_if( m_open.begin(), m_open.end(), HasLocation( neighbor_loc ) ) != m_open.end() )
            {
                //Debugstream() << "      in open set" << std::endl;
                continue;
            }

            // Check to see if this neighbor is in closed set 
            if( m_closed.find( neighbor_loc ) != m_closed.end() )
            {
                //Debugstream() << "      in closed set" << std::endl;
                continue;
            }

            // Insert this location into open set
            //Debugstream() << "       pushing " << std::endl;
            BFNode* neighbor_node = new BFNode( neighbor_loc,
                                                &m_map( neighbor_loc ),
                                                static_cast<Direction>( i ),
                                                current->depth+1,
                                                current );
            m_open.push_back( neighbor_node  );
        }
    }

    return true;
}


#endif // BF_H_
