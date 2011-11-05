
//#include "BF.h"
#include "AStar.h"
#include "BFS.h"
#include "Bot.h"
#include "Debug.h"
#include "Path.h"

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

    // Checks if this ant has a valid path and resets the path if not
    bool hasValidPath( Ant* ant, const Map& map )
    {
        // TODO: check goal to make sure it still has the food, etc
        if( ant->path.empty() ) return false;
        Location next_loc = map.getLocation( ant->location, ant->path.nextStep() );
        return map( next_loc ).isAvailable(); 
    }
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
    // Prioritize map
    //
    for( State::LocationSet::const_iterator it = m_state.frontier().begin(); it != m_state.frontier().end(); ++it )
    {
        std::vector<Location> neighbors;
        m_state.map().getNeighbors( *it, isLand, neighbors );
        for( std::vector<Location>::iterator it = neighbors.begin(); it != neighbors.end(); ++it )
        {
          m_state.map().setPriority( *it, 10 );
        }
    }

    for( State::Ants::const_iterator it = m_state.myAnts().begin(); it != m_state.myAnts().end(); ++it )
    {
        // TODO: more principled choice of weights (based on view dist??), no magic numbers
        m_state.map().setPriority( (*it)->location, -100 );
    }

    for( State::LocationList::const_iterator it = m_state.enemyHills().begin(); it != m_state.enemyHills().end(); ++it )
    {
        m_state.map().setPriority( *it, 1000 );
    }
    

    // TODO: unknown squares??? 
    
    int diffusion_steps = std::max( m_state.rows(), m_state.cols() ) / 2;
    Debug::stream() << " diffusion steps " <<  diffusion_steps << std::endl;
    m_state.map().diffusePriority( diffusion_steps );

    //Debug::stream() << m_state.map() << std::endl;

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
    // First make decisions based on local battles
    //
    // TODO
    // return;

    // 
    // Select a new path if needed
    //
    if( !hasValidPath( ant, m_state.map() ) )
    {
        Debug::stream() << "Looking for new goal based path" << std::endl;
        std::vector<Candidate> candidates; 

        // Hills 
        const State::LocationList& hills = m_state.enemyHills();
        for( State::LocationList::const_iterator it = hills.begin(); it != hills.end(); ++it )
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

        if( !candidates.empty() ) 
        {
            std::sort( candidates.begin(), candidates.end(), CandidateCompare() );

            // TODO: try more than one candidate
            AStar astar( m_state.map(), cur_location, candidates.front().second  ); 
            if( astar.search() )
            {
                Debug::stream() << "  found new goal based path" << std::endl;
                astar.getPath( ant->path );
            }
            else
            {
                Debug::stream() << "  failed to find new goal based path" << std::endl;
            }
        }
    }

    //
    //
    //
    if( hasValidPath( ant, m_state.map() ) )
    {
        Debug::stream() << "  yes valid goal based path" << std::endl;
        Direction dir = ant->path.popNextStep();
        m_state.makeMove( ant, dir );
    }

    //
    // Choose path according to diffusion map priorities
    //
    else
    {
        Debug::stream() << "  no valid goal based path" << std::endl;
        std::vector<Location> neighbors;
        m_state.map().getNeighbors( cur_location, notWater, neighbors );
        if( neighbors.empty() ) return;

        Location move_loc  = *( neighbors.begin() );
        float max_priority = m_state.map().getPriority( move_loc );
        for( std::vector<Location>::iterator it = neighbors.begin()+1; it != neighbors.end(); ++it )
        {
            Location neighbor_loc = *it;
            float neighbor_priority = m_state.map().getPriority( neighbor_loc );
            if( neighbor_priority > max_priority )
            {
                max_priority = neighbor_priority;
                move_loc     = neighbor_loc;
            }
        }

        m_state.makeMove( ant, move_loc );
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

