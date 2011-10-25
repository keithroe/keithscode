#ifndef BOT_H_
#define BOT_H_

#include "State.h"

///
/// This struct represents your bot in the game of Ants
///
class Bot
{
public:
    Bot();

    /// plays a single game of Ants
    void playGame();

    /// makes moves for a single turn
    void makeMoves();   

    /// indicates to the engine that it has made its moves
    void endTurn();

private:

    void makeMove( const Location& cur_location );
    
    State m_state;
};

#endif //BOT_H_
