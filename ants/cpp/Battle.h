
#ifndef BATTLE_H_
#define BATTLE_H_

#include <vector>
#include <set>

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


    Battle( Map& map, LocationSet& targeted_food );
    ~Battle();
             
    void solve( const Ants& ants, const Locations& enemy_ants );

    const AntSet&      getAllies()const    { return m_allies; }
    const LocationSet& getEnemies()const   { return m_enemies; }

private:

    struct Score
    {
        Score();

        bool  operator< ( const Score& s )const;
        bool  operator> ( const Score& s )const;
        bool  operator==( const Score& s )const;
        bool  operator>=( const Score& s )const;
        Score operator+ ( const Score& s )const;
        void  operator+=( const Score& s );

        int enemy_deaths;   // enemy_kills
        int ally_deaths;    // ally deaths
        int attack_depths;  // depth of attacking ants
    };

    friend std::ostream& operator<<( std::ostream& out, const Score& s );

    struct EnemyCount
    {
        EnemyCount();
        EnemyCount( int a, int b, int c );

        void operator+=( const EnemyCount& s );
        
        int sum()const;
        int deepest()const;
        int deepestEnemyCount()const;

        bool beats( const EnemyCount& s )const;
        bool beatsAlly( const EnemyCount& s )const;

        void reset();

        int a, b, c;
    };

    friend std::ostream& operator<<( std::ostream& out, const EnemyCount& s );


    const static int MY_ANT_ID = 0;

    void fill( const Location& location, int ant_id, int inc );
    void fillPlusOne( const Location& location, int ant_id, int inc );

    Score score( const AntEnemies& ant_enemies, const Directions& moves )const;
    float P( Battle::Score old_score, Battle::Score new_score, float temp )const;

    void solve1v1( const Location& ally, const Location& enemy );
    void solve2v1( const Location& ally0, const Location& ally1, const Location& enemy );
    void countDeaths( const Location& ally0, const Location& ally1, const Location& enemy, 
                      int& ally_deaths, int& enemy_deaths, int& distance )const;

    Map&          m_map;
    LocationSet&  m_targeted_food;    //< So we can remove targeted food if ant reassigned

    AntSet        m_allies;  //< Allies used in battle
    LocationSet   m_enemies; //< Enemies used in battle

    EnemyCount*** m_grid; 
};


//------------------------------------------------------------------------------
//
// Score 
//
//------------------------------------------------------------------------------

inline Battle::Score::Score()
    : enemy_deaths( 0 ),
      ally_deaths( 0 ),
      attack_depths( 0 )
{
}


inline bool Battle::Score::operator<( const Battle::Score& s )const
{
#ifdef DEFENSIVE
    return ally_deaths   > s.ally_deaths     ? true  :
           ally_deaths   < s.ally_deaths     ? false :
           enemy_deaths  < s.enemy_deaths    ? true  :
           enemy_deaths  > s.enemy_deaths    ? false :
           attack_depths < s.attack_depths;
#else
    return enemy_deaths  < s.enemy_deaths    ? true  :
           enemy_deaths  > s.enemy_deaths    ? false :
           attack_depths < s.attack_depths   ? true  :
           attack_depths > s.attack_depths   ? false :
           ally_deaths   > s.ally_deaths;
#endif
}


inline bool Battle::Score::operator==( const  Battle::Score& s )const
{
    return enemy_deaths  == s.enemy_deaths &&
           ally_deaths   == s.ally_deaths  &&
           attack_depths == s.attack_depths;
}


inline bool Battle::Score::operator>( const Battle::Score& s )const
{
#ifdef DEFENSIVE
    return ally_deaths     < s.ally_deaths    ? true  :
           ally_deaths     > s.ally_deaths    ? false :
           enemy_deaths    > s.enemy_deaths   ? true  :
           enemy_deaths    < s.enemy_deaths   ? false :
           attack_depths   > s.attack_depths;
#else
    return enemy_deaths    > s.enemy_deaths   ? true  :
           enemy_deaths    < s.enemy_deaths   ? false :
           attack_depths   > s.attack_depths  ? true  :
           attack_depths   < s.attack_depths  ? false :
           ally_deaths     < s.ally_deaths;
#endif
}


inline bool Battle::Score::operator>=( const Battle::Score& s )const
{
    return !( *this < s );
}


inline Battle::Score Battle::Score::operator+( const Battle::Score& s )const
{
    Score t;
    t.enemy_deaths     = enemy_deaths  + s.enemy_deaths;
    t.ally_deaths      = ally_deaths   + s.ally_deaths;
    t.attack_depths    = attack_depths + s.attack_depths;
    return t;
}

inline void Battle::Score::operator+=( const Battle::Score& s )
{
    enemy_deaths  += s.enemy_deaths;
    ally_deaths   += s.ally_deaths;
    attack_depths += s.attack_depths;
}


inline std::ostream& operator<<( std::ostream& out, const Battle::Score& s )
{
    out << s.enemy_deaths << "." << s.ally_deaths << "." << s.attack_depths;
    return out;
}

//------------------------------------------------------------------------------
//
// EnemyCount
//
//------------------------------------------------------------------------------

inline Battle::EnemyCount::EnemyCount()
    : a(0), b(0), c(0)
{
}


inline Battle::EnemyCount::EnemyCount( int a, int b, int c ) 
    : a(a),
      b(b),
      c(c) 
{
}


inline void Battle::EnemyCount::operator+=( const EnemyCount& s )
{ 
    a += s.a;
    b += s.b;
    c += s.c;
}


inline int Battle::EnemyCount::sum()const
{ 
    return a + b + c;
}


inline int Battle::EnemyCount::deepest()const
{ 
    return a ? 3 : b ? 2 : 1;
}


inline int Battle::EnemyCount::deepestEnemyCount()const
{
    return a ? a : b ? b : c;
}


inline bool Battle::EnemyCount::beats( const EnemyCount& s )const
{
    return sum() <= s.sum() && deepest() <= s.deepest() && deepestEnemyCount() <= s.deepestEnemyCount();
}


inline bool Battle::EnemyCount::beatsAlly( const EnemyCount& s )const
{
    return deepestEnemyCount() <= s.deepestEnemyCount() || sum() < s.sum();
}


inline void Battle::EnemyCount::reset()
{
    a = b = c = 0;
}


inline std::ostream& operator<<( std::ostream& out, const Battle::EnemyCount& s )
{
    out << s.a << "." << s.b << "." << s.c;
    return out;
}


#endif // BATTLE_H_


