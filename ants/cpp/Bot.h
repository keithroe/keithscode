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
    void makeMove( Ant* ant  );


    std::set<Location> m_location;
    float              m_max_time;
    State              m_state;
};

#endif //BOT_H_
