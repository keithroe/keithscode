
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


#include "Map.h"
#include "Location.h"
#include <vector>
#include <set>

class AStar
{
public:
    struct Node
    {
        Node() 
            :  g( 0 ), h( 0 ), parent( 0 ), child( 0 ) {}

        Node( const Location& l, int h ) 
            : loc( l ), g( 0 ), h( h ), parent( 0 ), child( 0 ) {}

        int f()const { return g+h; }

        Location loc;
        int g;
        int h;
        Node* parent;
        Node* child;
    };

    struct NodeCompare
    {
        bool operator()( const Node* n0, const Node* n1 )
        { return n0->f() > n1->f(); }
    };

    AStar( const Map& map, const Location& goal_loc, const Location& start_loc );
    AStar( const Map& map, const Location& goal_loc, const std::vector<Location>& start_locs );

    void setMaxDepth( unsigned max_depth )   { m_max_depth = max_depth; }

    bool search();

    bool step();

    bool getPath( std::vector<Location>& path )const;

private:

    typedef std::vector<Node*> NodeHeap;
    typedef std::set<Node*>    NodeSet;

    NodeHeap              m_open;
    NodeSet               m_closed;

    std::vector<Location> m_path;
    
    const Map&            m_map;
    const Location        m_goal; 

    unsigned              m_cur_depth;
    unsigned              m_max_depth;
};




#endif // ASTAR_H_
