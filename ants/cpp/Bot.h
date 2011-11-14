#ifndef BOT_H_
#define BOT_H_

#include "State.h"
#include <set>

class Location;

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
    
    void assignToFood( std::set<Ant*>& assigned, unsigned max_dist, bool override_hills );

    /// Move ant according to diffused map priorities
    void makeMove( Ant* ant  );

    typedef std::set<Location>  LocationSet;

    LocationSet        m_enemy_hills;
    LocationSet        m_targeted_food;

    float              m_max_time;
    State              m_state;
};

#endif //BOT_H_
