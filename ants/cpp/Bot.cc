
#include "BFS.h"
#include "Bot.h"
#include "Debug.h"
#include "Path.h"
#include "PathFinder.h"

#include <algorithm>
#include <cstdlib>

//
// Helpers
//

namespace 
{
    typedef std::pair<int, Location> Candidate;
    struct CandidateCompare
    {
        bool operator()( const Candidate& c0, const Candidate& c1 )
        { return c0.first > c1.first; }
    };
}


//
// Bot implementation
//

Bot::Bot()
    : m_max_time( 0.0f )
{
}


Bot::~Bot()
{
}


void Bot::playGame()
{
    Debug::stream() << "Game started" << std::endl;
    
    //reads the game parameters and sets up
    std::cin >> m_state;
    m_state.setup();
    endTurn();

    Debug::stream() << m_state << std::endl;

    //continues making moves while the game is not over
    while( std::cin >> m_state )
    {
        m_state.updateVisionInformation();
        makeMoves();
        endTurn();
    }
}


void Bot::makeMoves()
{
    Debug::stream() << "turn " << m_state.turn() << ":" << std::endl;

    //
    // First make decision based on local battles
    //

    //
    // Choose important rally points
    //
    
    
    //
    // Now make move choices for individual ants
    //
    for( State::Ants::const_iterator it = m_state.myAnts().begin();
         it != m_state.myAnts().end();
         ++it )
    {
        makeMove( *it );
    }
    
}


void Bot::endTurn()
{
    if( m_state.turn() > 0 )
        m_state.reset();
    m_state.endTurn();

    const float turn_time = m_state.timer().getTime();
    m_max_time = turn_time > m_max_time ? turn_time : m_max_time;
    Debug::stream() << "time taken: \n" << turn_time << "ms.  Max: " << m_max_time << "ms." << std::endl;

    std::cout << "go" << std::endl;
}


void Bot::makeMove( Ant* ant )
{
    const Location cur_location = ant->location;

    //
    // Check to see if there is already a valid path for this ant
    // 
    if( !ant->path.empty() )
    {
        Debug::stream() << "        reusing path" << std::endl;

        // TODO: check if the goal has disappeard (food eaten, etc)
        Direction dir = ant->path.popNextStep();
        Location  loc = m_state.map().getLocation( cur_location, dir );
        if( m_state.map()( loc ).isAvailable() )
        {
            m_state.makeMove( ant, cur_location, dir );
            return;
        }
        else
        {
            ant->path.reset();
        }
    }
    

    std::vector<Candidate> candidates; 

    // Hills 
    const State::LocationList& hills = m_state.enemyHills();
    for( State::LocationList::const_iterator it = hills.begin(); it != hills.end(); ++it )
    {
        if( !( m_state.map()( *it ).isAvailable() ) ) continue;
        {
            candidates.push_back( std::make_pair( 30 - m_state.map().manhattanDistance( cur_location, *it ), *it ) );
        }
    }

    // Food 
    const State::Locations& food = m_state.food();
    for( State::Locations::const_iterator it = food.begin(); it != food.end(); ++it )
    {
        if( !m_state.map()( *it ).isAvailable() ) continue;
        float dist = m_state.map().distance( cur_location, *it );
        if( dist <= m_state.viewRadius() )
          candidates.push_back( std::make_pair( 10 - m_state.map().manhattanDistance( cur_location, *it ), *it ) );
    }

    // Unexplored areas
    const State::LocationSet frontier = m_state.frontier();
    for( State::LocationSet::iterator it = frontier.begin(); it != frontier.end(); ++it )
    {
        if( !m_state.map()( *it ).isAvailable() ) continue;
        candidates.push_back( std::make_pair( 0 - m_state.map().manhattanDistance( cur_location, *it ), *it ) );
    }
    
    if( !candidates.empty() ) 
    {
        std::sort( candidates.begin(), candidates.end(), CandidateCompare() );

        PathFinder path_finder( m_state.map() );
        path_finder.getPath( cur_location, candidates.front().second, ant->path); 

        Direction dir = ant->path.popNextStep();
        if( dir != NONE )
        {
            Location  loc = m_state.map().getLocation( cur_location, dir );
            if( m_state.map()( loc ).isAvailable() )
            {
                m_state.makeMove( ant, cur_location, dir );
                return;
            }
        }
    }

    int offset = rand() % NUM_DIRECTIONS;
    for( int i = 0; i < NUM_DIRECTIONS; ++i )
    {
        Direction d = static_cast<Direction>( (i+offset) % NUM_DIRECTIONS );
        Location loc = m_state.map().getLocation( cur_location, static_cast<Direction>( d ) );

        // Add helper for checking destinations
        if( m_state.map()( loc.row, loc.col ).isAvailable() )
        {
            m_state.makeMove( ant, cur_location, static_cast<Direction>( d ) );
            break;
        }
    }
}
/*
void Bot::makeMove( const Ant& ant )
{
    const Location cur_location = ant.location;
    std::vector<Candidate> candidates; 

    // Hills 
    const State::Locations& hills = m_state.enemyHills();
    for( State::Locations::const_iterator it = hills.begin(); it != hills.end(); ++it )
    {
        if( !( m_state.map()( *it ).isAvailable() ) ) continue;
        //float dist = m_state.map().distance( cur_location, *it );
        //if( dist <= m_state.viewRadius() )
          candidates.push_back( std::make_pair( 30 - m_state.map().manhattanDistance( cur_location, *it ), *it ) );
    }

    // Food 
    const State::Locations& food = m_state.food();
    for( State::Locations::const_iterator it = food.begin(); it != food.end(); ++it )
    {
        if( !m_state.map()( *it ).isAvailable() ) continue;
        float dist = m_state.map().distance( cur_location, *it );
        if( dist <= m_state.viewRadius() )
          candidates.push_back( std::make_pair( 10 - m_state.map().manhattanDistance( cur_location, *it ), *it ) );
    }

    // Unexplored areas
    const State::LocationSet frontier = m_state.frontier();
    for( State::LocationSet::iterator it = frontier.begin(); it != frontier.end(); ++it )
    {
        if( !m_state.map()( *it ).isAvailable() ) continue;
        candidates.push_back( std::make_pair( 0 - m_state.map().manhattanDistance( cur_location, *it ), *it ) );
    }
    
    if( !candidates.empty() ) 
    {
        std::sort( candidates.begin(), candidates.end(), CandidateCompare() );

        PathFinder path_finder( m_state.map() );
        Path path;
        path_finder.getPath( cur_location, candidates.front().second, path ); 

        Direction d = path.nextStep();
        if( d != NONE )
        {
            m_state.makeMove( cur_location, d );
            return;
        }

    }

    int offset = rand() % NUM_DIRECTIONS;
    for( int i = 0; i < NUM_DIRECTIONS; ++i )
    {
        Direction d = static_cast<Direction>( (i+offset) % NUM_DIRECTIONS );
        Location loc = m_state.map().getLocation( cur_location, static_cast<Direction>( d ) );

        // Add helper for checking destinations
        if( m_state.map()( loc.row, loc.col ).isAvailable() )
        {
            m_state.makeMove( cur_location, static_cast<Direction>( d ) );
            break;
        }
    }

}
*/

void Bot::chooseDestination( const Location& cur_location )
{
    
}
