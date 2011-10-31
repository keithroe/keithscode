
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
// reconstruct reverse path from goal to start
// by following parent pointers
//

//
// TODO: Better handling of path finding failures
//
//

#include "Direction.h"
#include "Location.h"
#include <set>
#include <vector>

class Map;
class Path;

class AStar
{
public:

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


    struct NodeCompare
    {
        bool operator()( const Node* n0, const Node* n1 )
        { return n0->f() > n1->f(); }
    };


    AStar( const Map& map, const Location& start_loc, const Location& goal_loc );
    AStar( const Map& map, const std::vector<Location>& start_locs, const Location& goal_loc );

    void setMaxDepth( unsigned max_depth )   { m_max_depth = max_depth; }

    bool search();


    void getPath( Path& path )const;

private:
    bool step();

    typedef std::vector<Direction> DirectionVec;
    typedef std::vector<Node*>     NodeVec;
    typedef std::vector<Location>  LocationVec;

    NodeVec               m_open;      ///< Heapified list of candidate nodes
    NodeVec               m_closed;    ///< Processed nodes

    DirectionVec          m_path;      ///< Results of search
    Location              m_origin;    ///< Starting node of path
    
    const Map&            m_map;       ///< Map to be searched
    const Location        m_goal;      ///< Goal locations

    unsigned              m_max_depth; ///< Max depth on this search
};




#endif // ASTAR_H_
