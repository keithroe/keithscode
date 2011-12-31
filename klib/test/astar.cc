
#include "../AStar.h"
#include "../Logger.h"

#include <fstream>

class Graph
{
public:

    struct Tile
    {
        Tile() : is_wall( false ), is_path( false ) {}
        bool is_wall;
        bool is_path;
    };


    class Node
    {
    public:

        Node() : x( 0 ), y( 0 ) {}
        Node( int x, int y ) : x( x ), y( y ) {}

        bool operator<(  const Node& node1 )const
        {
            return ( x <  node1.x ) ||
                   ( x == node1.x && y < node1.y );
        }

        bool operator==(  const Node& node1 )const
        {
            return x == node1.x && y == node1.y;
        }

        int x, y;
    };


    Graph( int x, int y )
        : m_x( x ), m_y( y )
    {
        m_grid = new Tile*[ m_x ];
        for( int i = 0; i < m_x; ++i )
            m_grid[ i ] = new Tile[ m_y ];
    }


    ~Graph()
    {
        for( int i = 0; i < m_x; ++i )
            delete [] m_grid[ i ]; 
        delete [] m_grid;

    }

    float getCostHeuristic( const Node& node0, const Node& node1 )const
    {
        // Manhattan dist
        return static_cast<float>( abs( node0.x - node1.x ) +
                                   abs( node0.y - node1.y ) );
    }

    float getCost( const Node& node0, const Node& node1 )const
    {
        return 1.0f;
    }

    bool inRange( const Node& node )const
    {
        return node.x >= 0 && node.x < m_x-1  && node.y >= 0 && node.y < m_y-1;
    }

    void getNeighbors( const Node& node, std::vector<Node>& neighbors )const
    {

        Node neighbor;
        neighbor.x = node.x-1;
        neighbor.y = node.y+0;
        if( inRange( neighbor ) && !m_grid[ neighbor.x ][ neighbor.y ].is_wall )
            neighbors.push_back( neighbor );

        neighbor.x = node.x+1;
        neighbor.y = node.y+0;
        if( inRange( neighbor ) && !m_grid[ neighbor.x ][ neighbor.y ].is_wall )
            neighbors.push_back( neighbor );
        
        neighbor.x = node.x+0;
        neighbor.y = node.y-1;
        if( inRange( neighbor ) && !m_grid[ neighbor.x ][ neighbor.y ].is_wall )
            neighbors.push_back( neighbor );

        neighbor.x = node.x+0;
        neighbor.y = node.y+1;
        if( inRange( neighbor ) && !m_grid[ neighbor.x ][ neighbor.y ].is_wall )
            neighbors.push_back( neighbor );
    }


    int m_x, m_y;
    Tile** m_grid;
};


std::ostream& operator<<( std::ostream& out, const Graph::Node& node )
{
    out << "[" << node.x << "," << node.y << "]";
    return out;
}

std::ostream& operator<<( std::ostream& out, const Graph& graph )
{
    for( int i = 0; i < graph.m_x; ++i )
    {
        for( int j = 0; j < graph.m_y; ++j )
        {
            if( graph.m_grid[j][i].is_wall )
                out << "X ";
            else if( graph.m_grid[j][i].is_path )
                out << "o ";
            else 
                out << ". ";
        }
        out << std::endl;
    }

    return out;
}


void testLarge()
{
    Graph graph( 1024, 1024 );
    for( int i = 0; i < 1008; ++i ) graph.m_grid[ i ][ 16 ].is_wall = true;
    for( int i = 8; i < 1024; ++i ) graph.m_grid[ i ][ 32 ].is_wall = true;
    for( int i = 0; i < 1008; ++i ) graph.m_grid[ i ][ 48 ].is_wall = true;
    for( int i = 48; i < 1008; ++i ) graph.m_grid[ 1008 ][ i ].is_wall = true;
    for( int i = 8; i < 1008; ++i ) graph.m_grid[ i ][ 1008 ].is_wall = true;
    

    AStar<Graph> astar( graph, 10000, Graph::Node( 1, 1 ),
                                      Graph::Node( 512, 512 ) );
    astar.search();

    std::vector<Graph::Node> path;
    astar.getPath( path );

    /*
    for( std::vector<Graph::Node>::iterator it = path.begin();
         it != path.end();
         ++it )
    {
        std::cerr << *it << std::endl;
        graph.m_grid[ it->x ][ it->y ].is_path = true;
    }

    std::cerr << graph;
    */
}


void testSmall()
{

    Graph graph( 16, 16 );
    for( int i = 0; i < 12; ++i ) graph.m_grid[ i ][ 4  ].is_wall = true;
    for( int i = 4; i < 16; ++i ) graph.m_grid[ i ][ 8  ].is_wall = true;
    for( int i = 9; i < 12; ++i ) graph.m_grid[ 4 ][ i  ].is_wall = true;
    for( int i = 4; i < 12; ++i ) graph.m_grid[ i ][ 12 ].is_wall = true;
    
    std::cerr << graph;

    AStar<Graph> astar( graph, 10000, Graph::Node( 1, 1 ),
                                      Graph::Node( 12, 12 ) );
                                      //Graph::Node( 12, 3 ),
                                      //Graph::Node( 3, 7 ) );
    astar.search();

    std::vector<Graph::Node> path;
    astar.getPath( path );

    for( std::vector<Graph::Node>::iterator it = path.begin();
         it != path.end();
         ++it )
    {
        std::cerr << *it << std::endl;
        graph.m_grid[ it->x ][ it->y ].is_path = true;
    }

    std::cerr << graph;
}


int main( int argc, char** argv )
{
    Log::setReportingLevel( Log::INFO );

    std::ofstream fout( "log.txt" ); 
    Log::setStream( fout );

    //testSmall();
    for( int i = 0; i < 10; ++i )
        testLarge();
    
}
