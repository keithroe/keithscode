

#include <cstring>
#include <map>

#include "Ant.h"
#include "BF.h"
#include "Battle.h"
#include "Debug.h"
#include "Map.h"

namespace
{
    const int MY_ANT_ID = 0;

    typedef std::vector<Ant*>                               Ants;
    typedef std::set<Ant*>                                  AntSet;

    typedef std::vector<Location>                           Locations;
    typedef std::set<Location>                              LocationSet;

    typedef std::vector< std::pair<Location, Locations> >   AntEnemies;

    typedef std::vector<Direction>                          Directions;


    // TODO: for now, only take into account my ants as enemies of enemies
    //       need to add in other players ants as enemies of enemies too?
    //       just run EnemyCombatants from all enemies found by first pass.
    struct AllEnemyAnts 
    {
        AllEnemyAnts( const Location& location, int ant_id, AntEnemies& ant_enemies ) 
            : location( location ),
              ant_id( ant_id ),
              ant_enemies( ant_enemies ),
              found_enemy( false )
        {}

        bool operator()( const BFNode* node )
        { 
            int other_ant_id = node->square->ant_id;
            if(  other_ant_id >= 0 && other_ant_id != ant_id  )
            {
                if( !found_enemy ) // First enemy found for this ant
                {
                    ant_enemies.push_back( std::make_pair(location, Locations() ) );
                    found_enemy = true;
                }
                ant_enemies.back().second.push_back( node->loc );
            }
            return true;
        }
        
        const Location      location;
        const int           ant_id;
        AntEnemies&         ant_enemies;
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


    void fill( int*** grid, unsigned width, unsigned height, const Location& location, int inc)
    {
        // TODO: should extend this to battle zone+1 ring

        Location x = clamp( Location( location.row-2, location.col-1 ), height, width );
        grid[ x.row ][ x.col ][ 0 ] += inc;
        tile( x );
        x = clamp( Location( location.row-2, location.col-0 ), height, width );
        grid[ x.row ][ x.col ][ 0 ] += inc;
        tile( x );
        x = clamp( Location( location.row-2, location.col+1 ), height, width );
        grid[ x.row ][ x.col ][ 0 ] += inc;
        tile( x );

        x = clamp( Location( location.row-1, location.col-2 ), height, width );
        grid[ x.row ][ x.col ][ 0 ] += inc;
        tile( x );
        x = clamp( Location( location.row-1, location.col-1 ), height, width );
        grid[ x.row ][ x.col ][ 0 ] += inc;
        tile( x );
        x = clamp( Location( location.row-1, location.col-0 ), height, width );
        grid[ x.row ][ x.col ][ 0 ] += inc;
        tile( x );
        x = clamp( Location( location.row-1, location.col+1 ), height, width );
        grid[ x.row ][ x.col ][ 0 ] += inc;
        tile( x );
        x = clamp( Location( location.row-1, location.col+2 ), height, width );
        grid[ x.row ][ x.col ][ 0 ] += inc;
        tile( x );

        x = clamp( Location( location.row-0, location.col-2 ), height, width );
        grid[ x.row ][ x.col ][ 0 ] += inc;
        tile( x );
        x = clamp( Location( location.row-0, location.col-1 ), height, width );
        grid[ x.row ][ x.col ][ 0 ] += inc;
        tile( x );
        x = clamp( Location( location.row-0, location.col-0 ), height, width );
        grid[ x.row ][ x.col ][ 0 ] += inc;
        tile( x );
        x = clamp( Location( location.row-0, location.col+1 ), height, width );
        grid[ x.row ][ x.col ][ 0 ] += inc;
        tile( x );
        x = clamp( Location( location.row-0, location.col+2 ), height, width );
        grid[ x.row ][ x.col ][ 0 ] += inc;
        tile( x );

        x = clamp( Location( location.row+1, location.col-2 ), height, width );
        grid[ x.row ][ x.col ][ 0 ] += inc;
        tile( x );
        x = clamp( Location( location.row+1, location.col-1 ), height, width );
        grid[ x.row ][ x.col ][ 0 ] += inc;
        tile( x );
        x = clamp( Location( location.row+1, location.col-0 ), height, width );
        grid[ x.row ][ x.col ][ 0 ] += inc;
        tile( x );
        x = clamp( Location( location.row+1, location.col+1 ), height, width );
        grid[ x.row ][ x.col ][ 0 ] += inc;
        tile( x );
        x = clamp( Location( location.row+1, location.col+2 ), height, width );
        grid[ x.row ][ x.col ][ 0 ] += inc;
        tile( x );

        x = clamp( Location( location.row+2, location.col-1 ), height, width );
        grid[ x.row ][ x.col ][ 0 ] += inc;
        tile( x );
        x = clamp( Location( location.row+2, location.col-0 ), height, width );
        grid[ x.row ][ x.col ][ 0 ] += inc;
        tile( x );
        x = clamp( Location( location.row+2, location.col+1 ), height, width );
        grid[ x.row ][ x.col ][ 0 ] += inc;
        tile( x );
    }

    void fillPlusOne( int*** grid, int width, int height, const Location& location, int ant_id, int inc )
    {
        // TODO: only expand the zone in directions which do not lead to water
        // TODO: how can we better represent enemy ant location on next move???
        Location x = clamp( Location( location.row-3, location.col-1 ), height, width );
        grid[ x.row ][ x.col ][ ant_id ] += inc;
        tile( x );
        x = clamp( Location( location.row-3, location.col-0 ), height, width );
        grid[ x.row ][ x.col ][ ant_id ] += inc;
        tile( x );
        x = clamp( Location( location.row-3, location.col+1 ), height, width );
        grid[ x.row ][ x.col ][ ant_id ] += inc;
        tile( x );

        x = clamp( Location( location.row-2, location.col-2 ), height, width );
        grid[ x.row ][ x.col ][ ant_id ] += inc;
        tile( x );
        x = clamp( Location( location.row-2, location.col-1 ), height, width );
        grid[ x.row ][ x.col ][ ant_id ] += inc;
        tile( x );
        x = clamp( Location( location.row-2, location.col-0 ), height, width );
        grid[ x.row ][ x.col ][ ant_id ] += inc;
        tile( x );
        x = clamp( Location( location.row-2, location.col+1 ), height, width );
        grid[ x.row ][ x.col ][ ant_id ] += inc;
        tile( x );
        x = clamp( Location( location.row-2, location.col+2 ), height, width );
        grid[ x.row ][ x.col ][ ant_id ] += inc;
        tile( x );

        x = clamp( Location( location.row-1, location.col-3 ), height, width );
        grid[ x.row ][ x.col ][ ant_id ] += inc;
        tile( x );
        x = clamp( Location( location.row-1, location.col-2 ), height, width );
        grid[ x.row ][ x.col ][ ant_id ] += inc;
        tile( x );
        x = clamp( Location( location.row-1, location.col-1 ), height, width );
        grid[ x.row ][ x.col ][ ant_id ] += inc;
        tile( x );
        x = clamp( Location( location.row-1, location.col-0 ), height, width );
        grid[ x.row ][ x.col ][ ant_id ] += inc;
        tile( x );
        x = clamp( Location( location.row-1, location.col+1 ), height, width );
        grid[ x.row ][ x.col ][ ant_id ] += inc;
        tile( x );
        x = clamp( Location( location.row-1, location.col+2 ), height, width );
        grid[ x.row ][ x.col ][ ant_id ] += inc;
        tile( x );
        x = clamp( Location( location.row-1, location.col+3 ), height, width );
        grid[ x.row ][ x.col ][ ant_id ] += inc;
        tile( x );
        
        x = clamp( Location( location.row-0, location.col-3 ), height, width );
        grid[ x.row ][ x.col ][ ant_id ] += inc;
        tile( x );
        x = clamp( Location( location.row-0, location.col-2 ), height, width );
        grid[ x.row ][ x.col ][ ant_id ] += inc;
        tile( x );
        x = clamp( Location( location.row-0, location.col-1 ), height, width );
        grid[ x.row ][ x.col ][ ant_id ] += inc;
        tile( x );
        x = clamp( Location( location.row-0, location.col-0 ), height, width );
        grid[ x.row ][ x.col ][ ant_id ] += inc;
        tile( x );
        x = clamp( Location( location.row-0, location.col+1 ), height, width );
        grid[ x.row ][ x.col ][ ant_id ] += inc;
        tile( x );
        x = clamp( Location( location.row-0, location.col+2 ), height, width );
        grid[ x.row ][ x.col ][ ant_id ] += inc;
        tile( x );
        x = clamp( Location( location.row-0, location.col+3 ), height, width );
        grid[ x.row ][ x.col ][ ant_id ] += inc;
        tile( x );
        
        x = clamp( Location( location.row+1, location.col-3 ), height, width );
        grid[ x.row ][ x.col ][ ant_id ] += inc;
        tile( x );
        x = clamp( Location( location.row+1, location.col-2 ), height, width );
        grid[ x.row ][ x.col ][ ant_id ] += inc;
        tile( x );
        x = clamp( Location( location.row+1, location.col-1 ), height, width );
        grid[ x.row ][ x.col ][ ant_id ] += inc;
        tile( x );
        x = clamp( Location( location.row+1, location.col-0 ), height, width );
        grid[ x.row ][ x.col ][ ant_id ] += inc;
        tile( x );
        x = clamp( Location( location.row+1, location.col+1 ), height, width );
        grid[ x.row ][ x.col ][ ant_id ] += inc;
        tile( x );
        x = clamp( Location( location.row+1, location.col+2 ), height, width );
        grid[ x.row ][ x.col ][ ant_id ] += inc;
        tile( x );
        x = clamp( Location( location.row+1, location.col+3 ), height, width );
        grid[ x.row ][ x.col ][ ant_id ] += inc;
        tile( x );

        x = clamp( Location( location.row+2, location.col-2 ), height, width );
        grid[ x.row ][ x.col ][ ant_id ] += inc;
        tile( x );
        x = clamp( Location( location.row+2, location.col-1 ), height, width );
        grid[ x.row ][ x.col ][ ant_id ] += inc;
        tile( x );
        x = clamp( Location( location.row+2, location.col-0 ), height, width );
        grid[ x.row ][ x.col ][ ant_id ] += inc;
        tile( x );
        x = clamp( Location( location.row+2, location.col+1 ), height, width );
        grid[ x.row ][ x.col ][ ant_id ] += inc;
        tile( x );
        x = clamp( Location( location.row+2, location.col+2 ), height, width );
        grid[ x.row ][ x.col ][ ant_id ] += inc;
        tile( x );
        
        x = clamp( Location( location.row+3, location.col-1 ), height, width );
        grid[ x.row ][ x.col ][ ant_id ] += inc;
        tile( x );
        x = clamp( Location( location.row+3, location.col-0 ), height, width );
        grid[ x.row ][ x.col ][ ant_id ] += inc;
        tile( x );
        x = clamp( Location( location.row+3, location.col+1 ), height, width );
        grid[ x.row ][ x.col ][ ant_id ] += inc;
        tile( x );
    }


    // TODO: going to need incremental updates to score???
    int score( const Map& map, const AntEnemies& ant_enemies, const Directions& moves, int*** grid )
    {
        LocationSet dead_enemies; 
        int         current_score=0;

        int idx = 0;
        for( AntEnemies::const_iterator it = ant_enemies.begin(); it != ant_enemies.end(); ++it )
        {
            // add up my_enemy_count for moved position
            Location loc   = map.getLocation( it->first, moves[ idx++ ] );
            int my_enemies = grid[ loc.row ][ loc.col ][ 1 ];
            my_enemies    += grid[ loc.row ][ loc.col ][ 2 ];
            my_enemies    += grid[ loc.row ][ loc.col ][ 3 ];
            my_enemies    += grid[ loc.row ][ loc.col ][ 4 ];
            my_enemies    += grid[ loc.row ][ loc.col ][ 5 ];
            my_enemies    += grid[ loc.row ][ loc.col ][ 6 ];
            my_enemies    += grid[ loc.row ][ loc.col ][ 7 ];
            my_enemies    += grid[ loc.row ][ loc.col ][ 8 ];
            my_enemies    += grid[ loc.row ][ loc.col ][ 9 ];
            
            // for each of my nearby_enemies within dist2 of 10 (they can step into our zone)
            bool my_ant_dies = false;
            const Locations& enemies = it->second;
            for( Locations::const_iterator enemy = enemies.begin(); enemy != enemies.end(); ++enemy )
            {
                Location enemy_loc = *enemy;
                if( map.distance2( loc, enemy_loc ) <= 10 ) // TODO: magic number
                {
                    // Potentially in range.  Count this enemy's enemy count
                    // (for now approximate as just ant_id zero enemies)
                    int enemy_enemies = grid[ enemy_loc.row ][ enemy_loc.col ][ MY_ANT_ID ];
                    if( my_enemies >= enemy_enemies ) my_ant_dies = true; 
                    if( enemy_enemies >= my_enemies ) dead_enemies.insert( enemy_loc );
                }
            }
            current_score -= static_cast<int>( my_ant_dies );
        }

        current_score += dead_enemies.size();
        return current_score;
    }
    

    inline unsigned rnd()
    {
        static unsigned seed = 12345u;
        const unsigned A = 1664525u;
        const unsigned C = 1013904223u;
        seed = A*seed + C;
        return seed & 0x00FFFFFF;
    }


    inline float rndf()
    {
        return static_cast<float>( rnd() )/ static_cast<float>( 0x01000000 );
    }


    inline float lerp( float a, float b, float t )
    {
        return a + ( b - a ) * t; 
    }


    // temp must be in [0, 1]
    float P( int old_score, int new_score, float temp )
    {
        // At temp 1.0 (max temp)
        //   score increase => p = 0.65
        //   score match    => p = 0.50 
        //   score decrease => p = 0.35
        // At temp 0.0 (min temp)
        //   score increase => p = 1.00 
        //   score match    => p = 0.50 
        //   score decrease => p = 0.00
        const float t = new_score > old_score ? 1.0f : 
                        new_score < old_score ? 0.0f : 
                        0.5f;
        const float offset = lerp( 0.0f, 0.35f, temp );
        return lerp( 0.0f+offset, 1.0f-offset, t );

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
    

    //
    // Search from each of our ants to find enemies within 2 steps of battle
    //
    AntEnemies ally_enemies;
    for( Ants::const_iterator it = ants.begin(); it != ants.end(); ++it )
    {
        Location location = (*it )->location;
        AllEnemyAnts    enemy_ants( location, MY_ANT_ID, ally_enemies );
        BattlePeriphery periphery( map, location );
        EnemyCombatants bfs( map, location, enemy_ants, periphery );
        
        //Debug::stream() << "Battle: searching from " << location << std::endl;
        bfs.traverse();
    }

    if( ally_enemies.empty() ) return; // No potential battles found

    Debug::stream() << "Battle:   found " << ally_enemies.size() << " ally ants in battle" << std::endl;

    //
    // Gather all enemies seen
    //
    LocationSet enemies;
    for( AntEnemies::const_iterator it = ally_enemies.begin(); it != ally_enemies.end(); ++it )
    {
        const Locations& ant_enemies = it->second;
        for( Locations::const_iterator enemy = ant_enemies.begin(); enemy != ant_enemies.end(); ++enemy )
        {
            enemies.insert( *enemy );
        }
    }
    
    Debug::stream() << "Battle:   found " << enemies.size() << " enemy ants in battle" << std::endl;
    
    
    //
    // create grid
    //
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
    

    //
    // Splat the ants into the grid
    //

    setFillColor( 0, 0, 255, 0.1f );
    for( AntEnemies::const_iterator it = ally_enemies.begin(); it != ally_enemies.end(); ++it )
    {
        fillPlusOne( grid, width, height, it->first, MY_ANT_ID , 1 );
        circle( it->first, 1, true );
    }
    
    setFillColor( 255, 0, 0, 0.1f );
    for( LocationSet::const_iterator it = enemies.begin(); it != enemies.end(); ++it )
    {
        fillPlusOne( grid, width, height, *it, map( *it ).ant_id, 1 );
        circle( *it, 1, true );
    }
    setFillColor( 0, 0, 0, 0.0f );

    //
    // Run simulated annealing on the ants
    //
    // TODO: bias the ants to attack
    const int MAX_ITERATIONS = 20;

    Directions moves     ( ally_enemies.size(), NONE );
    Directions best_moves( ally_enemies.size(), NONE );

    int max_score = score( map, ally_enemies, moves, grid );
    int cur_score = max_score;

    Debug::stream() << "Battle: initial score is " << max_score << std::endl;

    // TODO: any benefit to randomizing?
    std::vector<int> transition_order( ally_enemies.size() );
    for( unsigned i = 0; i < ally_enemies.size(); ++i ) transition_order[i] = i;
    std::random_shuffle( transition_order.begin(), transition_order.end() );

    for( int i = 0; i < MAX_ITERATIONS; ++i )
    {
        // TODO: check if move is available 
        int       rant     = transition_order[ i % ally_enemies.size() ];
        Location  rant_loc = ally_enemies[ rant ].first;
        Direction cur_move = moves[ rant ];
        Direction new_move = static_cast<Direction>( ( cur_move + rand()%4 + 1 ) % 5 );
        Location  new_loc  = map.getLocation( rant_loc, new_move );

        // Make sure we get an available move 
        int tries = 0;
        while( new_move != NONE && !map( new_loc ).isAvailable() && tries < 3 )
        {
            new_move = static_cast<Direction>( (new_move+1) % 5 );
            if( new_move == cur_move ) // Skip over cur_move
                new_move = static_cast<Direction>( (new_move+1) % 5 );
            new_loc = map.getLocation( rant_loc, new_move );
            tries++;
        }

        Debug::stream() << "Battle:   trying move of " << rant_loc << " in " << DIRECTION_CHAR[new_move] << std::endl;

        // Make the move
        fillPlusOne( grid, width, height, map.getLocation( rant_loc, cur_move ), MY_ANT_ID, -1 );
        fillPlusOne( grid, width, height, new_loc,                               MY_ANT_ID, +1 );
        moves[ rant ] = new_move;
        
        // Check if we have a new best state
        const int new_score = score( map, ally_enemies, moves, grid );
        Debug::stream() << "Battle:      score: " << new_score << std::endl; 
        if( new_score > max_score || ( new_score == max_score && cur_move == NONE ) ) // Prefer movement
        {
            Debug::stream() << "Battle:  found new better score " << cur_score << std::endl;
            max_score = new_score;
            best_moves.assign( moves.begin(), moves.end() );
        }

        // Stochastically choose whether we accept this move, based on score and cooling schedule
        float p = P( cur_score, new_score, static_cast<float>( i ) / MAX_ITERATIONS ); 
        float e = rndf();
        Debug::stream() << "Battle       p: " << p << " rnd: " << e << std::endl;
        if( p > e )
        {
            Debug::stream() << "Battle:          accepting transition" << cur_score << std::endl;
            cur_score     = new_score;
        }
        else
        {
            // Undo the last move
            Debug::stream() << "Battle:          rejecting transition" << cur_score << std::endl;
            fillPlusOne( grid, width, height, map.getLocation( rant_loc, moves[ rant ] ), MY_ANT_ID, -1 );
            fillPlusOne( grid, width, height, map.getLocation( rant_loc, cur_move      ), MY_ANT_ID, +1 );
            moves[ rant ] = cur_move;
        }
    }
    Debug::stream() << "Battle:  max score is" << max_score << std::endl;

    //
    // Assign paths for all battle ants
    //
    for( unsigned i = 0; i < ally_enemies.size(); ++i )
    {
        Location ant_location = ally_enemies[i].first;
        Ant* ant = map( ant_location ).ant;
        ant->path.assign( map.getLocation( ant_location, moves[i] ), best_moves[i], Path::ATTACK );
        assigned.insert( ant );
        
        Debug::stream() << "Battle:     moving ant at " << ant_location << " in " << DIRECTION_CHAR[ best_moves[i] ] << std::endl;
    }


    //
    // Clean up
    //
    for( unsigned i = 0; i < height; ++i )
    {
        for( unsigned j = 0; j < width; ++j )
            delete [] grid[i][j];
        delete grid[i];
    }
    delete grid;
    grid = 0u;

    /*
    AntEnemies enemy_enemies;
    for( Ants::const_iterator it = ants.begin(); it != ants.end(); ++it )
    {
        Location location = (*it )->location;
        AllEnemyAnts    enemy_ants( location, 0, ally_enemies );
        BattlePeriphery periphery( map, location );
        EnemyCombatants bfs( map, location, enemy_ants, periphery );
        
        Debug::stream() << "    searching from " << location << std::endl;
        bfs.traverse();
    }

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

    */


   
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
