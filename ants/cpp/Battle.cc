

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
    
    void fillWithColor( int*** grid, unsigned width, unsigned height, const Location& location, int ant_id, int inc)
    {
        // TODO: only expand the zone in directions which do not lead to water
        Location x = clamp( Location( location.row-2, location.col-1 ), height, width );
        grid[ x.row ][ x.col ][ ant_id ] += inc;
        tile( x );
        x = clamp( Location( location.row-2, location.col-0 ), height, width );
        grid[ x.row ][ x.col ][ ant_id ] += inc;
        tile( x );
        x = clamp( Location( location.row-2, location.col+1 ), height, width );
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

        x = clamp( Location( location.row+2, location.col-1 ), height, width );
        grid[ x.row ][ x.col ][ ant_id ] += inc;
        tile( x );
        x = clamp( Location( location.row+2, location.col-0 ), height, width );
        grid[ x.row ][ x.col ][ ant_id ] += inc;
        tile( x );
        x = clamp( Location( location.row+2, location.col+1 ), height, width );
        grid[ x.row ][ x.col ][ ant_id ] += inc;
        tile( x );
    }


    void fill( int*** grid, unsigned width, unsigned height, const Location& location, int ant_id, int inc)
    {
        Location x = clamp( Location( location.row-2, location.col-1 ), height, width );
        grid[ x.row ][ x.col ][ ant_id ] += inc;
        x = clamp( Location( location.row-2, location.col-0 ), height, width );
        grid[ x.row ][ x.col ][ ant_id ] += inc;
        x = clamp( Location( location.row-2, location.col+1 ), height, width );
        grid[ x.row ][ x.col ][ ant_id ] += inc;

        x = clamp( Location( location.row-1, location.col-2 ), height, width );
        grid[ x.row ][ x.col ][ ant_id ] += inc;
        x = clamp( Location( location.row-1, location.col-1 ), height, width );
        grid[ x.row ][ x.col ][ ant_id ] += inc;
        x = clamp( Location( location.row-1, location.col-0 ), height, width );
        grid[ x.row ][ x.col ][ ant_id ] += inc;
        x = clamp( Location( location.row-1, location.col+1 ), height, width );
        grid[ x.row ][ x.col ][ ant_id ] += inc;
        x = clamp( Location( location.row-1, location.col+2 ), height, width );
        grid[ x.row ][ x.col ][ ant_id ] += inc;

        x = clamp( Location( location.row-0, location.col-2 ), height, width );
        grid[ x.row ][ x.col ][ ant_id ] += inc;
        x = clamp( Location( location.row-0, location.col-1 ), height, width );
        grid[ x.row ][ x.col ][ ant_id ] += inc;
        x = clamp( Location( location.row-0, location.col-0 ), height, width );
        grid[ x.row ][ x.col ][ ant_id ] += inc;
        x = clamp( Location( location.row-0, location.col+1 ), height, width );
        grid[ x.row ][ x.col ][ ant_id ] += inc;
        x = clamp( Location( location.row-0, location.col+2 ), height, width );
        grid[ x.row ][ x.col ][ ant_id ] += inc;

        x = clamp( Location( location.row+1, location.col-2 ), height, width );
        grid[ x.row ][ x.col ][ ant_id ] += inc;
        x = clamp( Location( location.row+1, location.col-1 ), height, width );
        grid[ x.row ][ x.col ][ ant_id ] += inc;
        x = clamp( Location( location.row+1, location.col-0 ), height, width );
        grid[ x.row ][ x.col ][ ant_id ] += inc;
        x = clamp( Location( location.row+1, location.col+1 ), height, width );
        grid[ x.row ][ x.col ][ ant_id ] += inc;
        x = clamp( Location( location.row+1, location.col+2 ), height, width );
        grid[ x.row ][ x.col ][ ant_id ] += inc;

        x = clamp( Location( location.row+2, location.col-1 ), height, width );
        grid[ x.row ][ x.col ][ ant_id ] += inc;
        x = clamp( Location( location.row+2, location.col-0 ), height, width );
        grid[ x.row ][ x.col ][ ant_id ] += inc;
        x = clamp( Location( location.row+2, location.col+1 ), height, width );
        grid[ x.row ][ x.col ][ ant_id ] += inc;
    }

    void fillPlusOne( int*** grid, const Map& map, int width, int height, const Location& location, int ant_id, int inc )
    {
        fillWithColor( grid, width, height, location, ant_id, inc);

        // The four move locations
        Location x_n = clamp( Location( location.row-1, location.col+0 ), height, width );
        Location x_s = clamp( Location( location.row+1, location.col+0 ), height, width );
        Location x_e = clamp( Location( location.row+0, location.col+1 ), height, width );
        Location x_w = clamp( Location( location.row+0, location.col-1 ), height, width );

        if( !isWaterOrFood( map( x_n ) ) )
        {
            Location x = clamp( Location( location.row-3, location.col-1 ), height, width );
            grid[ x.row ][ x.col ][ ant_id ] += inc;
            tile( x );
            x = clamp( Location( location.row-3, location.col-0 ), height, width );
            grid[ x.row ][ x.col ][ ant_id ] += inc;
            tile( x );
            x = clamp( Location( location.row-3, location.col+1 ), height, width );
            grid[ x.row ][ x.col ][ ant_id ] += inc;
            tile( x );
        }

        if( !isWaterOrFood( map( x_s ) ) )
        {
            Location x = clamp( Location( location.row+3, location.col-1 ), height, width );
            grid[ x.row ][ x.col ][ ant_id ] += inc;
            tile( x );
            x = clamp( Location( location.row+3, location.col-0 ), height, width );
            grid[ x.row ][ x.col ][ ant_id ] += inc;
            tile( x );
            x = clamp( Location( location.row+3, location.col+1 ), height, width );
            grid[ x.row ][ x.col ][ ant_id ] += inc;
            tile( x );
            
        }
        
        if( !isWaterOrFood( map( x_w ) ) )
        {
            Location x = clamp( Location( location.row-1, location.col-3 ), height, width );
            grid[ x.row ][ x.col ][ ant_id ] += inc;
            tile( x );
            x = clamp( Location( location.row+0, location.col-3 ), height, width );
            grid[ x.row ][ x.col ][ ant_id ] += inc;
            tile( x );
            x = clamp( Location( location.row+1, location.col-3 ), height, width );
            grid[ x.row ][ x.col ][ ant_id ] += inc;
            tile( x );
        }
            
        if( !isWaterOrFood( map( x_e ) ) )
        {
            Location x = clamp( Location( location.row-1, location.col+3 ), height, width );
            grid[ x.row ][ x.col ][ ant_id ] += inc;
            tile( x );
            x = clamp( Location( location.row+0, location.col+3 ), height, width );
            grid[ x.row ][ x.col ][ ant_id ] += inc;
            tile( x );
            x = clamp( Location( location.row+1, location.col+3 ), height, width );
            grid[ x.row ][ x.col ][ ant_id ] += inc;
            tile( x );
        }

        if( !isWaterOrFood( map( x_e ) ) || !isWaterOrFood( map( x_n ) ) )
        {
            Location x = clamp( Location( location.row+2, location.col+2 ), height, width );
            grid[ x.row ][ x.col ][ ant_id ] += inc;
            tile( x );
        }
        
        if( !isWaterOrFood( map( x_w ) ) || !isWaterOrFood( map( x_n ) ) )
        {
            Location x = clamp( Location( location.row-2, location.col+2 ), height, width );
            grid[ x.row ][ x.col ][ ant_id ] += inc;
            tile( x );
        }

        if( !isWaterOrFood( map( x_w ) ) || !isWaterOrFood( map( x_s ) ) )
        {
            Location x = clamp( Location( location.row-2, location.col-2 ), height, width );
            grid[ x.row ][ x.col ][ ant_id ] += inc;
            tile( x );
        }

        if( !isWaterOrFood( map( x_e ) ) || !isWaterOrFood( map( x_s ) ) )
        {
            Location x = clamp( Location( location.row+2, location.col-2 ), height, width );
            grid[ x.row ][ x.col ][ ant_id ] += inc;
            tile( x );
        }
    }


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
            
            Debug::stream() << "scoring: " << loc << std::endl
                            << "     my_enemies: " << my_enemies << std::endl;

            
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
                    Debug::stream() << "    enemy " << enemy_loc <<  ": " << enemy_enemies << std::endl;
                }
            }
            current_score -= static_cast<int>( my_ant_dies );
            
            Debug::stream() << "        current_score " << current_score << std::endl;

        }

        current_score += dead_enemies.size();
        Debug::stream() << "    dead enemies " << dead_enemies.size() << std::endl;
        Debug::stream() << "    final score  " << current_score << std::endl;
        return current_score;
    }
    

    inline unsigned rnd()
    {
        static unsigned seed = 1234567u;
        const unsigned A = 1664525u;
        const unsigned C = 1013904223u;
        seed = A*seed + C;
        return seed;
    }


    inline float rndf()
    {
        return static_cast<float>( rnd() )/ 4294967296.0f;
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
    // TODO: move this into init code
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
        fillPlusOne( grid, map, width, height, it->first, MY_ANT_ID, 1 );
        circle( it->first, 1, true );
    }
    
    setFillColor( 255, 0, 0, 0.1f );
    for( LocationSet::const_iterator it = enemies.begin(); it != enemies.end(); ++it )
    {
        fillPlusOne( grid, map, width, height, *it, map( *it ).ant_id, 1 );
        circle( *it, 1, true );
    }
    setFillColor( 0, 0, 0, 0.0f );


    //
    // Divide into clusters
    //
    std::vector< std::vector<Location> >( enemies.size() );
    for( LocationSet::const_iterator it = enemies.begin(); it != enemies.end(); ++it )
    {

    }



    //
    // Run simulated annealing on each cluster 
    //
    const int MAX_ITERATIONS = 1000;

    Directions cur_moves ( ally_enemies.size(), NONE );
    Directions best_moves( ally_enemies.size(), NONE );
    int        cur_score  = score( map, ally_enemies, cur_moves, grid );
    int        best_score = cur_score;

    Debug::stream() << "Battle: initial score is " << cur_score << std::endl;

    LocationSet original_squares;
    LocationSet occupied_squares;
    for( AntEnemies::const_iterator it = ally_enemies.begin(); it != ally_enemies.end(); ++it )
    {
        original_squares.insert( it->first );
        occupied_squares.insert( it->first );
    }
    
    // TODO: any benefit to randomizing?
    std::vector<int> transition_order( ally_enemies.size() );
    for( unsigned i = 0; i < ally_enemies.size(); ++i ) transition_order[i] = i;
    std::random_shuffle( transition_order.begin(), transition_order.end() );

    for( int i = 0; i < MAX_ITERATIONS; ++i )
    {
        // Select ant to transition
        int       rant     = transition_order[ i % ally_enemies.size() ];
        Location  orig_loc = ally_enemies[ rant ].first;

        // Get the ants current position
        Direction cur_move = cur_moves[ rant ];
        Location  cur_loc  = map.getLocation( orig_loc, cur_move );

        // Select a transition
        Direction new_move = static_cast<Direction>( ( cur_move + rand()%4 + 1 ) % 5 );
        Location  new_loc  = map.getLocation( orig_loc, new_move );

        // Make sure the transition is viable 
        Debug::stream() << "  Transitioning ant " << orig_loc << " cur: " <<  DIRECTION_CHAR[ cur_move ] 
                        << " best: " << DIRECTION_CHAR[ best_moves[ rant ] ] << std::endl;
        Debug::stream() << "     first trying move " << DIRECTION_CHAR[ new_move ] << std::endl;
        int tries = 0;
        while( ( ( !map( new_loc ).isAvailable() && original_squares.find( new_loc ) == original_squares.end() ) || 
               occupied_squares.find( new_loc ) != occupied_squares.end() ) && 
               tries < 3 )
        {  
            // TODO: this is broken if ant surrounded (no valid move).  need to allow 'no transition'
            //       occupied_squares check will fail for 'no transition' as well
            new_move = static_cast<Direction>( (new_move+1) % 5 );
            if( new_move == cur_move ) // Skip over cur_move
                new_move = static_cast<Direction>( (new_move+1) % 5 );
            Debug::stream() << "       switching to move " << DIRECTION_CHAR[ new_move ] << std::endl;
            new_loc = map.getLocation( orig_loc, new_move );
            tries++;
        }

        // Make the move
        occupied_squares.erase ( cur_loc );
        occupied_squares.insert( new_loc );
        fillPlusOne( grid, map, width, height, cur_loc, MY_ANT_ID, -1 );
        fillPlusOne( grid, map, width, height, new_loc, MY_ANT_ID, +1 );
        cur_moves[ rant ] = new_move;
        
        // Check if we have a new best state
        const int new_score = score( map, ally_enemies, cur_moves, grid );
        Debug::stream() << "Battle:      score: " << new_score << " best: " << best_score << std::endl; 
        if( new_score > best_score || ( new_score == best_score && cur_move == NONE ) ) // Prefer movement
        {
            Debug::stream() << "Battle:  found new better score " << new_score << std::endl;
            best_score = new_score;
            best_moves.assign( cur_moves.begin(), cur_moves.end() );
        }

        // Stochastically choose whether we accept this move, based on score and cooling schedule
        float p = P( cur_score, new_score, static_cast<float>( MAX_ITERATIONS - i ) / MAX_ITERATIONS ); 
        float e = rndf();
        Debug::stream() << "Battle       cur_score:" << cur_score << " new_score: " << new_score 
                        << " p: " << p << " rnd: " << e << std::endl;
        if( p > e )
        {
            Debug::stream() << "Battle:          accepting transition" << cur_score << std::endl;
            cur_score     = new_score;
        }
        else
        {
            // Undo the last move
            Debug::stream() << "Battle:          rejecting transition" << cur_score << std::endl;
            occupied_squares.erase ( new_loc );
            occupied_squares.insert( cur_loc );
            fillPlusOne( grid, map, width, height, new_loc, MY_ANT_ID, -1 );
            fillPlusOne( grid, map, width, height, cur_loc, MY_ANT_ID, +1 );
            cur_moves[ rant ] = cur_move;
        }
    }
    Debug::stream() << "Battle:  max score is" << best_score << std::endl;

    //
    // Assign paths for all battle ants
    //
    for( unsigned i = 0; i < ally_enemies.size(); ++i )
    {
        Location ant_location = ally_enemies[i].first;
        Ant* ant = map( ant_location ).ant;
        ant->path.assign( map.getLocation( ant_location, best_moves[i] ), best_moves[i], Path::ATTACK );
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

}
