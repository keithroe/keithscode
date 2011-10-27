#include "PathFinder.h"
#include "Bot.h"
#include "Path.h"

using namespace std;

//constructor
Bot::Bot()
{

};

//plays a single game of Ants.
void Bot::playGame()
{
    m_state.debug() << "Bot:playGame() entered" << std::endl;
    //reads the game parameters and sets up
    cin >> m_state;
    m_state.debug() << "  State::setup() ... " << std::endl;
    m_state.setup();
    m_state.debug() << "    done." << std::endl;

    m_state.debug() << "  State::endturn() ... " << std::endl;
    endTurn();
    m_state.debug() << "    done." << std::endl;

    //continues making moves while the game is not over
    while(cin >> m_state)
    {
        m_state.debug() << "  game loop ... " << std::endl;

        m_state.updateVisionInformation();
        makeMoves();
        endTurn();
    }
    m_state.debug() << "Bot:playGame() exited" << std::endl;
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
  /*
    PathFinder path_finder( m_state.map() );
    Path path;
    path_finder.getPath( cur_location, cur_location, path ); //// TODO: find destination
    
    Direction d = path.nextStep();
    if( d != NONE )
        m_state.makeMove( cur_location, d );

        */
    for( int d = 1; d < NUM_DIRECTIONS; ++d )
    {
        Location loc = m_state.map().getLocation( cur_location, static_cast<Direction>( d ) );

        // Add helper for checking destinations
        if( m_state.map()( loc.row, loc.col ).isAvailable() )
        {
            m_state.makeMove( cur_location, static_cast<Direction>( d ) );
            break;
        }
    }
}
