#include "Bot.h"

using namespace std;

//constructor
Bot::Bot()
{

};

//plays a single game of Ants.
void Bot::playGame()
{
    //reads the game parameters and sets up
    cin >> m_state;
    m_state.setup();
    endTurn();

    //continues making moves while the game is not over
    while(cin >> m_state)
    {
        m_state.updateVisionInformation();
        makeMoves();
        endTurn();
    }
};

//makes the bots moves for the turn
void Bot::makeMoves()
{
    m_state.debug() << "turn " << m_state.turn() << ":" << endl;
    m_state.debug() << m_state << endl;

    //picks out moves for each ant
    for( int ant = 0; ant < m_state.myAnts().size(); ++ant )
    {
        makeMove( m_state.myAnts()[ant] );
    }

    m_state.debug() << "time taken: " << m_state.timer().getTime() << "ms" << endl << endl;
};

//finishes the turn
void Bot::endTurn()
{
    if( m_state.turn() > 0 )
        m_state.reset();
    m_state.endTurn();

    cout << "go" << endl;
};


void Bot::makeMove( const Location& cur_location )
{
    for(int d=0; d<TDIRECTIONS; d++)
    {
        Location loc = m_state.getLocation( cur_location, d );

        if(  m_state.grid()[loc.row][loc.col].content != Square::WATER )
        {
            m_state.makeMove( cur_location, d );
            break;
        }
    }
}
