#ifndef BOT_H_
#define BOT_H_

#include "State.h"
#include <set>
#include <map>

class Location;
class Battle;

///
/// This struct represents your bot in the game of Ants
///
class Bot
{
public:
    Bot();
    
    ~Bot();

    /// plays a single game of Ants
    void playGame();

    /// makes moves for a single turn
    void makeMoves();   

    /// indicates to the engine that it has made its moves
    void endTurn();

private:
    void updateHillList();
    void updateTargetedFood();
    bool checkValidPath( Ant* ant );

    //void battle( std::set<Ant*>& assigned );

    /// Check if this ant should move to attack a hill.  return true if ant assigned a path 
    bool attackHills( Ant* ant );
    
    void makeAssignments();

    void assignToHillAttack( unsigned max_dist );
    void assignToFood( unsigned max_dist, bool allow_overrides );
    void assignToMapPath( Ant* ant );
    void findStaticAnt( const Location& hill, const Location& defense_position );

    /// Move ant according to diffused map priorities
    void makeMove( Ant* ant  );
    void makeUncheckedMove( Ant* ant );

    typedef std::set<Location>  LocationSet;
    typedef std::set<Ant*>      AntSet;

    typedef std::map<Location, Ant*> AssignedAnts;


    LocationSet        m_enemy_hills;
    bool               m_enemy_hills_changed;
    LocationSet        m_targeted_food;
    LocationSet        m_hills_under_attack;

    AssignedAnts       m_food_ants;

    float              m_max_time;
    State              m_state;

    Battle*            m_battle;
};

#endif //BOT_H_
