

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
    const int MY_ANT_ID = 0;

    typedef std::vector<Ant*>                               Ants;
    typedef std::set<Ant*>                                  AntSet;

    typedef std::vector<Location>                           Locations;
    typedef std::set<Location>                              LocationSet;

    typedef std::vector< std::pair<Location, Locations> >   AntEnemies;

    typedef std::vector<Direction>                          Directions;

    struct Score
    {
        Score() : net_deaths( 0 ), ally_deaths( 0 ), attack_depths( 0 ) {}

        bool operator<( const Score& s )const
        {
            return net_deaths  < s.net_deaths  ? true  :
                   net_deaths  > s.net_deaths  ? false :
                   ally_deaths > s.ally_deaths ? true  :
                   ally_deaths < s.ally_deaths ? false :
                   attack_depths < s.attack_depths;
        }

        bool operator==( const Score& s )const
        {
            return net_deaths == s.net_deaths && ally_deaths == s.ally_deaths && attack_depths == s.attack_depths;
        }

        bool operator>( const Score& s )const
        {
            return net_deaths  > s.net_deaths  ? true  :
                   net_deaths  < s.net_deaths  ? false :
                   ally_deaths < s.ally_deaths ? true  :
                   ally_deaths > s.ally_deaths ? false :
                   attack_depths > s.attack_depths;
        }
        
        bool operator>=( const Score& s )const
        {
            return !( *this < s );
        }

        Score operator+( const Score& s )const
        {
            Score t;
            t.net_deaths       = net_deaths    + s.net_deaths;
            t.ally_deaths      = ally_deaths   + t.ally_deaths;
            t.attack_depths    = attack_depths + s.attack_depths;
            return t;
        }

        void operator+=( const Score& s )
        { *this = *this + s; }


        int net_deaths;     // enemy_kills - ally_deaths
        int ally_deaths;    // ally deaths
        int attack_depths;  // depth of attacking ants
    };

    std::ostream& operator<<( std::ostream& out, const Score& s )
    { out << s.net_deaths << "." << s.ally_deaths << "." << s.attack_depths; return out; }

    struct EnemyCount
    {
        EnemyCount() : a(0), b(0), c(0) {}
        EnemyCount( int a, int b, int c ) : a(a), b(b), c(c) {}

        void operator+=( const EnemyCount& s )
        { a += s.a; b += s.b; c += s.c; }
        
        int sum()const { return a + b + c; }

        int deepest()const      { return a ? 0 : b ? 1 : 2; }
        int deepestEnemyCount()const { return a ? a : b ? b : c; }

        bool beats( const EnemyCount& s )
        {
            //return sum() != s.sum() ? sum() < s.sum() :
            //       ( a != s.a )     ? a < s.a         :
            //       ( b != s.b )     ? b < s.b         :
            //                          c <= s.c ;
             
            //return ( sum() != s.sum() ) ? sum() < s.sum() :
            //                              deepest() <= s.deepest() && deepestEnemyCount() < s.deepestEnemyCount();
            
            Debug::stream() << "    " << *this << " beats? " << s << std::endl
                            << "        " << sum() << ":" << s.sum() << std::endl
                            << "        " << deepest() << ":" << s.deepest() << std::endl
                            << "        " << deepestEnemyCount() << ":" << s.deepestEnemyCount() << std::endl;

            //return sum() <= s.sum() && deepest() <= s.deepest() && deepestEnemyCount() <= s.deepestEnemyCount();
            return sum() <= s.sum() && deepest() <= s.deepest() && deepestEnemyCount() <= s.deepestEnemyCount();
        }
        bool beatsAlly( const EnemyCount& s )
        {
            //return sum() != s.sum() ? sum() < s.sum() :
            //       ( a != s.a )     ? a < s.a         :
            //       ( b != s.b )     ? b < s.b         :
            //                          c <= s.c ;
             
            //return ( sum() != s.sum() ) ? sum() < s.sum() :
            //                              deepest() <= s.deepest() && deepestEnemyCount() < s.deepestEnemyCount();
            
            //return sum() <= s.sum();
            return deepestEnemyCount() <= s.deepestEnemyCount() || sum() < s.sum();
        }

        bool operator<( const EnemyCount& s )
        {
            //return a < s.a ? true  : 
            //       a > s.a ? false :
            //       b < s.b ? true  : 
            //       b > s.b ? false :
            //       c < s.c ? true  : 
            //                 false ;

            //return ( a && s.a ) ? a < s.a :
            //       ( b && s.b ) ? b < s.b :
            //                      c < s.c ;

            //bool sum_less       = sum() < s.sum(); 
            //bool top_field_less = ( ( a || s.a ) && a != s.a ) ? a < s.a :
            //                      ( ( b || s.b ) && b != s.b ) ? b < s.b :
            //                                                     c < s.c ;

            bool sum_less       = sum() < s.sum(); 
            bool top_field_less = ( ( a && s.a ) && a != s.a ) ? a < s.a :
                                  ( ( b && s.b ) && b != s.b ) ? b < s.b :
                                                                 c < s.c ;
            return sum_less && top_field_less;
        }

        bool operator>=( const EnemyCount& s )
        {
            return !( *this < s );
        }


        int a, b, c;
    };

    std::ostream& operator<<( std::ostream& out, const EnemyCount& s )
    { out << s.a << "." << s.b << "." << s.c; return out; }



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
    
    void fillWithColor( EnemyCount*** grid, unsigned width, unsigned height, const Location& location, int ant_id, int inc)
    {
        Location x = clamp( Location( location.row-2, location.col-1 ), height, width );
        grid[ x.row ][ x.col ][ ant_id ].b += inc;
        //tile( x );
        x = clamp( Location( location.row-2, location.col-0 ), height, width );
        grid[ x.row ][ x.col ][ ant_id ].b += inc;
        //tile( x );
        x = clamp( Location( location.row-2, location.col+1 ), height, width );
        grid[ x.row ][ x.col ][ ant_id ].b += inc;
        //tile( x );

        x = clamp( Location( location.row-1, location.col-2 ), height, width );
        grid[ x.row ][ x.col ][ ant_id ].b += inc;
        //tile( x );
        x = clamp( Location( location.row-1, location.col-1 ), height, width );
        grid[ x.row ][ x.col ][ ant_id ].a += inc;
        //tile( x );
        x = clamp( Location( location.row-1, location.col-0 ), height, width );
        grid[ x.row ][ x.col ][ ant_id ].a += inc;
        //tile( x );
        x = clamp( Location( location.row-1, location.col+1 ), height, width );
        grid[ x.row ][ x.col ][ ant_id ].a += inc;
        //tile( x );
        x = clamp( Location( location.row-1, location.col+2 ), height, width );
        grid[ x.row ][ x.col ][ ant_id ].b += inc;
        //tile( x );

        x = clamp( Location( location.row-0, location.col-2 ), height, width );
        grid[ x.row ][ x.col ][ ant_id ].b += inc;
        //tile( x );
        x = clamp( Location( location.row-0, location.col-1 ), height, width );
        grid[ x.row ][ x.col ][ ant_id ].a += inc;
        //tile( x );
        x = clamp( Location( location.row-0, location.col-0 ), height, width );
        grid[ x.row ][ x.col ][ ant_id ].a += inc;
        //tile( x );
        x = clamp( Location( location.row-0, location.col+1 ), height, width );
        grid[ x.row ][ x.col ][ ant_id ].a += inc;
        //tile( x );
        x = clamp( Location( location.row-0, location.col+2 ), height, width );
        grid[ x.row ][ x.col ][ ant_id ].b += inc;
        //tile( x );

        x = clamp( Location( location.row+1, location.col-2 ), height, width );
        grid[ x.row ][ x.col ][ ant_id ].b += inc;
        //tile( x );
        x = clamp( Location( location.row+1, location.col-1 ), height, width );
        grid[ x.row ][ x.col ][ ant_id ].a += inc;
        //tile( x );
        x = clamp( Location( location.row+1, location.col-0 ), height, width );
        grid[ x.row ][ x.col ][ ant_id ].a += inc;
        //tile( x );
        x = clamp( Location( location.row+1, location.col+1 ), height, width );
        grid[ x.row ][ x.col ][ ant_id ].a += inc;
        //tile( x );
        x = clamp( Location( location.row+1, location.col+2 ), height, width );
        grid[ x.row ][ x.col ][ ant_id ].b += inc;
        //tile( x );

        x = clamp( Location( location.row+2, location.col-1 ), height, width );
        grid[ x.row ][ x.col ][ ant_id ].b += inc;
        //tile( x );
        x = clamp( Location( location.row+2, location.col-0 ), height, width );
        grid[ x.row ][ x.col ][ ant_id ].b += inc;
        //tile( x );
        x = clamp( Location( location.row+2, location.col+1 ), height, width );
        grid[ x.row ][ x.col ][ ant_id ].b += inc;
        //tile( x );
    }


    void fill( EnemyCount*** grid, unsigned width, unsigned height, const Location& location, int ant_id, int inc)
    {
        Location x = clamp( Location( location.row-2, location.col-1 ), height, width );
        grid[ x.row ][ x.col ][ ant_id ].b += inc;
        x = clamp( Location( location.row-2, location.col-0 ), height, width );
        grid[ x.row ][ x.col ][ ant_id ].b += inc;
        x = clamp( Location( location.row-2, location.col+1 ), height, width );
        grid[ x.row ][ x.col ][ ant_id ].b += inc;

        x = clamp( Location( location.row-1, location.col-2 ), height, width );
        grid[ x.row ][ x.col ][ ant_id ].b += inc;
        x = clamp( Location( location.row-1, location.col-1 ), height, width );
        grid[ x.row ][ x.col ][ ant_id ].a += inc;
        x = clamp( Location( location.row-1, location.col-0 ), height, width );
        grid[ x.row ][ x.col ][ ant_id ].a += inc;
        x = clamp( Location( location.row-1, location.col+1 ), height, width );
        grid[ x.row ][ x.col ][ ant_id ].a += inc;
        x = clamp( Location( location.row-1, location.col+2 ), height, width );
        grid[ x.row ][ x.col ][ ant_id ].b += inc;

        x = clamp( Location( location.row-0, location.col-2 ), height, width );
        grid[ x.row ][ x.col ][ ant_id ].b += inc;
        x = clamp( Location( location.row-0, location.col-1 ), height, width );
        grid[ x.row ][ x.col ][ ant_id ].a += inc;
        x = clamp( Location( location.row-0, location.col-0 ), height, width );
        grid[ x.row ][ x.col ][ ant_id ].a += inc;
        x = clamp( Location( location.row-0, location.col+1 ), height, width );
        grid[ x.row ][ x.col ][ ant_id ].a += inc;
        x = clamp( Location( location.row-0, location.col+2 ), height, width );
        grid[ x.row ][ x.col ][ ant_id ].b += inc;

        x = clamp( Location( location.row+1, location.col-2 ), height, width );
        grid[ x.row ][ x.col ][ ant_id ].b += inc;
        x = clamp( Location( location.row+1, location.col-1 ), height, width );
        grid[ x.row ][ x.col ][ ant_id ].a += inc;
        x = clamp( Location( location.row+1, location.col-0 ), height, width );
        grid[ x.row ][ x.col ][ ant_id ].a += inc;
        x = clamp( Location( location.row+1, location.col+1 ), height, width );
        grid[ x.row ][ x.col ][ ant_id ].a += inc;
        x = clamp( Location( location.row+1, location.col+2 ), height, width );
        grid[ x.row ][ x.col ][ ant_id ].b += inc;

        x = clamp( Location( location.row+2, location.col-1 ), height, width );
        grid[ x.row ][ x.col ][ ant_id ].b += inc;
        x = clamp( Location( location.row+2, location.col-0 ), height, width );
        grid[ x.row ][ x.col ][ ant_id ].b += inc;
        x = clamp( Location( location.row+2, location.col+1 ), height, width );
        grid[ x.row ][ x.col ][ ant_id ].b += inc;
    }

    void fillPlusOneWithColor( EnemyCount*** grid, const Map& map, int width, int height, const Location& location, int ant_id, int inc )
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
            grid[ x.row ][ x.col ][ ant_id ].c += inc;
            //tile( x );
            x = clamp( Location( location.row-3, location.col-0 ), height, width );
            grid[ x.row ][ x.col ][ ant_id ].c += inc;
            //tile( x );
            x = clamp( Location( location.row-3, location.col+1 ), height, width );
            grid[ x.row ][ x.col ][ ant_id ].c += inc;
            //tile( x );
        }

        if( !isWaterOrFood( map( x_s ) ) )
        {
            Location x = clamp( Location( location.row+3, location.col-1 ), height, width );
            grid[ x.row ][ x.col ][ ant_id ].c += inc;
            //tile( x );
            x = clamp( Location( location.row+3, location.col-0 ), height, width );
            grid[ x.row ][ x.col ][ ant_id ].c += inc;
            //tile( x );
            x = clamp( Location( location.row+3, location.col+1 ), height, width );
            grid[ x.row ][ x.col ][ ant_id ].c += inc;
            //tile( x );
            
        }
        
        if( !isWaterOrFood( map( x_w ) ) )
        {
            Location x = clamp( Location( location.row-1, location.col-3 ), height, width );
            grid[ x.row ][ x.col ][ ant_id ].c += inc;
            //tile( x );
            x = clamp( Location( location.row+0, location.col-3 ), height, width );
            grid[ x.row ][ x.col ][ ant_id ].c += inc;
            //tile( x );
            x = clamp( Location( location.row+1, location.col-3 ), height, width );
            grid[ x.row ][ x.col ][ ant_id ].c += inc;
            //tile( x );
        }
            
        if( !isWaterOrFood( map( x_e ) ) )
        {
            Location x = clamp( Location( location.row-1, location.col+3 ), height, width );
            grid[ x.row ][ x.col ][ ant_id ].c += inc;
            //tile( x );
            x = clamp( Location( location.row+0, location.col+3 ), height, width );
            grid[ x.row ][ x.col ][ ant_id ].c += inc;
            //tile( x );
            x = clamp( Location( location.row+1, location.col+3 ), height, width );
            grid[ x.row ][ x.col ][ ant_id ].c += inc;
            //tile( x );
        }

        if( !isWaterOrFood( map( x_e ) ) || !isWaterOrFood( map( x_n ) ) )
        {
            Location x = clamp( Location( location.row+2, location.col+2 ), height, width );
            grid[ x.row ][ x.col ][ ant_id ].c += inc;
            //tile( x );
        }
        
        if( !isWaterOrFood( map( x_w ) ) || !isWaterOrFood( map( x_n ) ) )
        {
            Location x = clamp( Location( location.row-2, location.col+2 ), height, width );
            grid[ x.row ][ x.col ][ ant_id ].c += inc;
            //tile( x );
        }

        if( !isWaterOrFood( map( x_w ) ) || !isWaterOrFood( map( x_s ) ) )
        {
            Location x = clamp( Location( location.row-2, location.col-2 ), height, width );
            grid[ x.row ][ x.col ][ ant_id ].c += inc;
            //tile( x );
        }

        if( !isWaterOrFood( map( x_e ) ) || !isWaterOrFood( map( x_s ) ) )
        {
            Location x = clamp( Location( location.row+2, location.col-2 ), height, width );
            grid[ x.row ][ x.col ][ ant_id ].c += inc;
            //tile( x );
        }
    }
    void fillPlusOne( EnemyCount*** grid, const Map& map, int width, int height, const Location& location, int ant_id, int inc )
    {
        fill( grid, width, height, location, ant_id, inc);

        // The four move locations
        Location x_n = clamp( Location( location.row-1, location.col+0 ), height, width );
        Location x_s = clamp( Location( location.row+1, location.col+0 ), height, width );
        Location x_e = clamp( Location( location.row+0, location.col+1 ), height, width );
        Location x_w = clamp( Location( location.row+0, location.col-1 ), height, width );

        if( !isWaterOrFood( map( x_n ) ) )
        {
            Location x = clamp( Location( location.row-3, location.col-1 ), height, width );
            grid[ x.row ][ x.col ][ ant_id ].c += inc;
            x = clamp( Location( location.row-3, location.col-0 ), height, width );
            grid[ x.row ][ x.col ][ ant_id ].c += inc;
            x = clamp( Location( location.row-3, location.col+1 ), height, width );
            grid[ x.row ][ x.col ][ ant_id ].c += inc;
        }

        if( !isWaterOrFood( map( x_s ) ) )
        {
            Location x = clamp( Location( location.row+3, location.col-1 ), height, width );
            grid[ x.row ][ x.col ][ ant_id ].c += inc;
            x = clamp( Location( location.row+3, location.col-0 ), height, width );
            grid[ x.row ][ x.col ][ ant_id ].c += inc;
            x = clamp( Location( location.row+3, location.col+1 ), height, width );
            grid[ x.row ][ x.col ][ ant_id ].c += inc;
        }
        
        if( !isWaterOrFood( map( x_w ) ) )
        {
            Location x = clamp( Location( location.row-1, location.col-3 ), height, width );
            grid[ x.row ][ x.col ][ ant_id ].c += inc;
            x = clamp( Location( location.row+0, location.col-3 ), height, width );
            grid[ x.row ][ x.col ][ ant_id ].c += inc;
            x = clamp( Location( location.row+1, location.col-3 ), height, width );
            grid[ x.row ][ x.col ][ ant_id ].c += inc;
        }
            
        if( !isWaterOrFood( map( x_e ) ) )
        {
            Location x = clamp( Location( location.row-1, location.col+3 ), height, width );
            grid[ x.row ][ x.col ][ ant_id ].c += inc;
            x = clamp( Location( location.row+0, location.col+3 ), height, width );
            grid[ x.row ][ x.col ][ ant_id ].c += inc;
            x = clamp( Location( location.row+1, location.col+3 ), height, width );
            grid[ x.row ][ x.col ][ ant_id ].c += inc;
        }

        if( !isWaterOrFood( map( x_e ) ) || !isWaterOrFood( map( x_n ) ) )
        {
            Location x = clamp( Location( location.row+2, location.col+2 ), height, width );
            grid[ x.row ][ x.col ][ ant_id ].c += inc;
        }
        
        if( !isWaterOrFood( map( x_w ) ) || !isWaterOrFood( map( x_n ) ) )
        {
            Location x = clamp( Location( location.row-2, location.col+2 ), height, width );
            grid[ x.row ][ x.col ][ ant_id ].c += inc;
        }

        if( !isWaterOrFood( map( x_w ) ) || !isWaterOrFood( map( x_s ) ) )
        {
            Location x = clamp( Location( location.row-2, location.col-2 ), height, width );
            grid[ x.row ][ x.col ][ ant_id ].c += inc;
        }

        if( !isWaterOrFood( map( x_e ) ) || !isWaterOrFood( map( x_s ) ) )
        {
            Location x = clamp( Location( location.row+2, location.col-2 ), height, width );
            grid[ x.row ][ x.col ][ ant_id ].c += inc;
        }
    }


    Score score( const Map& map, const AntEnemies& ant_enemies, const Directions& moves, EnemyCount*** grid )
    {
        LocationSet dead_enemies; 
        Score       current_score;

        int idx = 0;
        for( AntEnemies::const_iterator it = ant_enemies.begin(); it != ant_enemies.end(); ++it )
        {
            // add up my_enem, distancey_count for moved position
            Location loc   = map.getLocation( it->first, moves[ idx++ ] );

            EnemyCount my_enemies = grid[ loc.row ][ loc.col ][ 1 ];
            my_enemies += grid[ loc.row ][ loc.col ][ 2 ];
            my_enemies += grid[ loc.row ][ loc.col ][ 3 ];
            my_enemies += grid[ loc.row ][ loc.col ][ 4 ];
            my_enemies += grid[ loc.row ][ loc.col ][ 5 ];
            my_enemies += grid[ loc.row ][ loc.col ][ 6 ];
            my_enemies += grid[ loc.row ][ loc.col ][ 7 ];
            my_enemies += grid[ loc.row ][ loc.col ][ 8 ];
            my_enemies += grid[ loc.row ][ loc.col ][ 9 ];
            
            Debug::stream() << "scoring: " << loc << std::endl
                            << "     my_enemies: " << my_enemies << std::endl;

            
            // for each of my nearby_enemies within dist2 of 10 (they can step into our zone)
            bool my_ant_dies = false;
            int  depths = 0;
            const Locations& enemies = it->second;
            for( Locations::const_iterator enemy = enemies.begin(); enemy != enemies.end(); ++enemy )
            {
                Location enemy_loc = *enemy;
                if( map.distance2( loc, enemy_loc ) <= 10 ) // TODO: magic number
                {
                    // Potentially in range.  Count this enemy's enemy count
                    // (for now approximate as just ant_id zero enemies)
                    EnemyCount enemy_enemies = grid[ enemy_loc.row ][ enemy_loc.col ][ MY_ANT_ID ];
                    if( my_enemies.beats( enemy_enemies ) )
                    { dead_enemies.insert( enemy_loc ); depths += ( 2 - my_enemies.deepest() ); }
                    if( enemy_enemies.beatsAlly( my_enemies ) )
                    { my_ant_dies = true; }
                    Debug::stream() << "    enemy " << enemy_loc <<  ": " << enemy_enemies << std::endl
                                    << "             my_ant_dies = " << my_ant_dies << std::endl
                                    << "             enemy_dies  = " << my_enemies.beats( enemy_enemies ) << std::endl;
                    
                }
            }
            current_score.net_deaths    -= static_cast<int>( my_ant_dies );
            current_score.ally_deaths   += static_cast<int>( my_ant_dies );
            current_score.attack_depths += depths;
            
            Debug::stream() << "        current_score " << current_score << std::endl;

        }

        current_score.net_deaths += dead_enemies.size();
        Debug::stream() << "    dead enemies " << dead_enemies.size() << std::endl;
        Debug::stream() << "    final score  " << current_score << std::endl;
        return current_score;
    }
    

    void solve1v1( const Location& ally, const Location& enemy, const Map& map, std::set<Ant*>& assigned )
    {
        Debug::stream() << "Battle:  solve1v1 ... " << ally << " vs " << enemy << std::endl; 

        // TODO: special case > 10 (one ring) to keep ally in one ring???
        //
        // TODO: insert new location into occupied list

        // Move away from the ant

        int max_dist = map.distance2( enemy, ally );
        Location  away_loc = ally;


        Locations neighbors;
        map.getNeighbors( ally, isAvailable, neighbors );
        
        if( max_dist > 10 )
        {
            // Stay within the one-ring
            for( Locations::iterator it = neighbors.begin(); it != neighbors.end(); ++it )
            {
                int dist = map.distance2( enemy, *it );
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
                int dist = map.distance2( enemy, *it );
                if( dist > max_dist ) 
                {
                    max_dist = dist;
                    away_loc = *it;
                }
            }
        }

        Direction away = map.getDirection( ally, away_loc );

        Ant* ant = map( ally ).ant;
        ant->path.assign( away_loc, away, Path::ATTACK );
        assigned.insert( ant );

        Debug::stream() << "Battle:      solve1v1 moving ant " << ally << " in " << DIRECTION_CHAR[ away ] << std::endl;
    }
    

    // TODO: switch this to minimax scoring (return maximum ally deaths and associated enemy deaths then minimize this)
    inline void countDeaths( const Location& ally0, const Location& ally1, const Location& enemy, const Map& map,
                             int& ally_deaths, int& enemy_deaths, int& distance )
    {
        int dist0           = map.distance2( ally0, enemy );
        int dist1           = map.distance2( ally1, enemy );
        int allies_in_range = static_cast<int>( dist0 <= 5 ) +
                              static_cast<int>( dist1 <= 5 );
        enemy_deaths = static_cast<int>( allies_in_range >= 1 );
        ally_deaths  = static_cast<int>( allies_in_range == 1 );
        distance     = dist0 + dist1;

        for( int i = 0; i < 4; ++i )
        {
            Location moved_enemy = map.getLocation( enemy, static_cast<Direction>( i ) ); 
            int dist0           = map.distance2( ally0, moved_enemy );
            int dist1           = map.distance2( ally1, moved_enemy );
            int allies_in_range = static_cast<int>( dist0 <= 5 ) +
                                  static_cast<int>( dist1 <= 5 );
            enemy_deaths += static_cast<int>( allies_in_range >= 1 );
            ally_deaths  += static_cast<int>( allies_in_range == 1 );
            distance     += dist0 + dist1;
        }
    }


    void solve2v1( const Location& ally0, const Location& ally1, const Location& enemy,
                   const Map& map, std::set<Ant*>& assigned )
    {
        Debug::stream() << "solve2v1 " << ally0 << "," << ally1 << " vs " << enemy << std::endl;

        Direction best_move0 = NONE;
        Direction best_move1 = NONE;
        Location  best_loc0 = ally0;
        Location  best_loc1 = ally1;
        int best_ally_dies = 10, best_enemy_dies = 0, best_distance = 1000;

        for( int i = 0; i < 5; ++i )
        {
            Location moved_ally0 = map.getLocation( ally0, static_cast<Direction>( i ) ); 
            if( !map( moved_ally0 ).isAvailable() && moved_ally0 != ally1 )
                continue;
            for( int j = 0; j < 5; ++j )
            {
                Location moved_ally1 = map.getLocation( ally1, static_cast<Direction>( j ) ); 
                if( moved_ally1 == moved_ally0 || ( !map( moved_ally1 ).isAvailable() && moved_ally1 != ally0 ) )
                        continue;
                int ally_dies, enemy_dies, distance;
                countDeaths( moved_ally0, moved_ally1, enemy, map, ally_dies, enemy_dies, distance );

                Debug::stream() << "    trying " << moved_ally0 << "," << moved_ally1 
                                << " new allydies:" << ally_dies << " enemydies:" << enemy_dies
                                << " distance:" << distance << std::endl;
                // too passive
                //if( ( best_ally_dies > 0 && ally_dies == 0 )                                            ||
                //    ( best_ally_dies > 0 && enemy_dies - ally_dies > best_enemy_dies - best_ally_dies ) ||
                //    ( ally_dies == best_ally_dies && enemy_dies >  best_enemy_dies )                    ||
                //    ( ally_dies == best_ally_dies && enemy_dies == best_enemy_dies && distance < best_distance ) )  
                if( enemy_dies - ally_dies > best_enemy_dies - best_ally_dies         ||
                  ( ally_dies == best_ally_dies && enemy_dies == best_enemy_dies && distance < best_distance ) )  
                {
                    best_enemy_dies = enemy_dies;
                    best_ally_dies  = ally_dies;
                    best_distance   = distance;
                    best_move0 = static_cast<Direction>( i );
                    best_move1 = static_cast<Direction>( j );
                    best_loc0  = moved_ally0;
                    best_loc1  = moved_ally1;
                }
            }
        }
        
        Debug::stream() << " moving " << ally0 << " in " << DIRECTION_CHAR[ best_move0 ] << std::endl
                        << " moving " << ally1 << " in " << DIRECTION_CHAR[ best_move1 ] << std::endl;
        Ant* ant = map( ally0 ).ant;
        ant->path.assign( best_loc0, best_move0, Path::ATTACK );
        assigned.insert( ant );

        ant = map( ally1 ).ant;
        ant->path.assign( best_loc1, best_move1, Path::ATTACK );
        assigned.insert( ant );
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
    float P( Score old_score, Score new_score, float temp )
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
    EnemyCount*** grid; 
    grid = new EnemyCount**[ height ];
    for( unsigned i = 0; i < height; ++i )
    {
        grid[i] = new EnemyCount*[ width ];
        for( unsigned j = 0; j < width; ++j )
        {
            grid[i][j] = new EnemyCount[ 10 ];
        }
    }
    

    //
    // Splat the ants into the grid
    //
    //setFillColor( 0, 0, 255, 0.1f );
    for( AntEnemies::const_iterator it = ally_enemies.begin(); it != ally_enemies.end(); ++it )
    {
        fillPlusOneWithColor( grid, map, width, height, it->first, MY_ANT_ID, 1 );
        //circle( it->first, 1, true );
    }
    
    //setFillColor( 255, 0, 0, 0.1f );
    for( LocationSet::const_iterator it = enemies.begin(); it != enemies.end(); ++it )
    {
        fillPlusOneWithColor( grid, map, width, height, *it, map( *it ).ant_id, 1 );
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


        Debug::stream() << " creating new cluster for ant at " << it0->first << std::endl;

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
        Debug::stream() << " cluster: " << i << std::endl;
        for( AntEnemies::const_iterator it = ally_enemies.begin(); it != ally_enemies.end(); ++it )
        {

            Debug::stream() << "    ally: " << it->first << std::endl;
            circle( it->first, 1, true );
            const Locations& enemies = it->second;
            for( Locations::const_iterator enemy = enemies.begin(); enemy != enemies.end(); ++enemy )
            {
                Debug::stream() << "        enemy: " << *enemy << std::endl;
                circle( *enemy, 1, true );
            }
        }
    }


    for( size_t i = 0; i < clusters.size(); ++i )
    {
        const AntEnemies& cluster = clusters[i];
        const LocationSet& enemies = enemy_clusters[i];
        Debug::stream() << "running cluster of size " << cluster.size() << " vs " << enemies.size() << std::endl;

        //
        // Check for special small cases to solve by exhaustive search
        //

        if( cluster.size() == 1 )
        {
            const LocationSet& enemies = enemy_clusters[i];
            if( enemies.size() == 1 )
            {
                solve1v1( cluster[0].first, *enemies.begin(), map, assigned );
                continue;
            }
        }
        
        if( cluster.size() == 2 )
        {
            const LocationSet& enemies = enemy_clusters[i];
            if( enemies.size() == 1 )
            {
                solve2v1( cluster[0].first, cluster[1].first, *enemies.begin(), map, assigned );
                continue;
            }
        }

        //
        // Run simulated annealing on each cluster 
        //

        const int MAX_ITERATIONS = 1000;

        Directions cur_moves ( cluster.size(), NONE );
        Directions best_moves( cluster.size(), NONE );
        Score      cur_score  = score( map, cluster, cur_moves, grid );
        Score      best_score = cur_score;

        Debug::stream() << "Battle: initial score is " << cur_score << std::endl;

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
            const Score new_score = score( map, cluster, cur_moves, grid );
            Debug::stream() << "Battle:      score: " << new_score << " best: " << best_score << std::endl; 
            if( new_score > best_score ) //|| ( new_score == best_score && cur_move == NONE ) ) // Prefer movement
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
                Debug::stream() << "Battle:          rejecting transition " << cur_score << std::endl;
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
        for( unsigned i = 0; i < cluster.size(); ++i )
        {
            Location ant_location = cluster[i].first;
            Ant* ant = map( ant_location ).ant;
            ant->path.assign( map.getLocation( ant_location, best_moves[i] ), best_moves[i], Path::ATTACK );
            assigned.insert( ant );

            Debug::stream() << "Battle:     moving ant " << ant_location << " in " << DIRECTION_CHAR[ best_moves[i] ]
                            << std::endl;
        }
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
