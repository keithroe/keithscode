
#ifndef KLIB_ASTAR_H_
#define KLIB_ASTAR_H_

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
// TODO: can a direct indexed array be used for 2D grid case?
//


// Graph
//     - Must have Graph::Node typename
//     - Must implement
//
//       Graph::getNeighbors( const Graph::Node& node, 
//                            std::vector<Graph::Node>& neighbors )const
//                            
//       Graph::getCostHeuristic( const Graph::Node& node0,
//                                const Graph::Node& node1 ); 
//
//       Graph::getCost( const Graph::Node& node0,
//                       const Graph::Node& node1 ); 
//
//     - cost heuristic should be admissable
//
// Graph::Node
//     - should be cheap to copy
//     - must have operator<  defined
//     - must have operator== defined
//               


#include <algorithm>
#include <set>
#include <vector>
#include <cassert>


//////////////////////////
#include <iostream>
//////////////////////////




template <typename Graph>
class AStar
{
public:
    typedef typename Graph::Node       GraphNode;
    typedef std::vector<GraphNode>     GraphNodeVec;


    AStar( const Graph&     graph,
           unsigned         max_depth,
           const GraphNode& origin,
           const GraphNode& destination );

    ~AStar();


    template <typename Iter>
    AStar( const Graph&     graph,
           unsigned         max_depth,
           Iter             origins_begin,
           Iter             origins_end,
           const GraphNode& destination )
        : m_graph( graph),
          m_destination( destination ),
          m_max_depth( max_depth ),
          m_destination_node( 0 ),
          m_steps( 0 ),
          m_updates( 0 )
    {
        for( Iter it = origins_begin; it != origins_end; ++it )
        {
            const float g = 0.0f;
            const float h = m_graph.getCostHeuristic( *it, destination );
            pushNode( *it, g, h, 0 );
        }
    }


    bool search();

    void getPath ( GraphNodeVec& path )const;
    void getRPath( GraphNodeVec& path )const;

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

        Node( const GraphNode& graph_node,
              float g,
              float h,
              Node* prev,
              int id ) 
            : graph_node( graph_node ),
              g( g ),
              h( h ),
              prev( prev ),
              id( id )
        {
        }

        float f()const { return g+h; }

        GraphNode graph_node;
        float g;
        float h;
        Node* prev;
        int id;
    };

    
    /// For heap sorting our open set
    struct HeapCompare
    {
        bool operator()( const Node* n0, const Node* n1 )
        { 
            return ( n0->f() >  n1->f() ) ||
                   ( n0->f() == n1->f() && n0->h > n1->h ); 
            //return ( n0->f() >  n1->f() ) ||
            //       ( n0->f() == n1->f() && n0->id < n1->id ); 
            //return n0->f() >  n1->f();
        }
    };
   
   
    /// For searching stl containers for AStar::Node with a given Graph::Node 
    struct HasGraphNode
    {
        HasGraphNode( const GraphNode& graph_node ) 
            : graph_node( graph_node ) {}

        bool operator()( Node* node )const
        { return node->graph_node == graph_node; }

        GraphNode graph_node;
    }; 


    void pushNode( const GraphNode& graph_node,
                   float g,
                   float h,
                   Node* prev);
    Node* popNode();

    bool step();


    typedef std::vector<Node*>        NodeVec;
    typedef std::set<Node*>           NodeSet;
    typedef std::set<GraphNode>       GraphNodeSet;

    // TODO: profile to see if m_closed_set is a speed up

    NodeVec               m_open_queue;  ///< Heapified list of candidate nodes
    GraphNodeSet          m_open_set;    ///< For quick open set membership test
    GraphNodeSet          m_closed_set;  ///< Processed nodes

    GraphNodeVec          m_neighbors;

    NodeVec               m_all_nodes;   ///< List of all nodes so we can clean 
    const Graph&          m_graph;       ///< Map to be searched
    const GraphNode&      m_destination; ///< Goal location

    unsigned              m_max_depth;   ///< Max depth on this search
    Node*                 m_destination_node;

    int                   m_steps;
    int                   m_updates;

};



template <typename Graph>
AStar<Graph>::AStar( const Graph& graph, 
                     unsigned max_depth,
                     const GraphNode& origin,
                     const GraphNode& destination )
    : m_graph( graph),
      m_destination( destination ),
      m_max_depth( max_depth ),
      m_destination_node( 0 ),
      m_steps( 0 ),
      m_updates( 0 )
{
    const float g = 0.0f;
    const float h = m_graph.getCostHeuristic( origin, destination );
    pushNode(  origin, g, h, 0 );
}


template <typename Graph>
AStar<Graph>::~AStar()
{
    for( typename NodeVec::iterator it = m_all_nodes.begin();
         it != m_all_nodes.end();
         ++it )
    {
        delete *it;
    }
}


template <typename Graph>
bool AStar<Graph>::search()
{
    while( m_open_queue.size() > 0 )
        if( step() )
        {
            std::cerr << "FOUND GOAL.  Steps  : " << m_steps << std::endl
                      << "             updates: " << m_updates << std::endl;
            return true;
        }
    return false;
}


template <typename Graph>
void AStar<Graph>::pushNode( const GraphNode& graph_node,
                     float g,
                     float h,
                     Node* prev)
{
    m_open_set.insert( graph_node ); 

    Node* node = new Node( graph_node, g, h, prev, m_all_nodes.size() );
    m_all_nodes.push_back( node );
    m_open_queue.push_back( node );

    std::push_heap( m_open_queue.begin(), m_open_queue.end(), HeapCompare() );
    
    /*
    std::cerr << "    pushing: " << node->graph_node << std::endl
              << "             " << node->g << std::endl
              << "             " << node->h << std::endl
              << "             " << node->prev << std::endl;
              */
}


template <typename Graph>
typename AStar<Graph>::Node* AStar<Graph>::popNode()
{
    Node* node = m_open_queue.front();
    std::pop_heap( m_open_queue.begin(), m_open_queue.end(), HeapCompare() );
    m_open_queue.pop_back();

    m_open_set.erase( node->graph_node );
    m_closed_set.insert( node->graph_node );

    return node;
}


template <typename Graph>
bool AStar<Graph>::step()
{
    m_steps++;

    Node* current = popNode();

    /*
    std::cerr << "current: " << current->graph_node << std::endl
              << "         " << current->g << std::endl
              << "         " << current->h << std::endl
              << "         " << current->prev << std::endl
              << "         " << current << std::endl;
              */

    //
    // Check to see if we have reached our destination
    //
    if( current->graph_node == m_destination )
    {
        m_destination_node = current;
        return true;
    }

    //
    // Process neighbors
    //
    if( current->f() < m_max_depth )
    {
        m_neighbors.clear();
        m_graph.getNeighbors( current->graph_node, m_neighbors );

    // TODO: cull based on f() < max_depth too
        bool need_to_make_heap = false;
        for( typename GraphNodeVec::iterator neighbor = m_neighbors.begin();
             neighbor != m_neighbors.end();
             ++neighbor )
        {
            float neighbor_g = current->g +
                               m_graph.getCost( current->graph_node,
                                                *neighbor );

            // Search open list for this neighbor
            if( m_open_set.find( *neighbor ) != m_open_set.end() )
            {
                typename
                NodeVec::iterator it = std::find_if( m_open_queue.begin(),
                                                     m_open_queue.end(),
                                                     HasGraphNode(*neighbor) );
                Node* open_node = *it;

                if( neighbor_g < open_node->g )
                {
                    // We found a better path to this location -- update heap
                    m_updates++;
                    open_node->g    = neighbor_g;
                    open_node->prev = current;
                    need_to_make_heap = true;
                    
                    std::make_heap( m_open_queue.begin(),
                                    m_open_queue.end(),
                                    HeapCompare() );
                                    
                }
                continue;
            }
            
            // Check to see if this neighbor is in closed set 
            if( m_closed_set.find( *neighbor ) != m_closed_set.end() )
                continue;

            // We need to add a new node to our open list
            pushNode( *neighbor,
                      neighbor_g,
                      m_graph.getCostHeuristic( *neighbor, m_destination ),
                      current );
        }


        /*
        if( need_to_make_heap )
            std::make_heap( m_open_queue.begin(),
                            m_open_queue.end(),
                            HeapCompare() );
                            */
    }

    //
    // Indicate the search is not finished
    //
    return false;
}


template <typename Graph>
void AStar<Graph>::getPath( GraphNodeVec& path )const
{
    assert( m_destination_node );             // TODO: get rid
    Node* current = m_destination_node;
    while( current->prev != 0 ) 
    {
        path.push_back( current->graph_node );
        current = current->prev;
    }

    std::reverse( path.begin(), path.end() );
}


#endif // KLIB_ASTAR_H_
