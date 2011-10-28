
#include "PathFinder.h"
#include "Bot.h"
#include "Path.h"
#include <algorithm>


//constructor
Bot::Bot()
{

};


void Bot::playGame()
{
    m_state.debug() << "Bot:playGame() entered" << std::endl;
    //reads the game parameters and sets up
    std::cin >> m_state;
    m_state.debug() << "  State::setup() ... " << std::endl;
    m_state.setup();
    m_state.debug() << "    done." << std::endl;

    m_state.debug() << "  State::endturn() ... " << std::endl;
    endTurn();
    m_state.debug() << "    done." << std::endl;

    //continues making moves while the game is not over
    while( std::cin >> m_state )
    {
        m_state.debug() << "  game loop ... " << std::endl;

        m_state.updateVisionInformation();
        makeMoves();
        endTurn();
    }
    m_state.debug() << "Bot:playGame() exited" << std::endl;
};


void Bot::makeMoves()
{
    m_state.debug() << "turn " << m_state.turn() << ":" << std::endl;

    m_taken.clear();
    m_state.prioritizeMap();

    m_state.debug() << m_state.map() << std::endl;

    //picks out moves for each ant
    for( int ant = 0; ant < m_state.myAnts().size(); ++ant )
    {
        makeMove( m_state.myAnts()[ant] );
    }

    m_state.debug() << "time taken: " << m_state.timer().getTime() << "ms" << std::endl << std::endl;
};


void Bot::endTurn()
{
    if( m_state.turn() > 0 )
        m_state.reset();
    m_state.endTurn();

    std::cout << "go" << std::endl;
};


namespace 
{
    typedef std::pair<int, Location> Candidate;
    struct CandidateCompare
    {
        bool operator()( const Candidate& c0, const Candidate& c1 )
        { return c0.first > c1.first; }
    };
}

void Bot::makeMove( const Location& cur_location )
{

    std::vector<Candidate> candidates; 

    // Hills 
    const State::Locations& hills = m_state.enemyHills();
    for( State::Locations::const_iterator it = hills.begin(); it != hills.end(); ++it )
    {
        //float dist = m_state.map().distance( cur_location, *it );
        //if( dist <= m_state.viewRadius() )
          candidates.push_back( std::make_pair( 30 - m_state.map().manhattanDistance( cur_location, *it ), *it ) );
    }

    // Food 
    const State::Locations& food = m_state.food();
    for( State::Locations::const_iterator it = food.begin(); it != food.end(); ++it )
    {
        float dist = m_state.map().distance( cur_location, *it );
        if( dist <= m_state.viewRadius() )
          candidates.push_back( std::make_pair( 10 - m_state.map().manhattanDistance( cur_location, *it ), *it ) );
    }

    // Unexplored areas
    const State::LocationSet frontier = m_state.frontier();
    for( State::LocationSet::iterator it = frontier.begin(); it != frontier.end(); ++it )
    {
        candidates.push_back( std::make_pair( 0 - m_state.map().manhattanDistance( cur_location, *it ), *it ) );
    }


    if( candidates.empty() ) 
    {
      for( int d = 0; d < NUM_DIRECTIONS; ++d )
      {
        Location loc = m_state.map().getLocation( cur_location, static_cast<Direction>( d ) );

        // Add helper for checking destinations
        if( m_state.map()( loc.row, loc.col ).isAvailable() )
        {
          m_state.makeMove( cur_location, static_cast<Direction>( d ) );
          break;
        }
      }
      return;
    }
    
    std::sort( candidates.begin(), candidates.end(), CandidateCompare() );
    
    PathFinder path_finder( m_state.map() );
    Path path;
    path_finder.getPath( cur_location, candidates.front().second, path ); 
    
    Direction d = path.nextStep();
    if( d != NONE )
        m_state.makeMove( cur_location, d );

}

void Bot::chooseDestination( const Location& cur_location )
{
    
}
