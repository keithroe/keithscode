
#include "Timer.h"
#include "Path.h"
#include "AStar.h"
#include "Map.h"
#include <iostream>


const int SIZE = 200;


int main( int argc, char** argv )
{

    Map map( SIZE, SIZE );

    for( int i = 0; i < SIZE; ++i )
      for( int j = 0; j < SIZE; ++j )
        map( i, j ).type = Square::LAND;

    for( int i = 4; i < SIZE-4; ++i ) 
      map( SIZE/2, i ).type = Square::WATER;

    Location hill( SIZE / 3, SIZE / 2 );
    Location ant0(  2*SIZE / 3, 1*SIZE / 3 );
    Location ant1(  2*SIZE / 3, 2*SIZE / 3 );

    map( hill ).hill_id  = 1;
    map( ant0 ).ant_id   = 0;
    map( ant1 ).ant_id   = 0;
    map.setPriority( hill, 100.0f );
    map.setPriority( ant0, -10.0f );
    map.setPriority( ant1, -10.0f );

    std::cout << map << std::endl;

    std::cerr << " 1 --------------" << std::endl;
    map.diffusePriority( 1 );
    std::cout << map << std::endl;
    std::cerr << " 2 --------------" << std::endl;
    map.diffusePriority( 1 );
    std::cout << map << std::endl;
    std::cerr << " 3 --------------" << std::endl;
    map.diffusePriority( 1 );
    std::cout << map << std::endl;
    std::cerr << " 4 --------------" << std::endl;
    map.diffusePriority( 1 );
    std::cout << map << std::endl;
    std::cerr << " 5 --------------" << std::endl;

    Timer timer;
    timer.start();
    map.diffusePriority( 50 );
    std::cout << map << std::endl;
    double dt = timer.getTime();
    std::cerr << " Took " << dt << "ms" << std::endl;
    
    std::cout << map << std::endl;
}
