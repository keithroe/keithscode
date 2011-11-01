
#ifndef BFS_H_
#define BFS_H_

#include "Location.h"
#include "Direction.h"

#include <map>
#include <deque>
#include <vector>

class Map;
class Square;
class Path;

class BFS
{
public:
    typedef bool (*Predicate)( const Square& square );

    BFS( const Map& map, const Location& start_loc, Predicate predicate );

    template<class Iter>
    BFS( const Map& map, Iter begin, Iter end, Predicate predicate );

    void setMaxDepth( unsigned max_depth )   { m_max_depth = max_depth; }

    bool search();

    /// Get path from start_loc to discovered destination
    void getPath( Path& path )const;         
    
    /// Get path from discovered destination to start_loc
    void getReversePath( Path& path )const;
private:
    //
    // Uncopyable
    //
    BFS( const BFS& );
    BFS& operator=( const BFS& );
    

    struct Node
    {
        Node( const Location& loc ) 
            : loc( loc ), dir( NONE ), depth( 0u ), child( 0 ) {}
        
        Node( const Location& loc, Direction dir, unsigned depth, Node* child ) 
            : loc( loc ), dir( dir ), depth( depth ), child( child ) {}

        Location  loc;
        Direction dir;
        unsigned  depth;
        Node*     child;
    };

    /// For searching stl containers for node with a given location
    struct HasLocation
    {
        HasLocation( const Location& location ) : location( location ) {}
        bool operator()( Node* node )const { return node->loc == location; }
        Location location;
    };

    bool step();


    typedef std::deque<Node*>            NodeQueue;
    typedef std::map<Location, Node*>    LocationToNode;

    const Map&             m_map;
    NodeQueue              m_open;
    LocationToNode         m_closed;

    std::vector<Direction> m_path;
    Location               m_origin; 
    Location               m_destination; 

    unsigned               m_max_depth;
    Predicate              m_predicate;
};


template<class Iter>
BFS::BFS( const Map& map, Iter begins, Iter ends, Predicate predicate )
    : m_map( map ),
      m_max_depth( 25u ),
      m_predicate( predicate )
{
    for( Iter it = begins; it != ends; ++it )
        m_open.push_back( new Node( *it ) );
}


#endif // BFS_H_
