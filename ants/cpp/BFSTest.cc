

#include "Path.h"
#include "BFS.h"
#include "Map.h"
#include <iostream>

bool pred( const Square& s )
{
  return s.hill_id > 0;
}

const int SIZE = 32;
int main( int argc, char** argv )
{

    Map map( SIZE, SIZE );

    for( int i = 0; i < SIZE; ++i )
      for( int j = 0; j < SIZE; ++j )
        map( i, j ).content = Square::EMPTY;

    for( int i = 8; i < SIZE-8; ++i ) 
      map( SIZE/2, i ).content = Square::WATER;

    Location dest( 12, 16 );
    Location orig(  18, 16 );

    map( dest ).content = Square::HILL;
    map( dest ).hill_id = 1;
    map( orig ).ant_id  = 0;

    std::cout << map << std::endl;

    BFS   bfs( map, orig, pred );
    if( !bfs.search() ) std::cerr << "No path found!!!!!!" << std::endl;
    else                std::cerr << "Path found" << std::endl;  

    Path path;
    bfs.getPath( path );

    std::cerr <<  " path length: " << path.size() << std::endl;
    
    int i = 0;
    Location current = bfs.origin();
    while( path.nextStep() != NONE )
    {
       Direction d = path.popNextStep();
       std::cerr << " Step " << i++ << ": in direction '" << DIRECTION_CHAR[ d ] << "'" << std::endl;
       current = map.getLocation( current, d );
       map( current ).ant_id = 0;
       std::cout << map << std::endl;
    }

    std::cerr << " now in reverse ======================================" << std::endl;
    
    map.reset();

    Path rpath;
    bfs.getReversePath( rpath );
    
    std::cerr <<  " rpath length: " << rpath.size() << std::endl;
    
    i = 0;
    current = bfs.destination();
    while( rpath.nextStep() != NONE )
    {
       Direction d = rpath.popNextStep();
       std::cerr << " Step " << i++ << ": in direction '" << DIRECTION_CHAR[ d ] << "'" << std::endl;
       current = map.getLocation( current, d );
       map( current ).ant_id = 0;
       std::cout << map << std::endl;
    }
}
