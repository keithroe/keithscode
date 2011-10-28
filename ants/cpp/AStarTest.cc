

#include "Path.h"
#include "AStar.h"
#include "Map.h"
#include <iostream>


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
    map( dest ).hill = 1;
    map( orig ).ant  = 0;

    std::cout << map << std::endl;

    AStar astar( map, dest, orig );
    if( !astar.search() ) std::cerr << "No path found!!!!!!" << std::endl;
    else                  std::cerr << "Path found" << std::endl;  

    Path path;
    astar.getPath( path );

    std::cerr <<  " path length: " << path.size() << std::endl;

    
    int i = 0;
    while( path.nextStep() != NONE )
    {
       Direction d = path.popNextStep();
       std::cerr << " Step " << i++ << ": in direction '" << DIRECTION_CHAR[ d ] << "'" << std::endl;
       orig = map.getLocation( orig, d );
       map( orig ).ant = 0;
       std::cout << map << std::endl;
    }
}
