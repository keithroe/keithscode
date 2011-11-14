

#include <cstring>
#include <Map>

#include "Ant.h"
#include "BF.h"
#include "Battle.h"
#include "Debug.h"
#include "Map.h"

namespace
{
    typedef std::vector<Ant*>                          Ants;
    typedef std::set<Ant*>                             AntSet;
    typedef std::vector<Location>                      EnemyAnts;
    typedef std::set<Location>                         EnemyAntSet;
    typedef std::map<Location, std::vector<Location> > NearbyAnts;

    // TODO: for now, only take into account my ants as enemies of enemies
    //       need to add in other players ants as enemies of enemies too?
    struct AllEnemyAnts 
    {
        AllEnemyAnts( const Location& location, EnemyAntSet& enemies, NearbyAnts& nearby  ) 
            : location( location ), enemies( enemies ), nearby( nearby ), found_enemy( false ) {}

        bool operator()( const BFNode* node )
        { 
            if( node->square->ant_id > 0 )
            {
                enemies.insert( node->loc );
                nearby[ location  ].push_back( node->loc )
                nearby[ node->loc ].push_back( location )
                found_enemy = true;
            }
            return true;
        }
        
        Location            location;
        EnemyAntSet&        enemies;
        NearbyAnts&         nearby;
        bool                found_enemy;
    };


    struct BattlePeriphery
    {
        BattlePeriphery( const Map& map, const Location& origin ) 
            : map( map ), origin( origin ) {}

        bool operator()( const BFNode* current, const Location& location, const Square& neighbor )
        {
            // TODO: magic number -- compute this from attackrad2
            return map.distance2( origin, location ) <= 17;
        }
        const Map&     map;
        const Location origin;
    };

    
    typedef BF<AllEnemyAnts, BattlePeriphery> EnemyCombatants;


    inline void clamp( int width, int height, Location& location )
    {
        const int row = location.row;
        const int col = location.col;
        location.col = col < 0       ? col + width : 
                       col > width-1 ? col - width :
                       col;
        location.row = row < 0        ? row + height : 
                       row > height-1 ? row - height :
                       row;
    }


    inline Location clamp( int width, int height, int row, int col )
    {
        Location location( row, col );
        clamp( width, heigh, row, col );
        return location;
    }


    void fill( int*** grid, unsigned width, unsigned height, const Location& location, int inc)
    {
        setFillColor( 0, 0, 255, 0.1 );

        Location x = clamp( width, height, location.row-2, location.col-1 );
        grid[ x.row ][ x.col ][ 0 ] += inc;
        tile( x );
        x = clamp( width, height, location.row-2, location.col-0 );
        grid[ x.row ][ x.col ][ 0 ] += inc;
        tile( x );
        x = clamp( width, height, location.row-2, location.col+1 );
        grid[ x.row ][ x.col ][ 0 ] += inc;
        tile( x );

        x = clamp( width, height, location.row-1, location.col-2 );
        grid[ x.row ][ x.col ][ 0 ] += inc;
        tile( x );
        x = clamp( width, height, location.row-1, location.col-1 );
        grid[ x.row ][ x.col ][ 0 ] += inc;
        tile( x );
        x = clamp( width, height, location.row-1, location.col-0 );
        grid[ x.row ][ x.col ][ 0 ] += inc;
        tile( x );
        x = clamp( width, height, location.row-1, location.col+1 );
        grid[ x.row ][ x.col ][ 0 ] += inc;
        tile( x );
        x = clamp( width, height, location.row-1, location.col+2 );
        grid[ x.row ][ x.col ][ 0 ] += inc;
        tile( x );
        
        x = clamp( width, height, location.row-0, location.col-2 );
        grid[ x.row ][ x.col ][ 0 ] += inc;
        tile( x );
        x = clamp( width, height, location.row-0, location.col-1 );
        grid[ x.row ][ x.col ][ 0 ] += inc;
        tile( x );
        x = clamp( width, height, location.row-0, location.col-0 );
        grid[ x.row ][ x.col ][ 0 ] += inc;
        tile( x );
        x = clamp( width, height, location.row-0, location.col+1 );
        grid[ x.row ][ x.col ][ 0 ] += inc;
        tile( x );
        x = clamp( width, height, location.row-0, location.col+2 );
        grid[ x.row ][ x.col ][ 0 ] += inc;
        tile( x );
        
        x = clamp( width, height, location.row+1, location.col-2 );
        grid[ x.row ][ x.col ][ 0 ] += inc;
        tile( x );
        x = clamp( width, height, location.row+1, location.col-1 );
        grid[ x.row ][ x.col ][ 0 ] += inc;
        tile( x );
        x = clamp( width, height, location.row+1, location.col-0 );
        grid[ x.row ][ x.col ][ 0 ] += inc;
        tile( x );
        x = clamp( width, height, location.row+1, location.col+1 );
        grid[ x.row ][ x.col ][ 0 ] += inc;
        tile( x );
        x = clamp( width, height, location.row+1, location.col+2 );
        grid[ x.row ][ x.col ][ 0 ] += inc;
        tile( x );

        x = clamp( width, height, location.row+2, location.col-1 );
        grid[ x.row ][ x.col ][ 0 ] += inc;
        tile( x );
        x = clamp( width, height, location.row+2, location.col-0 );
        grid[ x.row ][ x.col ][ 0 ] += inc;
        tile( x );
        x = clamp( width, height, location.row+2, location.col+1 );
        grid[ x.row ][ x.col ][ 0 ] += inc;
        tile( x );
    }

    void fillEnemy( int*** grid, unsigned width, unsigned height, const Location& location, int ant_id, int inc)
    {
        setFillColor( 255, 0, 0, 0.1 );

        // TODO: only expand the zone in directions which do not lead to water
        // TODO: how can we better represent enemy ant location on next move???
        Location x = clamp( width, height, location.row-3, location.col-1 );
        grid[ x.row ][ x.col ][ ant_id ] += inc;
        tile( x );
        x = clamp( width, height, location.row-3, location.col-0 );
        grid[ x.row ][ x.col ][ ant_id ] += inc;
        tile( x );
        x = clamp( width, height, location.row-3, location.col+1 );
        grid[ x.row ][ x.col ][ ant_id ] += inc;
        tile( x );

        x = clamp( width, height, location.row-2, location.col-2 );
        grid[ x.row ][ x.col ][ ant_id ] += inc;
        tile( x );
        x = clamp( width, height, location.row-2, location.col-1 );
        grid[ x.row ][ x.col ][ ant_id ] += inc;
        tile( x );
        x = clamp( width, height, location.row-2, location.col-0 );
        grid[ x.row ][ x.col ][ ant_id ] += inc;
        tile( x );
        x = clamp( width, height, location.row-2, location.col+1 );
        grid[ x.row ][ x.col ][ ant_id ] += inc;
        tile( x );
        x = clamp( width, height, location.row-2, location.col+2 );
        grid[ x.row ][ x.col ][ ant_id ] += inc;
        tile( x );

        x = clamp( width, height, location.row-1, location.col-3 );
        grid[ x.row ][ x.col ][ ant_id ] += inc;
        tile( x );
        x = clamp( width, height, location.row-1, location.col-2 );
        grid[ x.row ][ x.col ][ ant_id ] += inc;
        tile( x );
        x = clamp( width, height, location.row-1, location.col-1 );
        grid[ x.row ][ x.col ][ ant_id ] += inc;
        tile( x );
        x = clamp( width, height, location.row-1, location.col-0 );
        grid[ x.row ][ x.col ][ ant_id ] += inc;
        tile( x );
        x = clamp( width, height, location.row-1, location.col+1 );
        grid[ x.row ][ x.col ][ ant_id ] += inc;
        tile( x );
        x = clamp( width, height, location.row-1, location.col+2 );
        grid[ x.row ][ x.col ][ ant_id ] += inc;
        tile( x );
        x = clamp( width, height, location.row-1, location.col+3 );
        grid[ x.row ][ x.col ][ ant_id ] += inc;
        tile( x );
        
        x = clamp( width, height, location.row-0, location.col-3 );
        grid[ x.row ][ x.col ][ ant_id ] += inc;
        tile( x );
        x = clamp( width, height, location.row-0, location.col-2 );
        grid[ x.row ][ x.col ][ ant_id ] += inc;
        tile( x );
        x = clamp( width, height, location.row-0, location.col-1 );
        grid[ x.row ][ x.col ][ ant_id ] += inc;
        tile( x );
        x = clamp( width, height, location.row-0, location.col-0 );
        grid[ x.row ][ x.col ][ ant_id ] += inc;
        tile( x );
        x = clamp( width, height, location.row-0, location.col+1 );
        grid[ x.row ][ x.col ][ ant_id ] += inc;
        tile( x );
        x = clamp( width, height, location.row-0, location.col+2 );
        grid[ x.row ][ x.col ][ ant_id ] += inc;
        tile( x );
        x = clamp( width, height, location.row-0, location.col+3 );
        grid[ x.row ][ x.col ][ ant_id ] += inc;
        tile( x );
        
        x = clamp( width, height, location.row+1, location.col-3 );
        grid[ x.row ][ x.col ][ ant_id ] += inc;
        tile( x );
        x = clamp( width, height, location.row+1, location.col-2 );
        grid[ x.row ][ x.col ][ ant_id ] += inc;
        tile( x );
        x = clamp( width, height, location.row+1, location.col-1 );
        grid[ x.row ][ x.col ][ ant_id ] += inc;
        tile( x );
        x = clamp( width, height, location.row+1, location.col-0 );
        grid[ x.row ][ x.col ][ ant_id ] += inc;
        tile( x );
        x = clamp( width, height, location.row+1, location.col+1 );
        grid[ x.row ][ x.col ][ ant_id ] += inc;
        tile( x );
        x = clamp( width, height, location.row+1, location.col+2 );
        grid[ x.row ][ x.col ][ ant_id ] += inc;
        tile( x );
        x = clamp( width, height, location.row+1, location.col+3 );
        grid[ x.row ][ x.col ][ ant_id ] += inc;
        tile( x );

        x = clamp( width, height, location.row+2, location.col-2 );
        grid[ x.row ][ x.col ][ ant_id ] += inc;
        tile( x );
        x = clamp( width, height, location.row+2, location.col-1 );
        grid[ x.row ][ x.col ][ ant_id ] += inc;
        tile( x );
        x = clamp( width, height, location.row+2, location.col-0 );
        grid[ x.row ][ x.col ][ ant_id ] += inc;
        tile( x );
        x = clamp( width, height, location.row+2, location.col+1 );
        grid[ x.row ][ x.col ][ ant_id ] += inc;
        tile( x );
        x = clamp( width, height, location.row+2, location.col+2 );
        grid[ x.row ][ x.col ][ ant_id ] += inc;
        tile( x );
        
        x = clamp( width, height, location.row+3, location.col-1 );
        grid[ x.row ][ x.col ][ ant_id ] += inc;
        tile( x );
        x = clamp( width, height, location.row+3, location.col-0 );
        grid[ x.row ][ x.col ][ ant_id ] += inc;
        tile( x );
        x = clamp( width, height, location.row+3, location.col+1 );
        grid[ x.row ][ x.col ][ ant_id ] += inc;
        tile( x );
    }


    // TODO: going to need incremental updates to score
    int score( const Ants& allies, const EnemyAntSet& enemies, const NearbyAnts& nearby, int*** grid )
    {
        for( Ants::const_iterator it = allies.begin(); it != allies.end(); ++it )
        {
            Location loc = (*it)->location;
            int counter=0;
            counter -= grid[ loc.row ][ loc.col ][ 1 ];
            counter -= grid[ loc.row ][ loc.col ][ 2 ];
            counter -= grid[ loc.row ][ loc.col ][ 3 ];
            counter -= grid[ loc.row ][ loc.col ][ 4 ];
            counter -= grid[ loc.row ][ loc.col ][ 5 ];
            counter -= grid[ loc.row ][ loc.col ][ 6 ];
            counter -= grid[ loc.row ][ loc.col ][ 7 ];
            counter -= grid[ loc.row ][ loc.col ][ 8 ];
            counter -= grid[ loc.row ][ loc.col ][ 9 ];

            for( NearbyAnts::iterator it = nearby.find( loc ); it != nearby.end(); ++it )
            {
            }
        }

        for( EnemyAntSet::const_iterator it = enemies.begin(); it != enemies.end(); ++it )
        {
            Location loc = *it;
            counter += grid[ loc.row ][ loc.col ][ 1 ];
        }
    }
}


void battle( Map& map,
             const std::vector<Ant*>& ants,
             const std::vector<Location>& enemy_ants,
             std::set<Ant*>& assigned )
{

    //
    // cluster ants
    //
    // for each cluster
    //
    //     create local grid
    //
    //     initial splat ants (splat enemies with extent of all possible moves)
    //
    //     simulated annealing
    //
    //         while !( all_enemies_dead and all_allies alive ) or max_attempts_reached
    //             mutate (try moving single ant first)
    //             adust transitioned ants splat
    //             check (enemies dead - allies alive ) to see if new max
    //
    //     process moves -- mark as many as possible free so they can do other things
    //

    //
    // Find all ants which have enemies within manhattan dist of 2 of battle zone
    //
    EnemyAntSet enemies;
    Ants        allies;
    NearbyAnts  nearby;

    for( Ants::const_iterator it = ants.begin(); it != ants.end(); ++it )
    {
        Debug::stream() << "    searching from " << (*it )->location << std::endl;
        AllEnemyAnts    enemy_ants( enemies, nearby );
        BattlePeriphery periphery( map, (*it )->location );
        EnemyCombatants bfs( map, (*it)->location, enemy_ants, periphery );
        bfs.traverse();
        if( enemy_ants.found_enemy ) allies.push_back( *it );
    }

    if( allies.empty() ) return;

    const unsigned height = map.height();
    const unsigned width  = map.width();
    int*** grid; 
    grid = new int**[ height ];
    for( unsigned i = 0; i < height; ++i )
    {
        grid[i] = new int*[ width ];
        for( unsigned j = 0; j < width; ++j )
        {
            grid[i][j] = new int[ 10 ];
            memset( reinterpret_cast<void*>( grid[i][j] ), 0, 10*sizeof(unsigned) );
        }
    }
    
    setFillColor( 0, 0, 255, 0.5f );
    for( EnemyAntSet::const_iterator it = enemies.begin(); it != enemies.end(); ++it )
    {
        fillEnemy( grid, width, height, *it, map( *it ).ant_id, 1 );
        circle( *it, 1, true );
    }

    setFillColor( 255, 0, 0, 0.5f );
    for( Ants::const_iterator it = allies.begin(); it != allies.end(); ++it )
    {
        fill( grid, width, height, (*it)->location, 1 );
        circle( (*it)->location, 1, true );
    }

    // TODO: bias the ants to attack
    const int MAX_ITERATIONS = 100;

    std::vector<Direction> moves( allies.size(), NONE );
    std::vector<Direction> best_moves( allies.size(), NONE );
    int max_score = score( allies, enemies, grid );
    for( int i = 0; i < MAX_ITERATIONS; ++i )
    {
        // TODO: randomize an array the length of allies with indices [0,allies.size()) and cycle through it
        int transition_ant = rand() % allies.size();

        fill( grid, width, height, map.getLocation( allies[transition_ant]->location, moves[transition_ant] ), -1 );
        moves[transition_ant] = static_cast<Direction>( ( moves[transition_ant ] + rand() % 4 ) % 5 );
        fill( grid, width, height, map.getLocation( allies[transition_ant]->location, moves[transition_ant] ), 1 );
        
        int cur_score = score( allies, enemies, nearby, grid );
        if( cur_score > max_score )
        {
            max_score = cur_score;
            best_moves.assign( moves.begin(), moves.end() );
        }
    }


    for( unsigned i = 0; i < height; ++i )
    {
        for( unsigned j = 0; j < width; ++j )
            delete [] grid[i][j];
        delete grid[i];
    }
    delete grid;
    grid = 0u;



   
    /*
    EnemyAntSet clustered;
    for( EnemyAnts::const_iterator it = enemy_ants.begin(); it != enemy_ants.end(); ++it )
    {
    }
    */


    /*
    // TODO: add positive weights for ants which can just see enemy ants (so that they draw other ants BEFORE battle )
    for( Ants::const_iterator it = ants.begin(); it != ants.end(); ++it )
    {
        FindAllAnts find_all;
        Always      always;
        FindAnts bfs( map, (*it)->location, find_all, always );
        bfs.setMaxDepth( 8 );
        bfs.traverse();
        Debug::stream() << "finding all ants around " << (*it)->location << std::endl;
        Debug::stream() << "    allies:" <<  std::endl;
        for( std::vector<Ant*>::iterator it = find_all.allies.begin(); it != find_all.allies.end(); ++it )
            Debug::stream() << "        " <<  **it <<  std::endl;
       
        Debug::stream() << "    enemies:" <<  std::endl;
        for( std::vector< std::pair<int, Location> >::iterator it = find_all.enemies.begin();
             it != find_all.enemies.end();
             ++it )
            Debug::stream() << "        " <<  it->first << ":" << it->second <<  std::endl;


        if( !find_all.enemies.empty() )
        {
            std::vector<Location> locs;
            for( std::vector< std::pair<int, Location> >::iterator it = find_all.enemies.begin();
                 it != find_all.enemies.end();
                 ++it )
                locs.push_back( it->second );

            Location enemy_centroid = map.computeCentroid( locs );
            if( find_all.enemies.size() >= find_all.allies.size() )
            { 
                for( std::vector<Ant*>::iterator it = find_all.allies.begin(); it != find_all.allies.end(); ++it )
                {
                    Ant* ant = *it;
                    Direction d = map.getDirection( enemy_centroid, ant->location );
                    // TODO: check if the dir is blocked
                    ant->path.assign( map.getLocation( ant->location, d  ), &d, &d+1 );
                    ant->path.setGoal( Path::ATTACK );
                }
            }
            else
            {
                for( std::vector<Ant*>::iterator it = find_all.allies.begin(); it != find_all.allies.end(); ++it )
                {
                    Ant* ant = *it;
                    Direction d = map.getDirection( ant->location, enemy_centroid );
                    ant->path.assign( map.getLocation( ant->location, d ), &d, &d+1 );
                    ant->path.setGoal( Path::ATTACK );
                }
            }
        }
    }
    */
}
