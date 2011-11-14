

#include "Path.h"
#include "BF.h"
#include "Map.h"
#include <iostream>

bool pred( const Square& s )
{
  return s.hill_id > 0;
}


struct FindHill 
{
    bool operator()( const BFNode* node )
    {
        if( node->square->hill_id > 0 )
        {
            node->getPath( path );
            node->getRPath( rpath );
            rpath_origin = node->loc;
            return false;
        }
        return true;
    }
    Path path;
    Path rpath;
    Location rpath_origin;
};


struct FirstStepAvailable
{
    bool operator()( const BFNode* current, const Location& location, const Square& neighbor )
    {
        return current->depth > 0 || neighbor.isAvailable();
    }
};



struct FindAnts
{
    bool operator()( const BFNode* node )
    {
        if( node->square->ant_id == 0 )
        {
            ants.push_back( node->loc );
        }
        return true;
    }
    std::vector<Location> ants;
};


struct AlwaysAvailable
{
    bool operator()( const BFNode* current, const Location& location, const Square& neighbor )
    {
        return true;
    }
};

const int SIZE = 32;
int main( int argc, char** argv )
{

    Map map( SIZE, SIZE );

    for( int i = 0; i < SIZE; ++i )
      for( int j = 0; j < SIZE; ++j )
        map( i, j ).type = Square::LAND;

    for( int i = 8; i < SIZE-8; ++i ) 
      map( SIZE/2, i ).type = Square::WATER;

    Location dest( 12, 16 );
    Location orig(  18, 16 );

    map( dest ).hill_id = 1;
    map( orig ).ant_id  = 0;

    std::cerr << map << std::endl;


    FindHill find_hill;
    FirstStepAvailable first_step_available;
    BF<FindHill, FirstStepAvailable>  bfs( map, orig, find_hill, first_step_available );
    bfs.traverse();

    std::cerr <<  " path length: " << find_hill.path.size() << std::endl;
    
    int i = 0;
    Location current = orig;
    while( find_hill.path.nextStep() != NONE )
    {
       Direction d = find_hill.path.popNextStep();
       std::cerr << " Step " << i++ << ": in direction '" << DIRECTION_CHAR[ d ] << "'" << std::endl;
       current = map.getLocation( current, d );
       map( current ).ant_id = 0;
       std::cerr << map << std::endl;
    }

    std::cerr << " now in reverse ======================================" << std::endl;
    
    map.reset();
    
    std::cerr <<  " rpath length: " << find_hill.rpath.size() << std::endl;
    
    i = 0;
    current = find_hill.rpath_origin;
    while( find_hill.rpath.nextStep() != NONE )
    {
       Direction d = find_hill.rpath.popNextStep();
       std::cerr << " Step " << i++ << ": in direction '" << DIRECTION_CHAR[ d ] << "'" << std::endl;
       current = map.getLocation( current, d );
       map( current ).ant_id = 0;
       std::cerr << map << std::endl;
    }


    std::cerr << "ant search =========================" << std::endl;

    map.reset();
    map( Location(  4,  8 ) ).ant_id = 0;
    map( Location( 18, 12 ) ).ant_id = 0;
    map( Location( 20, 24 ) ).ant_id = 0;
    
    FindAnts        find_ants;
    AlwaysAvailable always_available;
    BF<FindAnts, AlwaysAvailable>  bfs2( map, Location( 12,16 ), find_ants, always_available );
    bfs2.traverse();

    for( std::vector<Location>::iterator it = find_ants.ants.begin(); it != find_ants.ants.end(); ++it )
    {
        std::cerr << *it << std::endl;
    }



    Map map1( 32, 32 );

    std::vector<Location> locs;
    locs.push_back( Location( 28, 1 ) );
    locs.push_back( Location( 28, 3 ) );
    locs.push_back( Location(  1, 3 ) );
    locs.push_back( Location(  1, 30 ) );

    std::cerr << "\n\ncentroid test =========================" << std::endl;
    for( std::vector<Location>::iterator it = locs.begin(); it != locs.end(); ++it )
        std::cerr << "    " << *it << std::endl;
    std::cerr << " centroid 0: " << map1.computeCentroid( locs ) << std::endl;
    
}

