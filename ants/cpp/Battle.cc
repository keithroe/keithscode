

#include <cstring>
#include <iterator>
#include <map>

#include "Ant.h"
#include "BF.h"
#include "Battle.h"
#include "Debug.h"
#include "Map.h"

namespace
{
    struct AllEnemyAnts 
    {
        AllEnemyAnts( const Location& location, int ant_id, Battle::AntEnemies& ant_enemies ) 
            : location( location ),
              ant_id( ant_id ),
              ant_enemies( ant_enemies ),
              found_enemy( false )
        {
        }


        bool operator()( const BFNode* node )
        { 
            int other_ant_id = node->square->ant_id;
            if( other_ant_id >= 0 && other_ant_id != ant_id  )
            {
                if( !found_enemy ) // First enemy found for this ant
                {
                    ant_enemies.push_back( std::make_pair(location, Battle::Locations() ) );
                    found_enemy = true;
                }
                ant_enemies.back().second.push_back( node->loc );
            }
            return true;
        }
        
        const Location       location;
        const int            ant_id;
        Battle::AntEnemies&  ant_enemies;
        bool                 found_enemy;
    };

    struct AnyEnemyAnts 
    {
        AnyEnemyAnts( int ant_id ) : ant_id( ant_id ), found_enemy( false ) {}

        bool operator()( const BFNode* node )
        { 
            int other_ant_id = node->square->ant_id;
            if( other_ant_id >= 0 && other_ant_id != ant_id  )
            {
                found_enemy = true;
                return false;
            }
            return true;
        }
        
        const int  ant_id;
        bool       found_enemy;
    };



    struct WithinDistance2 
    {
        WithinDistance2( const Map& map, const Location& origin, int dist2 ) 
            : map( map ), origin( origin ), dist2( dist2 ) {}

        bool operator()( const BFNode* current, const Location& location, const Square& neighbor )
        {
            return map.distance2( origin, location ) <= dist2;
        }
        const Map&     map;
        const Location origin;
        const int      dist2;
    };

    
    typedef BF<AllEnemyAnts, WithinDistance2> EnemyCombatants;
    typedef BF<AnyEnemyAnts, WithinDistance2> AnyEnemyCombatants;
    


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


}


//------------------------------------------------------------------------------
//
// Battle implementation
//
//------------------------------------------------------------------------------

void Battle::fill( const Location& location, int ant_id, int inc)
{
    const unsigned height = m_map.height();
    const unsigned width  = m_map.width();

    Location x = clamp( Location( location.row-2, location.col-1 ), height, width );
    m_grid[ x.row ][ x.col ][ ant_id ].b += inc;
    x = clamp( Location( location.row-2, location.col-0 ), height, width );
    m_grid[ x.row ][ x.col ][ ant_id ].b += inc;
    x = clamp( Location( location.row-2, location.col+1 ), height, width );
    m_grid[ x.row ][ x.col ][ ant_id ].b += inc;

    x = clamp( Location( location.row-1, location.col-2 ), height, width );
    m_grid[ x.row ][ x.col ][ ant_id ].b += inc;
    x = clamp( Location( location.row-1, location.col-1 ), height, width );
    m_grid[ x.row ][ x.col ][ ant_id ].a += inc;
    x = clamp( Location( location.row-1, location.col-0 ), height, width );
    m_grid[ x.row ][ x.col ][ ant_id ].a += inc;
    x = clamp( Location( location.row-1, location.col+1 ), height, width );
    m_grid[ x.row ][ x.col ][ ant_id ].a += inc;
    x = clamp( Location( location.row-1, location.col+2 ), height, width );
    m_grid[ x.row ][ x.col ][ ant_id ].b += inc;

    x = clamp( Location( location.row-0, location.col-2 ), height, width );
    m_grid[ x.row ][ x.col ][ ant_id ].b += inc;
    x = clamp( Location( location.row-0, location.col-1 ), height, width );
    m_grid[ x.row ][ x.col ][ ant_id ].a += inc;
    x = clamp( Location( location.row-0, location.col-0 ), height, width );
    m_grid[ x.row ][ x.col ][ ant_id ].a += inc;
    x = clamp( Location( location.row-0, location.col+1 ), height, width );
    m_grid[ x.row ][ x.col ][ ant_id ].a += inc;
    x = clamp( Location( location.row-0, location.col+2 ), height, width );
    m_grid[ x.row ][ x.col ][ ant_id ].b += inc;

    x = clamp( Location( location.row+1, location.col-2 ), height, width );
    m_grid[ x.row ][ x.col ][ ant_id ].b += inc;
    x = clamp( Location( location.row+1, location.col-1 ), height, width );
    m_grid[ x.row ][ x.col ][ ant_id ].a += inc;
    x = clamp( Location( location.row+1, location.col-0 ), height, width );
    m_grid[ x.row ][ x.col ][ ant_id ].a += inc;
    x = clamp( Location( location.row+1, location.col+1 ), height, width );
    m_grid[ x.row ][ x.col ][ ant_id ].a += inc;
    x = clamp( Location( location.row+1, location.col+2 ), height, width );
    m_grid[ x.row ][ x.col ][ ant_id ].b += inc;

    x = clamp( Location( location.row+2, location.col-1 ), height, width );
    m_grid[ x.row ][ x.col ][ ant_id ].b += inc;
    x = clamp( Location( location.row+2, location.col-0 ), height, width );
    m_grid[ x.row ][ x.col ][ ant_id ].b += inc;
    x = clamp( Location( location.row+2, location.col+1 ), height, width );
    m_grid[ x.row ][ x.col ][ ant_id ].b += inc;
}


void Battle::fillPlusOne( const Location& location, int ant_id, int inc )
{
    const unsigned height = m_map.height();
    const unsigned width  = m_map.width();

    fill( location, ant_id, inc);

    // The four move locations
    Location x_n = clamp( Location( location.row-1, location.col+0 ), height, width );
    Location x_s = clamp( Location( location.row+1, location.col+0 ), height, width );
    Location x_e = clamp( Location( location.row+0, location.col+1 ), height, width );
    Location x_w = clamp( Location( location.row+0, location.col-1 ), height, width );

    if( !isWaterOrFood( m_map( x_n ) ) )
    {
        Location x = clamp( Location( location.row-3, location.col-1 ), height, width );
        m_grid[ x.row ][ x.col ][ ant_id ].c += inc;
        x = clamp( Location( location.row-3, location.col-0 ), height, width );
        m_grid[ x.row ][ x.col ][ ant_id ].c += inc;
        x = clamp( Location( location.row-3, location.col+1 ), height, width );
        m_grid[ x.row ][ x.col ][ ant_id ].c += inc;
    }

    if( !isWaterOrFood( m_map( x_s ) ) )
    {
        Location x = clamp( Location( location.row+3, location.col-1 ), height, width );
        m_grid[ x.row ][ x.col ][ ant_id ].c += inc;
        x = clamp( Location( location.row+3, location.col-0 ), height, width );
        m_grid[ x.row ][ x.col ][ ant_id ].c += inc;
        x = clamp( Location( location.row+3, location.col+1 ), height, width );
        m_grid[ x.row ][ x.col ][ ant_id ].c += inc;
    }

    if( !isWaterOrFood( m_map( x_w ) ) )
    {
        Location x = clamp( Location( location.row-1, location.col-3 ), height, width );
        m_grid[ x.row ][ x.col ][ ant_id ].c += inc;
        x = clamp( Location( location.row+0, location.col-3 ), height, width );
        m_grid[ x.row ][ x.col ][ ant_id ].c += inc;
        x = clamp( Location( location.row+1, location.col-3 ), height, width );
        m_grid[ x.row ][ x.col ][ ant_id ].c += inc;
    }

    if( !isWaterOrFood( m_map( x_e ) ) )
    {
        Location x = clamp( Location( location.row-1, location.col+3 ), height, width );
        m_grid[ x.row ][ x.col ][ ant_id ].c += inc;
        x = clamp( Location( location.row+0, location.col+3 ), height, width );
        m_grid[ x.row ][ x.col ][ ant_id ].c += inc;
        x = clamp( Location( location.row+1, location.col+3 ), height, width );
        m_grid[ x.row ][ x.col ][ ant_id ].c += inc;
    }

    if( !isWaterOrFood( m_map( x_e ) ) || !isWaterOrFood( m_map( x_n ) ) )
    {
        Location x = clamp( Location( location.row+2, location.col+2 ), height, width );
        m_grid[ x.row ][ x.col ][ ant_id ].c += inc;
    }

    if( !isWaterOrFood( m_map( x_w ) ) || !isWaterOrFood( m_map( x_n ) ) )
    {
        Location x = clamp( Location( location.row-2, location.col+2 ), height, width );
        m_grid[ x.row ][ x.col ][ ant_id ].c += inc;
    }

    if( !isWaterOrFood( m_map( x_w ) ) || !isWaterOrFood( m_map( x_s ) ) )
    {
        Location x = clamp( Location( location.row-2, location.col-2 ), height, width );
        m_grid[ x.row ][ x.col ][ ant_id ].c += inc;
    }

    if( !isWaterOrFood( m_map( x_e ) ) || !isWaterOrFood( m_map( x_s ) ) )
    {
        Location x = clamp( Location( location.row+2, location.col-2 ), height, width );
        m_grid[ x.row ][ x.col ][ ant_id ].c += inc;
    }
}


Battle::Score Battle::score( const Battle::AntEnemies& ant_enemies, const Directions& moves )const
{
    LocationSet dead_enemies; 
    Score       current_score;

    int idx = 0;
    for( AntEnemies::const_iterator it = ant_enemies.begin(); it != ant_enemies.end(); ++it )
    {
        // add up my_enem, distancey_count for moved position
        Location loc   = m_map.getLocation( it->first, moves[ idx++ ] );

        EnemyCount my_enemies = m_grid[ loc.row ][ loc.col ][ 1 ];
        my_enemies += m_grid[ loc.row ][ loc.col ][ 2 ]; 
        my_enemies += m_grid[ loc.row ][ loc.col ][ 3 ];
        my_enemies += m_grid[ loc.row ][ loc.col ][ 4 ];
        my_enemies += m_grid[ loc.row ][ loc.col ][ 5 ];
        my_enemies += m_grid[ loc.row ][ loc.col ][ 6 ];
        my_enemies += m_grid[ loc.row ][ loc.col ][ 7 ];
        my_enemies += m_grid[ loc.row ][ loc.col ][ 8 ];
        my_enemies += m_grid[ loc.row ][ loc.col ][ 9 ];

        //Debug::stream() << "scoring: " << loc << std::endl
        //    << "     my_enemies: " << my_enemies << std::endl;


        // for each of my nearby_enemies within dist2 of 10 (they can step into our zone)
        bool my_ant_dies = false;
        int  depths = 0;
        const Locations& enemies = it->second;
        for( Locations::const_iterator enemy = enemies.begin(); enemy != enemies.end(); ++enemy )
        {
            Location enemy_loc = *enemy;
            if( m_map.distance2( loc, enemy_loc ) <= 10 ) // TODO: magic number
            {
                // Potentially in range.  Count this enemy's enemy count
                // (for now approximate as just ant_id zero enemies)
                EnemyCount enemy_enemies = m_grid[ enemy_loc.row ][ enemy_loc.col ][ MY_ANT_ID ];
                if( my_enemies.beats( enemy_enemies ) )
                { dead_enemies.insert( enemy_loc ); depths +=  my_enemies.deepest(); }
                if( enemy_enemies.beatsAlly( my_enemies ) )
                { my_ant_dies = true; }
                //Debug::stream() << "    enemy " << enemy_loc <<  ": " << enemy_enemies << std::endl
                //    << "             my_ant_dies = " << my_ant_dies << std::endl
                //    << "             enemy_dies  = " << my_enemies.beats( enemy_enemies ) << std::endl;

            }
        }
        current_score.ally_deaths   += static_cast<int>( my_ant_dies );
        current_score.attack_depths += depths;

        //Debug::stream() << "        current_score " << current_score << std::endl;

    }

    current_score.enemy_deaths = dead_enemies.size();
    //Debug::stream() << "    dead enemies " << dead_enemies.size() << std::endl;
    //Debug::stream() << "    final score  " << current_score << std::endl;
    return current_score;
}


void Battle::solve1v1( const Location& ally, const Location& enemy )
{
    //Debug::stream() << "Battle:  solve1v1 ... " << ally << " vs " << enemy << std::endl; 

    //
    // TODO: insert new location into occupied list
    //

    int max_dist = m_map.distance2( enemy, ally );
    Location  away_loc = ally;


    Locations neighbors;
    m_map.getNeighbors( ally, isAvailable, neighbors );

    if( max_dist > 10 )
    {
        // Stay within the one-ring
        for( Locations::iterator it = neighbors.begin(); it != neighbors.end(); ++it )
        {
            int dist = m_map.distance2( enemy, *it );
            if( dist > 10 && dist <= 17 ) 
            {
                max_dist = dist;
                away_loc = *it;
                break;
            }
        }
    }
    else
    {
        // Move away from the enemy
        for( Locations::iterator it = neighbors.begin(); it != neighbors.end(); ++it )
        {
            int dist = m_map.distance2( enemy, *it );
            if( dist > max_dist ) 
            {
                max_dist = dist;
                away_loc = *it;
            }
        }
    }

    Direction away = m_map.getDirection( ally, away_loc );

    Ant* ant = m_map( ally ).ant;
    ant->path.assign( away_loc, away, Path::ATTACK );
    m_allies.insert( ant );

    //Debug::stream() << "Battle:      solve1v1 moving ant " << ally << " in " << DIRECTION_CHAR[ away ] << std::endl;
}


// TODO: switch this to minimax scoring (return maximum ally deaths and associated enemy deaths then minimize this)
void Battle::countDeaths( const Location& ally0, const Location& ally1, const Location& enemy,
                          int& ally_deaths, int& enemy_deaths, int& distance )const
{
    int dist0           = m_map.distance2( ally0, enemy );
    int dist1           = m_map.distance2( ally1, enemy );
    int allies_in_range = static_cast<int>( dist0 <= 5 ) +
        static_cast<int>( dist1 <= 5 );
    enemy_deaths = static_cast<int>( allies_in_range >= 1 );
    ally_deaths  = static_cast<int>( allies_in_range == 1 );
    distance     = dist0 + dist1;

    for( int i = 0; i < 4; ++i )
    {
        Location moved_enemy = m_map.getLocation( enemy, static_cast<Direction>( i ) ); 
        int dist0           = m_map.distance2( ally0, moved_enemy );
        int dist1           = m_map.distance2( ally1, moved_enemy );
        int allies_in_range = static_cast<int>( dist0 <= 5 ) +
            static_cast<int>( dist1 <= 5 );
        enemy_deaths += static_cast<int>( allies_in_range >= 1 );
        ally_deaths  += static_cast<int>( allies_in_range == 1 );
        distance     += dist0 + dist1;
    }
}


void Battle::solve2v1( const Location& ally0, const Location& ally1, const Location& enemy )
{
    //Debug::stream() << "solve2v1 " << ally0 << "," << ally1 << " vs " << enemy << std::endl;

    Direction best_move0 = NONE;
    Direction best_move1 = NONE;
    Location  best_loc0 = ally0;
    Location  best_loc1 = ally1;
    int best_ally_dies = 10, best_enemy_dies = 0, best_dist = 1000, best_ally_dist = 1000;

    for( int i = 0; i < 5; ++i )
    {
        Location moved_ally0 = m_map.getLocation( ally0, static_cast<Direction>( i ) ); 
        if( !m_map( moved_ally0 ).isAvailable() && moved_ally0 != ally1 && i != NONE )
            continue;
        for( int j = 0; j < 5; ++j && i != NONE )
        {
            Location moved_ally1 = m_map.getLocation( ally1, static_cast<Direction>( j ) ); 
            if( moved_ally1 == moved_ally0 || ( !m_map( moved_ally1 ).isAvailable() && moved_ally1 != ally0 ) )
                continue;
            int ally_dies, enemy_dies, distance;
            countDeaths( moved_ally0, moved_ally1, enemy, ally_dies, enemy_dies, distance );
            int ally_dist = m_map.distance2( moved_ally1, moved_ally0 );

            //Debug::stream() << "    trying " << moved_ally0 << "," << moved_ally1 
            //    << " new allydies:" << ally_dies << " enemydies:" << enemy_dies
            //    << " distance:" << distance << std::endl;
            // too passive
            //if( ( best_ally_dies > 0 && ally_dies == 0 )                                            ||
            //    ( best_ally_dies > 0 && enemy_dies - ally_dies > best_enemy_dies - best_ally_dies ) ||
            //    ( ally_dies == best_ally_dies && enemy_dies >  best_enemy_dies )                    ||
            //    ( ally_dies == best_ally_dies && enemy_dies == best_enemy_dies && distance < best_dist ) )  

            /*
               if( enemy_dies - ally_dies > best_enemy_dies - best_ally_dies         ||
               ( ally_dies == best_ally_dies && enemy_dies == best_enemy_dies && ally_dist <  best_ally_dist ) ||
               ( ally_dies == best_ally_dies && enemy_dies == best_enemy_dies && ally_dist == best_ally_dist && 
               distance < best_dist ) )  
               */

            if( ally_dies < best_ally_dies  ||
                    ( ally_dies == best_ally_dies && enemy_dies > best_enemy_dies ) ||
                    ( ally_dies == best_ally_dies && enemy_dies == best_enemy_dies && ally_dist <  best_ally_dist ) ||
                    ( ally_dies == best_ally_dies && enemy_dies == best_enemy_dies && ally_dist ==  best_ally_dist  && 
                      distance < best_dist ) )
            {
                best_enemy_dies = enemy_dies;
                best_ally_dies  = ally_dies;
                best_dist       = distance;
                best_ally_dist  = distance;
                best_move0      = static_cast<Direction>( i );
                best_move1      = static_cast<Direction>( j );
                best_loc0       = moved_ally0;
                best_loc1       = moved_ally1;
            }
        }
    }

    //Debug::stream() << " moving " << ally0 << " in " << DIRECTION_CHAR[ best_move0 ] << std::endl
    //    << " moving " << ally1 << " in " << DIRECTION_CHAR[ best_move1 ] << std::endl;
    Ant* ant = m_map( ally0 ).ant;
    ant->path.assign( best_loc0, best_move0, Path::ATTACK );
    m_allies.insert( ant );

    ant = m_map( ally1 ).ant;
    ant->path.assign( best_loc1, best_move1, Path::ATTACK );
    m_allies.insert( ant );
}



Battle::Battle( Map& map )
    : m_map( map )
{
    //
    // create m_grid
    //
    const unsigned height = m_map.height();
    const unsigned width  = m_map.width();
    m_grid = new EnemyCount**[ height ];
    for( unsigned i = 0; i < height; ++i )
    {
        m_grid[i] = new EnemyCount*[ width ];
        for( unsigned j = 0; j < width; ++j )
        {
            m_grid[i][j] = new EnemyCount[ 10 ];
        }
    }
}


Battle::~Battle()
{
    const unsigned height = m_map.height();
    const unsigned width  = m_map.width();
    for( unsigned i = 0; i < height; ++i )
    {
        for( unsigned j = 0; j < width; ++j )
            delete [] m_grid[i][j];
        delete m_grid[i];
    }
    delete m_grid;
    m_grid = 0u;

}


void Battle::solve( const Ants& ants, const Locations& enemy_ants )
{
    //
    // Reset state 
    //
    const unsigned height = m_map.height();
    const unsigned width  = m_map.width();
    for( unsigned i = 0; i < height; ++i )
        for( unsigned j = 0; j < width; ++j )
            for( unsigned k = 0; k < 10; ++k )
                m_grid[i][j][k].reset();

    m_allies.clear();
    m_enemies.clear();



    //
    // Search from each of our ants to find enemies within 2 steps of battle
    //
    AntEnemies ally_enemies;
    for( Ants::const_iterator it = ants.begin(); it != ants.end(); ++it )
    {
        Ant*     ant             = *it;
        Location search_location = ant->location;

        // Do not move static defense ants
        if( ant->assignment == Ant::STATIC_DEFENSE ) continue;

        // Search from the ant's next step if ant has a path assigned to it
        if( !ant->path.empty() )
        {
            Location next_loc = m_map.getLocation( search_location, ant->path.nextStep() );
            if( m_map( next_loc ).isAvailable() )
            {
                AnyEnemyAnts    any_enemy_ants( MY_ANT_ID );
                WithinDistance2 within_10( m_map, next_loc, 10 );
                AnyEnemyCombatants bfs( m_map, next_loc, any_enemy_ants, within_10);
                bfs.traverse();
                if( !any_enemy_ants.found_enemy ) continue;
            }
        }
        AllEnemyAnts    enemy_ants( search_location, MY_ANT_ID, ally_enemies );
        WithinDistance2 within_battle_range( m_map, search_location, 17 );
        EnemyCombatants bfs( m_map, search_location, enemy_ants, within_battle_range );

        //Debug::stream() << "Battle: searching from " << location << std::endl;
        bfs.traverse();
    }

    if( ally_enemies.empty() ) return; // No potential battles found

    //Debug::stream() << "Battle:   found " << ally_enemies.size() << " ally ants in battle" << std::endl;

    //
    // Gather all enemies seen
    //
    for( AntEnemies::const_iterator it = ally_enemies.begin(); it != ally_enemies.end(); ++it )
    {
        const Locations& ant_enemies = it->second;
        for( Locations::const_iterator enemy = ant_enemies.begin(); enemy != ant_enemies.end(); ++enemy )
        {
            m_enemies.insert( *enemy );
        }
    }

    //Debug::stream() << "Battle:   found " << m_enemies.size() << " enemy ants in battle" << std::endl;





    //
    // Splat the ants into the m_grid
    //
    //setFillColor( 0, 0, 255, 0.1f );
    for( AntEnemies::const_iterator it = ally_enemies.begin(); it != ally_enemies.end(); ++it )
    {
        fillPlusOne( it->first, MY_ANT_ID, 1 );
        //circle( it->first, 1, true );
    }

    //setFillColor( 255, 0, 0, 0.1f );
    for( LocationSet::const_iterator it = m_enemies.begin(); it != m_enemies.end(); ++it )
    {
        fillPlusOne( *it, m_map( *it ).ant_id, 1 );
        //circle( *it, 1, true );
    }
    //setFillColor( 0, 0, 0, 0.0f );


    //
    // Divide into clusters
    //
    LocationSet                clustered;
    std::vector< AntEnemies >  clusters;
    std::vector< LocationSet > enemy_clusters;

    for( AntEnemies::const_iterator it0 = ally_enemies.begin(); it0 != ally_enemies.end(); ++it0 )
    {
        // Check if this ally is already clustered
        if( clustered.find( it0->first ) != clustered.end() ) continue;


        // If not, create a new cluster for it
        clusters.push_back( AntEnemies() );
        enemy_clusters.push_back( LocationSet() );

        clustered.insert( it0->first );

        AntEnemies& cluster = clusters.back();
        LocationSet& enemy_cluster = enemy_clusters.back();

        cluster.push_back( std::pair<Location, Locations>() );
        cluster.back().first = it0->first;
        cluster.back().second.assign( it0->second.begin(), it0->second.end() ); 
        std::copy( it0->second.begin(),
                it0->second.end(), 
                std::inserter( enemy_cluster, enemy_cluster.end() ) );


        //Debug::stream() << " creating new cluster for ant at " << it0->first << std::endl;

        // Check if any other allies belong in this cluster
        bool cluster_extended;
        do
        {
            cluster_extended = false;
            for( AntEnemies::const_iterator it1 = it0+1; it1 != ally_enemies.end(); ++it1 )
            {
                Location ally = it1->first;
                if( clustered.find( ally ) != clustered.end() ) continue;
                for( Locations::const_iterator enemy = it1->second.begin(); enemy != it1->second.end(); ++enemy )
                {
                    if( enemy_cluster.find( *enemy ) != enemy_cluster.end() )
                    {
                        clustered.insert ( ally );
                        cluster.push_back( std::pair<Location, Locations>() );

                        cluster.back().first = ally;
                        cluster.back().second.assign( it1->second.begin(), it1->second.end() ); 
                        std::copy( it1->second.begin(),
                                it1->second.end(),
                                std::inserter( enemy_cluster, enemy_cluster.end() ) );

                        cluster_extended = true;
                        break;
                    }
                }
            }

        } while( cluster_extended );
    }

    for( size_t i = 0; i < clusters.size(); ++i )
    {
        const AntEnemies& ally_enemies = clusters[i];

        setFillColor( i%3 == 0 ? 0 : rnd()%256,
                i%3 == 1 ? 0 : rnd()%256,
                i%3 == 2 ? 0 : rnd()%256, 0.3f );
        //Debug::stream() << " cluster: " << i << std::endl;
        for( AntEnemies::const_iterator it = ally_enemies.begin(); it != ally_enemies.end(); ++it )
        {

            //Debug::stream() << "    ally: " << it->first << std::endl;
            //circle( it->first, 1, true );
            const Locations& enemies = it->second;
            for( Locations::const_iterator enemy = enemies.begin(); enemy != enemies.end(); ++enemy )
            {
                //Debug::stream() << "        enemy: " << *enemy << std::endl;
                //circle( *enemy, 1, true );
            }
        }
    }


    for( size_t i = 0; i < clusters.size(); ++i )
    {
        const AntEnemies& cluster = clusters[i];
        //const LocationSet& enemies = enemy_clusters[i];
        //Debug::stream() << "running cluster of size " << cluster.size() << " vs " << enemies.size() << std::endl;

        //
        // Check for special small cases to solve by exhaustive search
        //

        if( cluster.size() == 1 )
        {
            const LocationSet& enemies = enemy_clusters[i];
            if( enemies.size() == 1 )
            {
                solve1v1( cluster[0].first, *enemies.begin() );
                continue;
            }
        }

        if( cluster.size() == 2 )
        {
            const LocationSet& enemies = enemy_clusters[i];
            if( enemies.size() == 1 )
            {
                solve2v1( cluster[0].first, cluster[1].first, *enemies.begin() );
                continue;
            }
        }

        //
        // Run simulated annealing on each cluster 
        //

        const int MAX_ITERATIONS = 1000;

        Directions cur_moves ( cluster.size(), NONE );
        Directions best_moves( cluster.size(), NONE );
        Score      cur_score  = score( cluster, cur_moves );
        Score      best_score = cur_score;

        //Debug::stream() << "Battle: initial score is " << cur_score << std::endl;

        LocationSet original_squares;
        LocationSet occupied_squares;
        for( AntEnemies::const_iterator it = cluster.begin(); it != cluster.end(); ++it )
        {
            original_squares.insert( it->first );
            occupied_squares.insert( it->first );
        }

        // TODO: any benefit to randomizing?
        std::vector<int> transition_order( cluster.size() );
        for( unsigned i = 0; i < cluster.size(); ++i ) transition_order[i] = i;
        std::random_shuffle( transition_order.begin(), transition_order.end() );


        for( int i = 0; i < MAX_ITERATIONS; ++i )
        {
            // Select ant to transition
            int       rant     = transition_order[ i % cluster.size() ];
            Location  orig_loc = cluster[ rant ].first;

            // Get the ants current position
            Direction cur_move = cur_moves[ rant ];
            Location  cur_loc  = m_map.getLocation( orig_loc, cur_move );

            // Select a transition
            Direction new_move = static_cast<Direction>( ( cur_move + rand()%4 + 1 ) % 5 );
            Location  new_loc  = m_map.getLocation( orig_loc, new_move );

            // Make sure the transition is viable 
            //Debug::stream() << "  Transitioning ant " << orig_loc << " cur: " <<  DIRECTION_CHAR[ cur_move ] 
            //    << " best: " << DIRECTION_CHAR[ best_moves[ rant ] ] << std::endl;
            //Debug::stream() << "     first trying move " << DIRECTION_CHAR[ new_move ] << std::endl;
            int tries = 0;
            while( ( ( !m_map( new_loc ).isAvailable() && original_squares.find( new_loc ) == original_squares.end() ) || 
                        occupied_squares.find( new_loc ) != occupied_squares.end() ) && 
                    tries < 3 )
            {  
                // TODO: this is broken if ant surrounded (no valid move).  need to allow 'no transition'
                //       occupied_squares check will fail for 'no transition' as well
                new_move = static_cast<Direction>( (new_move+1) % 5 );
                if( new_move == cur_move ) // Skip over cur_move
                    new_move = static_cast<Direction>( (new_move+1) % 5 );
                //Debug::stream() << "       switching to move " << DIRECTION_CHAR[ new_move ] << std::endl;
                new_loc = m_map.getLocation( orig_loc, new_move );
                tries++;
            }

            // Make the move
            occupied_squares.erase ( cur_loc );
            occupied_squares.insert( new_loc );
            fillPlusOne( cur_loc, MY_ANT_ID, -1 );
            fillPlusOne( new_loc, MY_ANT_ID, +1 );
            cur_moves[ rant ] = new_move;

            // Check if we have a new best state
            const Score new_score = score( cluster, cur_moves );
            //Debug::stream() << "Battle:      score: " << new_score << " best: " << best_score << std::endl; 
            if( new_score > best_score ) //|| ( new_score == best_score && cur_move == NONE ) ) // Prefer movement
            {
                //Debug::stream() << "Battle:  found new better score " << new_score << std::endl;
                best_score = new_score;
                best_moves.assign( cur_moves.begin(), cur_moves.end() );
            }

            // Stochastically choose whether we accept this move, based on score and cooling schedule
            float p = P( cur_score, new_score, static_cast<float>( MAX_ITERATIONS - i ) / MAX_ITERATIONS ); 
            float e = rndf();
            //Debug::stream() << "Battle       cur_score:" << cur_score << " new_score: " << new_score 
            //    << " p: " << p << " rnd: " << e << std::endl;
            if( p > e )
            {
                //Debug::stream() << "Battle:          accepting transition" << cur_score << std::endl;
                cur_score     = new_score;
            }
            else
            {
                // Undo the last move
                //Debug::stream() << "Battle:          rejecting transition " << cur_score << std::endl;
                occupied_squares.erase ( new_loc );
                occupied_squares.insert( cur_loc );
                fillPlusOne( new_loc, MY_ANT_ID, -1 );
                fillPlusOne( cur_loc, MY_ANT_ID, +1 );
                cur_moves[ rant ] = cur_move;
            }
        }
        //Debug::stream() << "Battle:  max score is" << best_score << std::endl;

        //
        // Assign paths for all battle ants
        //
        for( unsigned i = 0; i < cluster.size(); ++i )
        {
            Location ant_location = cluster[i].first;
            Ant* ant = m_map( ant_location ).ant;
            ant->path.assign( m_map.getLocation( ant_location, best_moves[i] ), best_moves[i], Path::ATTACK );
            m_allies.insert( ant );

            //Debug::stream() << "Battle:     moving ant " << ant_location << " in " << DIRECTION_CHAR[ best_moves[i] ]
            //    << std::endl;
        }
    }
}


// temp must be in [0, 1]
float Battle::P( Battle::Score old_score, Battle::Score new_score, float temp )const
{
    // At temp 1.0 (max temp)
    //   score increase => p = 0.5
    //   score match    => p = 0.50 
    //   score decrease => p = 0.5
    // At temp 0.0 (min temp)
    //   score increase => p = 1.00 
    //   score match    => p = 0.50 
    //   score decrease => p = 0.00
    const float t = new_score > old_score ? 1.0f : 
        new_score < old_score ? 0.0f : 
        0.5f;
    const float offset = lerp( 0.0f, 0.5f, temp );
    return lerp( 0.0f+offset, 1.0f-offset, t );

}
