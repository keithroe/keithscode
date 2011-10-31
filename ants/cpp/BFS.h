
#ifndef BFS_H_
#define BFS_H_

#include "Location.h"
#include "Direction.h"

#include <map>
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

    void getPath( Path& path )const;
private:
    //
    // Uncopyable
    //
    BFS( const BFS& );
    BFS& operator=( const BFS& );
    

    struct Node
    {
        Node( const Location& loc ) 
            : loc( loc ), dir( NONE ), child( 0 ) {}
        
        Node( const Location& loc, Direction dir, Node* child ) 
            : loc( loc ), dir( dir ), child( child ) {}

        Location  loc;
        Direction dir;
        Node*     child;
    };


    bool step();


    typedef std::vector<Node*>           NodeVec;
    typedef std::map<Location, Node*>    LocationToNode;

    const Map&       m_map;
    NodeVec          m_open;
    LocationToNode   m_closed;

    Predicate        m_predicate;
    unsigned         m_max_depth;
};


template<class Iter>
BFS::BFS( const Map& map, Iter begins, Iter ends, Predicate predicate )
    : m_map( map ),
      m_predicate( predicate )
{
    for( Iter it = begins; it != ends; ++it )
        m_open.push_back( new Node( *it ) );
}


#endif // BFS_H_
