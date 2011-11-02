
#ifndef ASTAR_H_
#define ASTAR_H_

// 
// OPEN = priority queue containing START
// CLOSED = empty set
// while lowest rank in OPEN is not the GOAL:
//   current = remove lowest rank item from OPEN
//   add current to CLOSED
//   for neighbors of current:
//     cost = g(current) + movementcost(current, neighbor)
//     if neighbor in OPEN and cost less than g(neighbor):
//       remove neighbor from OPEN, because new path is better
//     if neighbor in CLOSED and cost less than g(neighbor): **
//       remove neighbor from CLOSED
//     if neighbor not in OPEN and neighbor not in CLOSED:
//       set g(neighbor) to cost
//       add neighbor to OPEN
//       set priority queue rank to g(neighbor) + h(neighbor)
//       set neighbor's parent to current
// 
// reconstruct reverse path from destination to start
// by following parent pointers
//

//
// TODO: Better handling of path finding failures. 
// TODO: Make sure handle calling search multiple times for single instance
//       (maybe make each instance only searchable once)
//
//

#include "Direction.h"
#include "Location.h"
#include "Map.h"

#include <algorithm>
#include <map>
#include <vector>


class Path;

class AStar
{
public:
    AStar( const Map& map, const Location& start_loc, const Location& destination_loc );

    template<class Iter>
    AStar( const Map& map, Iter begins, Iter ends, const Location& destination );

    void setMaxDepth( unsigned max_depth )   { m_max_depth = max_depth; }

    bool search();

    void getPath( Path& path )const;

private:
    //
    // Uncopyable
    //
    AStar( const AStar& );
    AStar& operator=( const AStar& );

    // 
    // A candidate node in the search.
    //
    struct Node
    {
        Node() : dir( NONE ), g( 0 ), h( 0 ), child( 0 ) {}

        Node( const Location& loc, Direction dir, int g, int h, Node* child ) 
            : loc( loc ), dir( dir ), g( g ), h( h ), child( child ) {}

        int f()const { return g+h; }

        Location loc;
        Direction dir;
        int g;
        int h;
        Node* child;
    };
    
    /// For heap sorting our open set
    struct NodeCompare
    {
        bool operator()( const Node* n0, const Node* n1 )
        { return n0->f() > n1->f(); }
    };

    /// For searching stl containers for node with a given location
    struct HasLocation
    {
        HasLocation( const Location& location ) : location( location ) {}
        bool operator()( Node* node )const { return node->loc == location; }
        Location location;
    };

    bool step();

    typedef std::vector<Direction>   DirectionVec;
    typedef std::vector<Node*>       NodeVec;
    typedef std::map<Location,Node*> LocationToNode;

    NodeVec               m_open;         ///< Heapified list of candidate nodes
    LocationToNode        m_closed;       ///< Processed nodes

    DirectionVec          m_path;         ///< Results of search
    
    const Map&            m_map;          ///< Map to be searched
    Location              m_origin;       ///< Starting node of path
    const Location        m_destination;  ///< Goal location

    unsigned              m_max_depth;    ///< Max depth on this search
};



template<class Iter>
AStar::AStar( const Map& map, Iter begins, Iter ends, const Location& destination )
    : m_map( map ),
      m_destination( destination ),
      m_max_depth( 20 )
{
    for( Iter it = begins; it != ends; ++it )
        m_open.push_back( new Node( *it, NONE, 0, m_map.manhattanDistance( *it, destination ), 0 ) );
    std::make_heap( m_open.begin(), m_open.end(), NodeCompare() );
}


#endif // ASTAR_H_
