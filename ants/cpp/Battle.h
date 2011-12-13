
#ifndef BATTLE_H_
#define BATTLE_H_

#include <vector>
#include <set>
#include <map>
#include "Debug.h"


class Map;
class Ant;
struct Location;
struct CombatTile;


class Battle
{
public:
    typedef std::vector<Ant*>                               Ants;
    typedef std::set<Ant*>                                  AntSet;

    typedef std::vector<Location>                           Locations;
    typedef std::set<Location>                              LocationSet;

    typedef std::vector< std::pair<Location, Locations> >   AntEnemies;
    typedef std::vector<Direction>                          Directions;

    typedef std::map< Location, Ant* >                      AssignedAnts;
    

    Battle( Map& map, AssignedAnts& food_ants );
    ~Battle();
             
    void solve( const Ants& ants, const Locations& enemy_ants );

    const AntSet&      getAllies()const    { return m_allies; }
    const LocationSet& getEnemies()const   { return m_enemies; }

private:


    const static int MY_ANT_ID = 0;

    void fill( const Location& location, int ant_id, int inc );
    void fillPlusOne( const Location& location, int ant_id, int inc );
    void fillLowestEnemies( const Location& location, int ant_id );
    void fillEnemyDistance( const Location& location );


    void solve1v1( const Location& ally, const Location& enemy );
    void solve2v1( const Location& ally0, const Location& ally1, const Location& enemy );
    void countDeaths( const Location& ally0, const Location& ally1, const Location& enemy, 
                      int& ally_deaths, int& enemy_deaths, int& distance )const;

    Map&          m_map;
    AssignedAnts& m_food_ants;        //< So we can remove targeted food if ant reassigned

    AntSet        m_allies;           //< Allies used in battle
    LocationSet   m_enemies;          //< Enemies used in battle
    LocationSet   m_assigned_tiles;   //< Tiles already used in fill methods 


    CombatTile** m_grid; 
};



#endif // BATTLE_H_


