

#include <cstring>
#include <iterator>
#include <map>

#include "Ant.h"
#include "BF.h"
#include "Battle.h"
#include "Debug.h"
#include "Map.h"
    
struct CombatTile
    {
        enum Result
        {
            LOSE=0,
            TIE,
            SAFE,
            WIN
        };

        CombatTile() { reset(); }

        void reset() 
        { 
            distance_sum = 0;
            memset( attacks,          0, sizeof( attacks ) );
            memset( lowest_enemies,   1, sizeof( lowest_enemies ) );
            lowest_enemies[0] = lowest_enemies[1] = lowest_enemies[2] = lowest_enemies[3] =
                                lowest_enemies[4] = lowest_enemies[5] = lowest_enemies[6] =
                                lowest_enemies[7] = lowest_enemies[8] = lowest_enemies[9] = -1;
        }

        int enemies( int player )const
        { 
            assert( 0 <= player && player < 10 );
            // TODO: precalculate if necessary
            return attacks[0] + attacks[1] + attacks[2] + attacks[3] + attacks[4] +
                   attacks[5] + attacks[6] + attacks[7] + attacks[8] + attacks[9] -
                   attacks[player];
        }

        int mmin( int a, int b ) { return a < b ? a : b; }
        
        void setLowestEnemies( int player, int enemies )
        {
            assert( 0 <= player && player < 10 );
            for( int i = 0; i < 10; ++i )
                if( i != player && ( enemies < lowest_enemies[i] || lowest_enemies[i] == -1 ) )
                {
                    Debug::stream() << "           setting lowest_enemies[ " << i << " ] to " << enemies << std::endl;
                    lowest_enemies[i] = enemies;
                }
                else
                {
                    Debug::stream() << "           keeping lowest_enemies[ " << i << " ] of " << lowest_enemies[i]
                                    << std::endl;
                }
        }       

        Result result( int player )
        {
            assert( 0 <= player && player < 10 );
            const int enemy_enemies  = lowest_enemies[ player ];
            Debug::stream() << " result(): lowest_enemy_enemies = " << enemy_enemies << std::endl;

            if( enemy_enemies == -1 )
            {
                Debug::stream() << "  result(): returning SAFE " << std::endl;
                return SAFE;
            }

            const int player_enemies = enemies( player );
            Debug::stream() << "  comparing result() p_enemies: " << player_enemies 
                            << " e_enemies: " << enemy_enemies << std::endl;
            return 
                   player_enemies < enemy_enemies ? WIN :
                   player_enemies > enemy_enemies ? LOSE :
                   TIE;
                  
        }

        int    distance_sum;           // Sum of all distnces to enemies 
        int    attacks[ 10 ];          // Number of ants which could attack this
        int    lowest_enemies[ 10 ];   // Lowest enemy's enemis in range of this
    };

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
    assert( 0 <= ant_id && ant_id < 10 );
    const unsigned height = m_map.height();
    const unsigned width  = m_map.width();

    Location x = clamp( Location( location.row-2, location.col-1 ), height, width );
    m_grid[ x.row ][ x.col ].attacks[ ant_id ] += inc;
    x = clamp( Location( location.row-2, location.col-0 ), height, width );
    m_grid[ x.row ][ x.col ].attacks[ ant_id ] += inc;
    x = clamp( Location( location.row-2, location.col+1 ), height, width );
    m_grid[ x.row ][ x.col ].attacks[ ant_id ] += inc;

    x = clamp( Location( location.row-1, location.col-2 ), height, width );
    m_grid[ x.row ][ x.col ].attacks[ ant_id ] += inc;
    x = clamp( Location( location.row-1, location.col-1 ), height, width );
    m_grid[ x.row ][ x.col ].attacks[ ant_id ] += inc;
    x = clamp( Location( location.row-1, location.col-0 ), height, width );
    m_grid[ x.row ][ x.col ].attacks[ ant_id ] += inc;
    x = clamp( Location( location.row-1, location.col+1 ), height, width );
    m_grid[ x.row ][ x.col ].attacks[ ant_id ] += inc;
    x = clamp( Location( location.row-1, location.col+2 ), height, width );
    m_grid[ x.row ][ x.col ].attacks[ ant_id ] += inc;

    x = clamp( Location( location.row-0, location.col-2 ), height, width );
    m_grid[ x.row ][ x.col ].attacks[ ant_id ] += inc;
    x = clamp( Location( location.row-0, location.col-1 ), height, width );
    m_grid[ x.row ][ x.col ].attacks[ ant_id ] += inc;
    x = clamp( Location( location.row-0, location.col-0 ), height, width );
    m_grid[ x.row ][ x.col ].attacks[ ant_id ] += inc;
    x = clamp( Location( location.row-0, location.col+1 ), height, width );
    m_grid[ x.row ][ x.col ].attacks[ ant_id ] += inc;
    x = clamp( Location( location.row-0, location.col+2 ), height, width );
    m_grid[ x.row ][ x.col ].attacks[ ant_id ] += inc;

    x = clamp( Location( location.row+1, location.col-2 ), height, width );
    m_grid[ x.row ][ x.col ].attacks[ ant_id ] += inc;
    x = clamp( Location( location.row+1, location.col-1 ), height, width );
    m_grid[ x.row ][ x.col ].attacks[ ant_id ] += inc;
    x = clamp( Location( location.row+1, location.col-0 ), height, width );
    m_grid[ x.row ][ x.col ].attacks[ ant_id ] += inc;
    x = clamp( Location( location.row+1, location.col+1 ), height, width );
    m_grid[ x.row ][ x.col ].attacks[ ant_id ] += inc;
    x = clamp( Location( location.row+1, location.col+2 ), height, width );
    m_grid[ x.row ][ x.col ].attacks[ ant_id ] += inc;

    x = clamp( Location( location.row+2, location.col-1 ), height, width );
    m_grid[ x.row ][ x.col ].attacks[ ant_id ] += inc;
    x = clamp( Location( location.row+2, location.col-0 ), height, width );
    m_grid[ x.row ][ x.col ].attacks[ ant_id ] += inc;
    x = clamp( Location( location.row+2, location.col+1 ), height, width );
    m_grid[ x.row ][ x.col ].attacks[ ant_id ] += inc;
}


void Battle::fillLowestEnemies( const Location& location, int ant_id )
{
    const unsigned height  = m_map.height();
    const unsigned width   = m_map.width();
    const int      enemies = m_grid[ location.row ][ location.col ].enemies( ant_id );

    Debug::stream() << "      propagating enemy count of " << enemies << std::endl;

    Location x = clamp( Location( location.row-2, location.col-1 ), height, width );
    Debug::stream() << "        calling setLowestEnemies on " << x << std::endl;
    m_grid[ x.row ][ x.col ].setLowestEnemies( ant_id, enemies );
    x = clamp( Location( location.row-2, location.col-0 ), height, width );
    Debug::stream() << "        calling setLowestEnemies on " << x << std::endl;
    m_grid[ x.row ][ x.col ].setLowestEnemies( ant_id, enemies );
    x = clamp( Location( location.row-2, location.col+1 ), height, width );
    Debug::stream() << "        calling setLowestEnemies on " << x << std::endl;
    m_grid[ x.row ][ x.col ].setLowestEnemies( ant_id, enemies );

    x = clamp( Location( location.row-1, location.col-2 ), height, width );
    Debug::stream() << "        calling setLowestEnemies on " << x << std::endl;
    m_grid[ x.row ][ x.col ].setLowestEnemies( ant_id, enemies );
    x = clamp( Location( location.row-1, location.col-1 ), height, width );
    Debug::stream() << "        calling setLowestEnemies on " << x << std::endl;
    m_grid[ x.row ][ x.col ].setLowestEnemies( ant_id, enemies );
    x = clamp( Location( location.row-1, location.col-0 ), height, width );
    Debug::stream() << "        calling setLowestEnemies on " << x << std::endl;
    m_grid[ x.row ][ x.col ].setLowestEnemies( ant_id, enemies );
    x = clamp( Location( location.row-1, location.col+1 ), height, width );
    Debug::stream() << "        calling setLowestEnemies on " << x << std::endl;
    m_grid[ x.row ][ x.col ].setLowestEnemies( ant_id, enemies );
    x = clamp( Location( location.row-1, location.col+2 ), height, width );
    Debug::stream() << "        calling setLowestEnemies on " << x << std::endl;
    m_grid[ x.row ][ x.col ].setLowestEnemies( ant_id, enemies );

    x = clamp( Location( location.row-0, location.col-2 ), height, width );
    Debug::stream() << "        calling setLowestEnemies on " << x << std::endl;
    m_grid[ x.row ][ x.col ].setLowestEnemies( ant_id, enemies );
    x = clamp( Location( location.row-0, location.col-1 ), height, width );
    Debug::stream() << "        calling setLowestEnemies on " << x << std::endl;
    m_grid[ x.row ][ x.col ].setLowestEnemies( ant_id, enemies );
    x = clamp( Location( location.row-0, location.col-0 ), height, width );
    Debug::stream() << "        calling setLowestEnemies on " << x << std::endl;
    m_grid[ x.row ][ x.col ].setLowestEnemies( ant_id, enemies );
    x = clamp( Location( location.row-0, location.col+1 ), height, width );
    Debug::stream() << "        calling setLowestEnemies on " << x << std::endl;
    m_grid[ x.row ][ x.col ].setLowestEnemies( ant_id, enemies );
    x = clamp( Location( location.row-0, location.col+2 ), height, width );
    Debug::stream() << "        calling setLowestEnemies on " << x << std::endl;
    m_grid[ x.row ][ x.col ].setLowestEnemies( ant_id, enemies );

    x = clamp( Location( location.row+1, location.col-2 ), height, width );
    Debug::stream() << "        calling setLowestEnemies on " << x << std::endl;
    m_grid[ x.row ][ x.col ].setLowestEnemies( ant_id, enemies );
    x = clamp( Location( location.row+1, location.col-1 ), height, width );
    Debug::stream() << "        calling setLowestEnemies on " << x << std::endl;
    m_grid[ x.row ][ x.col ].setLowestEnemies( ant_id, enemies );
    x = clamp( Location( location.row+1, location.col-0 ), height, width );
    Debug::stream() << "        calling setLowestEnemies on " << x << std::endl;
    m_grid[ x.row ][ x.col ].setLowestEnemies( ant_id, enemies );
    x = clamp( Location( location.row+1, location.col+1 ), height, width );
    Debug::stream() << "        calling setLowestEnemies on " << x << std::endl;
    m_grid[ x.row ][ x.col ].setLowestEnemies( ant_id, enemies );
    x = clamp( Location( location.row+1, location.col+2 ), height, width );
    Debug::stream() << "        calling setLowestEnemies on " << x << std::endl;
    m_grid[ x.row ][ x.col ].setLowestEnemies( ant_id, enemies );

    x = clamp( Location( location.row+2, location.col-1 ), height, width );
    Debug::stream() << "        calling setLowestEnemies on " << x << std::endl;
    m_grid[ x.row ][ x.col ].setLowestEnemies( ant_id, enemies );
    x = clamp( Location( location.row+2, location.col-0 ), height, width );
    Debug::stream() << "        calling setLowestEnemies on " << x << std::endl;
    m_grid[ x.row ][ x.col ].setLowestEnemies( ant_id, enemies );
    x = clamp( Location( location.row+2, location.col+1 ), height, width );
    Debug::stream() << "        calling setLowestEnemies on " << x << std::endl;
    m_grid[ x.row ][ x.col ].setLowestEnemies( ant_id, enemies );
}


void Battle::fillEnemyDistance( const Location& location )
{
    const unsigned height = m_map.height();
    const unsigned width  = m_map.width();

    Location x = clamp( Location( location.row-2, location.col-1 ), height, width );
    m_grid[ x.row ][ x.col ].distance_sum += m_map.distance2( location, x );;
    x = clamp( Location( location.row-2, location.col-0 ), height, width );
    m_grid[ x.row ][ x.col ].distance_sum += m_map.distance2( location, x );;
    x = clamp( Location( location.row-2, location.col+1 ), height, width );
    m_grid[ x.row ][ x.col ].distance_sum += m_map.distance2( location, x );;

    x = clamp( Location( location.row-1, location.col-2 ), height, width );
    m_grid[ x.row ][ x.col ].distance_sum += m_map.distance2( location, x );;
    x = clamp( Location( location.row-1, location.col-1 ), height, width );
    m_grid[ x.row ][ x.col ].distance_sum += m_map.distance2( location, x );;
    x = clamp( Location( location.row-1, location.col-0 ), height, width );
    m_grid[ x.row ][ x.col ].distance_sum += m_map.distance2( location, x );;
    x = clamp( Location( location.row-1, location.col+1 ), height, width );
    m_grid[ x.row ][ x.col ].distance_sum += m_map.distance2( location, x );;
    x = clamp( Location( location.row-1, location.col+2 ), height, width );
    m_grid[ x.row ][ x.col ].distance_sum += m_map.distance2( location, x );;

    x = clamp( Location( location.row-0, location.col-2 ), height, width );
    m_grid[ x.row ][ x.col ].distance_sum += m_map.distance2( location, x );;
    x = clamp( Location( location.row-0, location.col-1 ), height, width );
    m_grid[ x.row ][ x.col ].distance_sum += m_map.distance2( location, x );;
    x = clamp( Location( location.row-0, location.col-0 ), height, width );
    m_grid[ x.row ][ x.col ].distance_sum += m_map.distance2( location, x );;
    x = clamp( Location( location.row-0, location.col+1 ), height, width );
    m_grid[ x.row ][ x.col ].distance_sum += m_map.distance2( location, x );;
    x = clamp( Location( location.row-0, location.col+2 ), height, width );
    m_grid[ x.row ][ x.col ].distance_sum += m_map.distance2( location, x );;

    x = clamp( Location( location.row+1, location.col-2 ), height, width );
    m_grid[ x.row ][ x.col ].distance_sum += m_map.distance2( location, x );;
    x = clamp( Location( location.row+1, location.col-1 ), height, width );
    m_grid[ x.row ][ x.col ].distance_sum += m_map.distance2( location, x );;
    x = clamp( Location( location.row+1, location.col-0 ), height, width );
    m_grid[ x.row ][ x.col ].distance_sum += m_map.distance2( location, x );;
    x = clamp( Location( location.row+1, location.col+1 ), height, width );
    m_grid[ x.row ][ x.col ].distance_sum += m_map.distance2( location, x );;
    x = clamp( Location( location.row+1, location.col+2 ), height, width );
    m_grid[ x.row ][ x.col ].distance_sum += m_map.distance2( location, x );;

    x = clamp( Location( location.row+2, location.col-1 ), height, width );
    m_grid[ x.row ][ x.col ].distance_sum += m_map.distance2( location, x );;
    x = clamp( Location( location.row+2, location.col-0 ), height, width );
    m_grid[ x.row ][ x.col ].distance_sum += m_map.distance2( location, x );;
    x = clamp( Location( location.row+2, location.col+1 ), height, width );
    m_grid[ x.row ][ x.col ].distance_sum += m_map.distance2( location, x );;

    // TODO: improve this by avoiding moving multiple ants into same square
    //       just keep list of squares already moved into and add check to below
    //       isWaterOrFood checks

    // The four move locations
    Location x_n = clamp( Location( location.row-1, location.col+0 ), height, width );
    Location x_s = clamp( Location( location.row+1, location.col+0 ), height, width );
    Location x_e = clamp( Location( location.row+0, location.col+1 ), height, width );
    Location x_w = clamp( Location( location.row+0, location.col-1 ), height, width );

    if( !isWaterOrFood( m_map( x_n ) ) )
    {
        Location x = clamp( Location( location.row-3, location.col-1 ), height, width );
        m_grid[ x.row ][ x.col ].distance_sum += m_map.distance2( location, x );;
        x = clamp( Location( location.row-3, location.col-0 ), height, width );
        m_grid[ x.row ][ x.col ].distance_sum += m_map.distance2( location, x );;
        x = clamp( Location( location.row-3, location.col+1 ), height, width );
        m_grid[ x.row ][ x.col ].distance_sum += m_map.distance2( location, x );;
    }

    if( !isWaterOrFood( m_map( x_s ) ) )
    {
        Location x = clamp( Location( location.row+3, location.col-1 ), height, width );
        m_grid[ x.row ][ x.col ].distance_sum += m_map.distance2( location, x );;
        x = clamp( Location( location.row+3, location.col-0 ), height, width );
        m_grid[ x.row ][ x.col ].distance_sum += m_map.distance2( location, x );;
        x = clamp( Location( location.row+3, location.col+1 ), height, width );
        m_grid[ x.row ][ x.col ].distance_sum += m_map.distance2( location, x );;
    }

    if( !isWaterOrFood( m_map( x_w ) ) )
    {
        Location x = clamp( Location( location.row-1, location.col-3 ), height, width );
        m_grid[ x.row ][ x.col ].distance_sum += m_map.distance2( location, x );;
        x = clamp( Location( location.row+0, location.col-3 ), height, width );
        m_grid[ x.row ][ x.col ].distance_sum += m_map.distance2( location, x );;
        x = clamp( Location( location.row+1, location.col-3 ), height, width );
        m_grid[ x.row ][ x.col ].distance_sum += m_map.distance2( location, x );;
    }

    if( !isWaterOrFood( m_map( x_e ) ) )
    {
        Location x = clamp( Location( location.row-1, location.col+3 ), height, width );
        m_grid[ x.row ][ x.col ].distance_sum += m_map.distance2( location, x );;
        x = clamp( Location( location.row+0, location.col+3 ), height, width );
        m_grid[ x.row ][ x.col ].distance_sum += m_map.distance2( location, x );;
        x = clamp( Location( location.row+1, location.col+3 ), height, width );
        m_grid[ x.row ][ x.col ].distance_sum += m_map.distance2( location, x );;
    }

    if( !isWaterOrFood( m_map( x_e ) ) || !isWaterOrFood( m_map( x_n ) ) )
    {
        Location x = clamp( Location( location.row+2, location.col+2 ), height, width );
        m_grid[ x.row ][ x.col ].distance_sum += m_map.distance2( location, x );;
    }

    if( !isWaterOrFood( m_map( x_w ) ) || !isWaterOrFood( m_map( x_n ) ) )
    {
        Location x = clamp( Location( location.row-2, location.col+2 ), height, width );
        m_grid[ x.row ][ x.col ].distance_sum += m_map.distance2( location, x );;
    }

    if( !isWaterOrFood( m_map( x_w ) ) || !isWaterOrFood( m_map( x_s ) ) )
    {
        Location x = clamp( Location( location.row-2, location.col-2 ), height, width );
        m_grid[ x.row ][ x.col ].distance_sum += m_map.distance2( location, x );;
    }

    if( !isWaterOrFood( m_map( x_e ) ) || !isWaterOrFood( m_map( x_s ) ) )
    {
        Location x = clamp( Location( location.row+2, location.col-2 ), height, width );
        m_grid[ x.row ][ x.col ].distance_sum += m_map.distance2( location, x );;
    }
}


void Battle::fillPlusOne( const Location& location, int ant_id, int inc )
{
    assert( 0 <= ant_id && ant_id < 10 );
    const unsigned height = m_map.height();
    const unsigned width  = m_map.width();

    fill( location, ant_id, inc);

    // TODO: improve this by avoiding moving multiple ants into same square
    //       just keep list of squares already moved into and add check to below
    //       isWaterOrFood checks

    // The four move locations
    Location x_n = clamp( Location( location.row-1, location.col+0 ), height, width );
    Location x_s = clamp( Location( location.row+1, location.col+0 ), height, width );
    Location x_e = clamp( Location( location.row+0, location.col+1 ), height, width );
    Location x_w = clamp( Location( location.row+0, location.col-1 ), height, width );

    bool a_n = !isWaterOrFood( m_map( x_n ) ) && m_assigned_tiles.find( x_n ) == m_assigned_tiles.end();
    bool a_s = !isWaterOrFood( m_map( x_s ) ) && m_assigned_tiles.find( x_s ) == m_assigned_tiles.end();
    bool a_w = !isWaterOrFood( m_map( x_w ) ) && m_assigned_tiles.find( x_w ) == m_assigned_tiles.end();
    bool a_e = !isWaterOrFood( m_map( x_e ) ) && m_assigned_tiles.find( x_e ) == m_assigned_tiles.end();

    if( a_n ) 
    {
        Location x = clamp( Location( location.row-3, location.col-1 ), height, width );
        m_grid[ x.row ][ x.col ].attacks[ ant_id ] += inc;
        x = clamp( Location( location.row-3, location.col-0 ), height, width );
        m_grid[ x.row ][ x.col ].attacks[ ant_id ] += inc;
        x = clamp( Location( location.row-3, location.col+1 ), height, width );
        m_grid[ x.row ][ x.col ].attacks[ ant_id ] += inc;
    }

    if( a_s )
    {
        Location x = clamp( Location( location.row+3, location.col-1 ), height, width );
        m_grid[ x.row ][ x.col ].attacks[ ant_id ] += inc;
        x = clamp( Location( location.row+3, location.col-0 ), height, width );
        m_grid[ x.row ][ x.col ].attacks[ ant_id ] += inc;
        x = clamp( Location( location.row+3, location.col+1 ), height, width );
        m_grid[ x.row ][ x.col ].attacks[ ant_id ] += inc;
    }

    if( a_w ) 
    {
        Location x = clamp( Location( location.row-1, location.col-3 ), height, width );
        m_grid[ x.row ][ x.col ].attacks[ ant_id ] += inc;
        x = clamp( Location( location.row+0, location.col-3 ), height, width );
        m_grid[ x.row ][ x.col ].attacks[ ant_id ] += inc;
        x = clamp( Location( location.row+1, location.col-3 ), height, width );
        m_grid[ x.row ][ x.col ].attacks[ ant_id ] += inc;
    }

    if( a_e ) 
    {
        Location x = clamp( Location( location.row-1, location.col+3 ), height, width );
        m_grid[ x.row ][ x.col ].attacks[ ant_id ] += inc;
        x = clamp( Location( location.row+0, location.col+3 ), height, width );
        m_grid[ x.row ][ x.col ].attacks[ ant_id ] += inc;
        x = clamp( Location( location.row+1, location.col+3 ), height, width );
        m_grid[ x.row ][ x.col ].attacks[ ant_id ] += inc;
    }

    if( a_e || a_n )
    {
        Location x = clamp( Location( location.row+2, location.col+2 ), height, width );
        m_grid[ x.row ][ x.col ].attacks[ ant_id ] += inc;
    }

    if( a_w || a_n ) 
    {
        Location x = clamp( Location( location.row-2, location.col+2 ), height, width );
        m_grid[ x.row ][ x.col ].attacks[ ant_id ] += inc;
    }

    if( a_w || a_s )
    {
        Location x = clamp( Location( location.row-2, location.col-2 ), height, width );
        m_grid[ x.row ][ x.col ].attacks[ ant_id ] += inc;
    }

    if( a_e || a_s )
    {
        Location x = clamp( Location( location.row+2, location.col-2 ), height, width );
        m_grid[ x.row ][ x.col ].attacks[ ant_id ] += inc;
    }
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
    m_map( away_loc ).assigned = true;
    m_allies.insert( ant );

    //Debug::stream() << "Battle:      solve1v1 moving ant " << ally << " in " << DIRECTION_CHAR[ away ] << std::endl;
}


// TODO: switch this to minimax scoring (return maximum ally deaths and associated enemy deaths then minimize this)
void Battle::countDeaths( const Location& ally0, const Location& ally1, const Location& enemy,
                          float& p_die, float& p_kill, float& distance )const
{
    int dist0           = m_map.distance2( ally0, enemy );
    int dist1           = m_map.distance2( ally1, enemy );
    int allies_in_range = static_cast<int>( dist0 <= 5 ) + static_cast<int>( dist1 <= 5 );
    float enemy_deaths  = static_cast<float>( allies_in_range >= 1 );
    float ally_deaths   = static_cast<float>( allies_in_range == 1 );
    float positions     = 1.0f;
    distance            = dist0 + dist1;

    Debug::stream() << "      epos: " << enemy << " edeaths: " << enemy_deaths << " mydeaths: " << ally_deaths 
                    << " allyinrange: " << allies_in_range << " d0: " << dist0 << " d1: " << dist1 << std::endl;
    for( int i = 0; i < 4; ++i )
    {
        Location moved_enemy = m_map.getLocation( enemy, static_cast<Direction>( i ) ); 
        
        // TODO: could cache the available directions, pass into func
        if( m_map( moved_enemy ).isWater() ) continue;

        positions          += 1.0f;
        int dist0           = m_map.distance2( ally0, moved_enemy );
        int dist1           = m_map.distance2( ally1, moved_enemy );
        int allies_in_range = static_cast<int>( dist0 <= 5 ) + static_cast<int>( dist1 <= 5 );
        enemy_deaths       += static_cast<float>( allies_in_range >= 1 );
        ally_deaths        += static_cast<float>( allies_in_range == 1 );
        Debug::stream() << "      epos: " << moved_enemy << " edeaths: " << enemy_deaths << " mydeaths: " << ally_deaths
                        << " allyinrange: " << allies_in_range << " d0: " << dist0 << " d1: " << dist1 << std::endl;
        distance     += dist0 + dist1;
    }

    p_die  = ally_deaths  / positions;
    p_kill = enemy_deaths / positions;
}

void Battle::solve2v1( const Location& ally0, const Location& ally1, const Location& enemy, int max_depth,
                       Direction& move0, Direction& move1, float& p_die, float& p_kill )
{
    move0  = NONE;
    move1  = NONE;
    p_die  = 1.0f;
    p_kill = 0.0f; 
    float dist = 1000.0f;

    for( int i = 0; i < 5; ++i )
    {
        Location moved_ally0 = m_map.getLocation( ally0, static_cast<Direction>( i ) ); 
        if( !m_map( moved_ally0 ).isAvailable() && moved_ally0 != ally1 && i != NONE )
        {
            Debug::stream() << "    ally0 location " << moved_ally0 << " not available:" << std::endl
                            << "        " << m_map( moved_ally0 ) << std::endl;
            continue;
        }

        for( int j = 0; j < 5; ++j && i != NONE )
        {
            Location moved_ally1 = m_map.getLocation( ally1, static_cast<Direction>( j ) ); 
            if( moved_ally1 == moved_ally0 || 
              ( !m_map( moved_ally1 ).isAvailable() && moved_ally1 != ally0 && j != NONE ) )
            {
                Debug::stream() << "    ally1 location " << moved_ally1 << " not available:" << std::endl
                                << "        " << m_map( moved_ally0 ) << std::endl;
                continue;
            }
            float cur_p_die = 1.0f, cur_p_kill = 0.0f, cur_dist = 1000.0f;
            countDeaths( moved_ally0, moved_ally1, enemy, cur_p_die, cur_p_kill, cur_dist );

            Debug::stream() << "    trying " << moved_ally0 << "," << moved_ally1 << " p_ally_die:" << cur_p_die 
                            << " p_enemy_die:" << cur_p_kill << " dist:" << cur_dist << std::endl;

            if( cur_p_die <  p_die ||
              ( cur_p_die == p_die && cur_p_kill >  p_kill ) ||
              ( cur_p_die == p_die && cur_p_kill == p_kill && cur_dist < dist ) )
            {
                Debug::stream() << "      found new BEST!!!!!" << std::endl;
                p_die      = cur_p_die;
                p_kill     = cur_p_kill;
                dist       = dist;
                move0      = static_cast<Direction>( i );
                move1      = static_cast<Direction>( j );

                // We have a perfect solution -- bail
                if( p_die <  0.001f && p_kill > 0.999f ) return;
            }
        }
    }
}


void Battle::solve2v1( const Location& ally0, const Location& ally1, const Location& enemy )
{
    Debug::stream() << "solve2v1 " << ally0 << "," << ally1 << " vs " << enemy << std::endl;

    float p_die, p_kill;
    Direction move0, move1;
    solve2v1( ally0, ally1, enemy, 1, move0, move1, p_die, p_kill );

    Debug::stream() << " moving " << ally0 << " in " << DIRECTION_CHAR[ move0 ] << std::endl
                    << " moving " << ally1 << " in " << DIRECTION_CHAR[ move1 ] << std::endl;

    Location moved_ally0 = m_map.getLocation( ally0, move0 );
    Ant* ant             = m_map( ally0 ).ant;
    ant->path.assign( moved_ally0, move0, Path::ATTACK );
    m_map( moved_ally0 ).assigned = true;
    m_allies.insert( ant );

    Location moved_ally1 = m_map.getLocation( ally1, move1 );
    ant = m_map( ally1 ).ant;
    ant->path.assign( moved_ally1, move1, Path::ATTACK );
    m_map( moved_ally1 ).assigned = true;
    m_allies.insert( ant );
}



Battle::Battle( Map& map, AssignedAnts& food_ants )
    : m_map( map ),
      m_food_ants( food_ants )
{
    //
    // create m_grid
    //
    const unsigned height = m_map.height();
    const unsigned width  = m_map.width();
    m_grid = new CombatTile*[ height ];
    for( unsigned i = 0; i < height; ++i )
        m_grid[i] = new CombatTile[ width ];
}


Battle::~Battle()
{
    const unsigned height = m_map.height();
    for( unsigned i = 0; i < height; ++i )
    {
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
            m_grid[i][j].reset();

    m_allies.clear();
    m_enemies.clear();
    m_assigned_tiles.clear();



    //
    // Search from each of our ants to find enemies within 2 steps of battle
    //
    AntEnemies ally_enemies;
    for( Ants::const_iterator it = ants.begin(); it != ants.end(); ++it )
    {
        Debug::stream() << "Pre-processing ant " << *it << std::endl;
        Ant*     ant             = *it;
        Location search_location = ant->location;

        // Do not move static defense ants
        if( ant->assignment == Ant::STATIC_DEFENSE ) continue;

        // If ant has a high priority path target, see if it can avoid battle 
        if( !ant->path.empty() && 
          ( ant->path.goal() == Path::HILL || ant->path.goal() == Path::FOOD ) )
        {
            Location next_loc = m_map.getLocation( search_location, ant->path.nextStep() );
            AnyEnemyAnts    any_enemy_ants( MY_ANT_ID );
            WithinDistance2 within_10( m_map, next_loc, 10 );
            AnyEnemyCombatants bfs( m_map, next_loc, any_enemy_ants, within_10);
            bfs.traverse();
            if( !any_enemy_ants.found_enemy ) continue;

            if( ant->path.goal() == Path::FOOD )
            {
                AssignedAnts::iterator it = m_food_ants.find( ant->path.destination() );
                it->second->path.reset();
                m_food_ants.erase( it );
            }
            ant->path.reset();
            m_map( next_loc ).assigned = false;
        }

        AllEnemyAnts    enemy_ants( search_location, MY_ANT_ID, ally_enemies );
        WithinDistance2 within_battle_range( m_map, search_location, 17 );
        EnemyCombatants bfs( m_map, search_location, enemy_ants, within_battle_range );

        bfs.traverse();
    }

    if( ally_enemies.empty() ) return; // No potential battles found

    Debug::stream() << "Battle:   found " << ally_enemies.size() << " ally ants in battle" << std::endl;

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

    //
    // Splat the ants into the m_grid
    //
    //setFillColor( 255, 0, 255, 0.1f );
    for( AntEnemies::const_iterator it = ally_enemies.begin(); it != ally_enemies.end(); ++it )
    {
        fillPlusOne( it->first, MY_ANT_ID, 1 );
        m_assigned_tiles.insert( it->first );
        //circle( it->first, 1, true );
    }

    //setFillColor( 255, 0, 0, 0.1f );
    for( LocationSet::const_iterator it = m_enemies.begin(); it != m_enemies.end(); ++it )
    {
        fillPlusOne( *it, m_map( *it ).ant_id, 1 );
        m_assigned_tiles.insert( *it );
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
            const Location     ally    = cluster[0].first;
            
            // See if this ant can follow its low priority path
            Ant* ant = m_map( ally ).ant;
            if( !ant->path.empty() ) 
            {
                Location next_loc = m_map.getLocation( ant->location, ant->path.nextStep() );
                AnyEnemyAnts    any_enemy_ants( MY_ANT_ID );
                WithinDistance2 within_10( m_map, next_loc, 10 );
                AnyEnemyCombatants bfs( m_map, next_loc, any_enemy_ants, within_10);
                bfs.traverse();
                if( !any_enemy_ants.found_enemy ) continue;

                ant->path.reset();
                m_map( next_loc ).assigned = false;
            }

            // Special case 1v1 battles
            const LocationSet& enemies = enemy_clusters[i];
            if( enemies.size() == 1 )
            {
                solve1v1( ally, *enemies.begin() );
                continue;
            }

        }

        /*
        // No further attempts to honor pre-existing paths
        for( AntEnemies::const_iterator it = cluster.begin(); it != cluster.end(); ++it )
        {
            Ant* ant = m_map( it->first ).ant;
            if( !ant->path.empty() )
            {
                Location next_loc = m_map.getLocation( ant->location, ant->path.nextStep() );
                ant->path.reset();
                m_map( next_loc ).assigned = false;
            }
        }
        */

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
        // Find the minimum enemy's enemies for each square
        //
        for( AntEnemies::const_iterator it = cluster.begin(); it != cluster.end(); ++it )
        {
            Location loc = it->first;
           
            fillLowestEnemies( loc, MY_ANT_ID );

            Locations neighbors;
            m_map.getNeighbors( loc, isLand, neighbors );
            for( Locations::iterator it = neighbors.begin(); it != neighbors.end(); ++it )
                fillLowestEnemies( *it, MY_ANT_ID );
        }

        for( LocationSet::const_iterator it = m_enemies.begin(); it != m_enemies.end(); ++it )
        {
            Location loc = *it;
            int      ant_id = m_map( loc ).ant_id;

            Debug::stream() << "Filling lowest enemies for enemy: " << ant_id << "-" << loc << std::endl;
            
            fillLowestEnemies( loc, ant_id );
            fillEnemyDistance( loc );

            Locations neighbors;
            m_map.getNeighbors( loc, isLand, neighbors );
            for( Locations::iterator it = neighbors.begin(); it != neighbors.end(); ++it )
            {
                Debug::stream() << "     ant neighbor " << *it << std::endl;
                fillLowestEnemies( *it, ant_id );
            }
        }


        //
        // Now find a move for each ally ant
        //
        for( AntEnemies::const_iterator it = cluster.begin(); it != cluster.end(); ++it )
        {
            // TODO: optimize the number of good tiles taken up by ally moves here -- for now, greedy
            // TODO: add abilty to be aggressive (TIE > SAFE) or defensive (SAFE > TIE)
            Ant*               ant           = m_map( it->first ).ant;
            Location           loc           = it->first;

            Debug::stream() << "Battle: " << *ant << std::endl;
            if( !ant->path.empty() )
            {
                Location           path_loc    = m_map.getLocation( loc, ant->path.nextStep() );
                CombatTile::Result path_result = m_grid[ path_loc.row ][ path_loc.col ].result( MY_ANT_ID ); 
                Debug::stream() << "   checking path: " << path_loc << " result: " << path_result << std::endl;
                if( path_result >= CombatTile::SAFE )
                {
                    //m_map( path_loc ).assigned = true;
                    m_allies.insert( ant );
                    continue;
                }
                ant->path.reset();
                m_map( path_loc ).assigned = false;
            }

            Location           best_location = loc;
            CombatTile::Result best_result   = m_grid[ loc.row ][ loc.col ].result( MY_ANT_ID ); 
            int                best_distance = m_grid[ loc.row ][ loc.col ].distance_sum;

            Locations neighbors;
            m_map.getNeighbors( loc, isAvailable, neighbors );
            for( Locations::iterator it = neighbors.begin(); it != neighbors.end(); ++it )
            {
                Location           cur_location = *it;
                CombatTile::Result cur_result   = m_grid[ cur_location.row ][ cur_location.col ].result( MY_ANT_ID ); 
                int                cur_distance = m_grid[ cur_location.row ][ cur_location.col ].distance_sum;
                Debug::stream() << "       checking result " << cur_location << ": " << cur_result << std::endl;
                if( cur_result > best_result || ( cur_result == best_result && cur_distance < best_distance ) )
                {
                    best_location = cur_location;
                    best_result   = cur_result;
                    best_distance = cur_distance;
                }
            }

            Direction dir = m_map.getDirection( loc, best_location );
            ant->path.assign( best_location, dir, Path::ATTACK );
            m_map( best_location ).assigned = true;
            m_allies.insert( ant );

            Debug::stream() << "Battle:     moving ant " << loc << " in " << DIRECTION_CHAR[ dir ] << std::endl;
        }




        /*
        //
        // Run simulated annealing on each cluster 
        //

        const int MAX_ITERATIONS = 1000;

        Directions cur_moves ( cluster.size(), NONE );
        Directions best_moves( cluster.size(), NONE );
        Score      cur_score  = score( cluster, cur_moves );
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
            Location  cur_loc  = m_map.getLocation( orig_loc, cur_move );

            // Select a transition
            Direction new_move = static_cast<Direction>( ( cur_move + rand()%4 + 1 ) % 5 );
            Location  new_loc  = m_map.getLocation( orig_loc, new_move );

            // Make sure the transition is viable 
            Debug::stream() << "  Transitioning ant " << orig_loc << " cur: " <<  DIRECTION_CHAR[ cur_move ] 
                            << " best: " << DIRECTION_CHAR[ best_moves[ rant ] ] << std::endl;
            Debug::stream() << "     first trying move " << DIRECTION_CHAR[ new_move ] << std::endl;
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
                Debug::stream() << "       switching to move " << DIRECTION_CHAR[ new_move ] << std::endl;
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
                fillPlusOne( new_loc, MY_ANT_ID, -1 );
                fillPlusOne( cur_loc, MY_ANT_ID, +1 );
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
            Ant* ant = m_map( ant_location ).ant;
            Location new_location = m_map.getLocation( ant_location, best_moves[i] );
            ant->path.assign( new_location, best_moves[i], Path::ATTACK );
            m_map( new_location ).assigned = true; 
            m_allies.insert( ant );

            Debug::stream() << "Battle:     moving ant " << ant_location << " in " << DIRECTION_CHAR[ best_moves[i] ]
                            << std::endl;
        }
        */
    }
}

