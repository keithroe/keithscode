
#ifndef BATTLE_H_
#define BATTLE_H_

#include <vector>
#include <set>
#include "Debug.h"


class Map;
class Ant;
struct Location;


class Battle
{
public:
    typedef std::vector<Ant*>                               Ants;
    typedef std::set<Ant*>                                  AntSet;
    typedef std::vector<Location>                           Locations;
    typedef std::set<Location>                              LocationSet;
    typedef std::vector< std::pair<Location, Locations> >   AntEnemies;
    typedef std::vector<Direction>                          Directions;

    
    struct CombatTile
    {
        enum Result
        {
            LOSE=0,
            TIE,
            WIN
        };

        CombatTile() { reset(); }

        void reset() 
        { 
            memset( attacks,          0, sizeof( attacks ) );
            memset( lowest_enemies,   1, sizeof( lowest_enemies ) );
            lowest_enemies[0] = lowest_enemies[1] = lowest_enemies[2] = lowest_enemies[3] =
                                lowest_enemies[4] = lowest_enemies[5] = lowest_enemies[6] =
                                lowest_enemies[7] = lowest_enemies[8] = lowest_enemies[9] = 100;
        }

        int enemies( int player )const
        { 
            // TODO: precalculate if necessary
            return attacks[0] + attacks[1] + attacks[2] + attacks[3] + attacks[4] +
                   attacks[5] + attacks[6] + attacks[7] + attacks[8] + attacks[9] -
                   attacks[player];
        }

        int mmin( int a, int b ) { return a < b ? a : b; }
        
        void setLowestEnemies( int player, int enemies )
        {
            for( int i = 0; i < 10; ++i )
                if( i != player && enemies < lowest_enemies[i] )
                    lowest_enemies[i] = enemies;
        }       

        Result result( int player )
        {
            const int player_enemies = enemies( player );
            const int enemy_enemies  = lowest_enemies[ player ];
            Debug::stream() << "  result:  p_enemies: " << player_enemies << " e_enemies: " << enemy_enemies << std::endl;
            return player_enemies < enemy_enemies ? WIN :
                   player_enemies > enemy_enemies ? LOSE :
                   TIE;
                  
        }

        int    attacks[ 10 ];          // Number of ants which could attack this
        int    lowest_enemies[ 10 ];   // Lowest enemy's enemis in range of this
    };


    Battle( Map& map, LocationSet& targeted_food );
    ~Battle();
             
    void solve( const Ants& ants, const Locations& enemy_ants );

    const AntSet&      getAllies()const    { return m_allies; }
    const LocationSet& getEnemies()const   { return m_enemies; }

private:


    const static int MY_ANT_ID = 0;

    void fill( const Location& location, int ant_id, int inc );
    void fillPlusOne( const Location& location, int ant_id, int inc );
    void fillLowestEnemies( const Location& location, int ant_id );



    void solve1v1( const Location& ally, const Location& enemy );
    void solve2v1( const Location& ally0, const Location& ally1, const Location& enemy );
    void countDeaths( const Location& ally0, const Location& ally1, const Location& enemy, 
                      int& ally_deaths, int& enemy_deaths, int& distance )const;

    Map&          m_map;
    LocationSet&  m_targeted_food;    //< So we can remove targeted food if ant reassigned

    AntSet        m_allies;  //< Allies used in battle
    LocationSet   m_enemies; //< Enemies used in battle


    CombatTile** m_grid; 
};



#endif // BATTLE_H_


