
//#include "BF.h"
#include "AStar.h"
#include "BFS.h"
#include "Bot.h"
#include "Debug.h"
#include "Path.h"
#include "Square.h"

#include <algorithm>
#include <cstdlib>

//
// Helpers
//

namespace 
{
    struct Candidate
    {
        int        estimate;
        Location   location;
        Path::Goal goal;
    };

    struct CandidateCompare
    {
        bool operator()( const Candidate& c0, const Candidate& c1 )
        { return c0.estimate > c1.estimate; }
    };


    // Checks if this ant has a valid path and resets the path if not
    bool hasValidPath( Ant* ant, const Map& map )
    {
        if( ant->path.empty() )              return false;

        Location next_loc = map.getLocation( ant->location, ant->path.nextStep() );
        if( !map( next_loc ).isAvailable() ) return false; 

        Location goal_loc = ant->path.destination();
        if( ant->path.goal() == Path::FOOD ) return hasFood( map( goal_loc ) );
        if( ant->path.goal() == Path::HILL ) return hasEnemyHill( map( goal_loc ) );

        return true;
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
    // First make 'global' ant decisions such as battle
    //
    std::set<Ant*> battle_ants;
    for( State::Ants::const_iterator it = m_state.myAnts().begin();
         it != m_state.myAnts().end();
         ++it )
    {
        // for now, just move away from lost battles
    }

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
    

    
    int diffusion_steps = std::max( m_state.rows(), m_state.cols() ) / 2;
    Debug::stream() << " diffusion steps " <<  diffusion_steps << std::endl;
    m_state.map().diffusePriority( diffusion_steps );

    Debug::stream() << m_state.map() << std::endl;

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
    Debug::stream() << "makeMove( ant: " << cur_location << " )" << std::endl;

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
        Debug::stream() << "  Looking for new goal based path" << std::endl;
        std::vector<Candidate> candidates; 

        // Hills 
        const State::LocationList& hills = m_state.enemyHills();
        for( State::LocationList::const_iterator it = hills.begin(); it != hills.end(); ++it )
        {
            Candidate c;
            c.estimate = 40 - m_state.map().manhattanDistance( cur_location, *it );
            c.location = *it;
            c.goal     = Path::HILL;
            candidates.push_back( c );
        }

        // Food  TODO: handle food via BFS from food
        const State::Locations& food = m_state.food();
        for( State::Locations::const_iterator it = food.begin(); it != food.end(); ++it )
        {
            // Only add this if it is in our view radius
            float dist = m_state.map().distance( cur_location, *it );
            if( dist <= m_state.viewRadius() )
            {
                Candidate c;
                c.estimate = 10 - m_state.map().manhattanDistance( cur_location, *it );
                c.location = *it;
                c.goal     = Path::FOOD;
                candidates.push_back( c );
            }
        }

        if( !candidates.empty() ) 
        {
            std::sort( candidates.begin(), candidates.end(), CandidateCompare() );

            // TODO: try more than one candidate
            
            Debug::stream() << "      Searching for path to " << candidates.front().location << std::endl;
            AStar astar( m_state.map(), cur_location, candidates.front().location ); 
            if( astar.search() )
            {
                astar.getPath( ant->path );
                ant->path.setGoal( candidates.front().goal );
                Debug::stream() << "      found new goal based path " << ant->path << std::endl;
            }
        }
    }
    else
    {
        Debug::stream() << "  Preexisting valid goal based path " << ant->path << std::endl;
    }

    //
    // If we have a valid path, execute it
    //
    if( hasValidPath( ant, m_state.map() ) )
    {
        Debug::stream() << "  Yes valid goal based path" << std::endl;
        Direction dir = ant->path.popNextStep();
        m_state.makeMove( ant, dir );
    }
    //
    // Else choose path according to diffusion map priorities
    //
    else
    {
        Debug::stream() << "  No valid goal based path - following weights" << std::endl;
        std::vector<Location> neighbors;
        m_state.map().getNeighbors( cur_location, isAvailable, neighbors );
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

