
//#include "BF.h"
#include "AStar.h"
#include "BF.h"
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


    struct FindAnt
    {
        FindAnt() : ant( 0u ), found_ant( false ) {}

        bool operator()( const BFNode* node )
        {
            // TODO: test if ant assigned to hill attack???
            // Make sure that child of goal is available so we guarentee a valid path
            if( node->square->ant_id == 0 &&
                node->square->ant->path.goal() != Path::FOOD &&
                node->child->square->isAvailable() )
            {
                node->getRPath( node->square->ant->path );
                ant       = node->square->ant;
                found_ant = true;
                return false;
            }
            return true;
        }

        Ant*   ant;
        bool   found_ant;
    };


    struct Always
    {
        bool operator()( const BFNode* current, const Square& neighbor )
        {
            return true;
        }
    };

    typedef  BF<FindAnt, Always> FindNearestAnt;

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
        updateHillList();
        updateTargetedFood();
        makeMoves();
        endTurn();
    }
}


void Bot::makeMoves()
{
    Debug::stream() << " ===============================================" << std::endl; 
    Debug::stream() << " turn " << m_state.turn() << ":" << std::endl;

    std::list<Ant*> available( m_state.myAnts().begin(), m_state.myAnts().end() );

    Debug::stream() << " available ants: " << std::endl;
    for( std::list<Ant*>::iterator it = available.begin(); it != available.end(); ++it )
    {
        Debug::stream() << *it << std::endl;
    }

    //
    // Assign ants to food
    //
    std::set<Ant*> assigned_to_food;
    assignToFood( assigned_to_food );
    for( std::list<Ant*>::iterator it = available.begin(); it != available.end(); )
    {
        if( assigned_to_food.find( *it ) != assigned_to_food.end() )
            it = available.erase( it );
        else
            ++it;
    }

    
    //
    // First make important ant decisions such as attack/defend 
    //
    for( std::list<Ant*>::iterator it = available.begin(); it != available.end();  )
    {
        if( attackDefend( *it ) )
            it = available.erase( it );
        else
            ++it;
    }

    //
    // Prioritize map
    // TODO: Add priority for not visible
    //
    for( State::LocationSet::const_iterator it = m_state.frontier().begin(); it != m_state.frontier().end(); ++it )
    {
        std::vector<Location> neighbors;
        m_state.map().getNeighbors( *it, isLand, neighbors );
        for( std::vector<Location>::iterator it = neighbors.begin(); it != neighbors.end(); ++it )
        {
          m_state.map().setPriority( *it, 100 );
        }
    }

    for( State::Ants::const_iterator it = m_state.myAnts().begin(); it != m_state.myAnts().end(); ++it )
    {
        // TODO: more principled choice of weights (based on view dist??), no magic numbers
        m_state.map().setPriority( (*it)->location, -100 );
    }

    for( LocationSet::iterator it = m_enemy_hills.begin(); it != m_enemy_hills.end(); ++it )
    {
        m_state.map().setPriority( *it, 1000 );
    }
    

    
    int diffusion_steps = std::max( m_state.rows(), m_state.cols() ) / 2;
    m_state.map().diffusePriority( diffusion_steps );
    //Debug::stream() << m_state.map() << std::endl;

    //
    // Now make move choices for individual ants
    //
    for( std::list<Ant*>::iterator it = available.begin(); it != available.end(); ++it )
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


bool Bot::attackDefend( Ant* ant )
{
    Debug::stream() << "  Checking for battle " << std::endl;

    // TODO: experiment to see if we want to ignore previous paths (always keep paths to hills)
    if( hasValidPath( ant, m_state.map() ) )
    {
        Debug::stream() << "    No battle move -- previous path exists" << std::endl;
        return false;
    }

    // Check if there is a hill to rally to TODO: rally then attack?? 
    std::vector<Candidate> candidates; 
    for( LocationSet::iterator it = m_enemy_hills.begin(); it != m_enemy_hills.end(); ++it )
    {
        Debug::stream() << "    adding candidate " << *it << std::endl;
        int manhattan_distance =  m_state.map().manhattanDistance( ant->location, *it );
        if( manhattan_distance < 40 )
        {
            Candidate c;
            c.estimate = m_state.map().manhattanDistance( ant->location, *it );
            c.location = *it;
            c.goal     = Path::HILL;
            candidates.push_back( c );
        }
    }

    std::sort( candidates.begin(), candidates.end(), CandidateCompare() );
    for( std::vector<Candidate>::iterator it = candidates.begin(); it != candidates.end(); ++it )
    {
        Debug::stream() << "    Searching for path to " << it->location << std::endl;
        AStar astar( m_state.map(), ant->location, it->location ); 
        if( astar.search() )
        {
            // Set the ants path
            astar.getPath( ant->path );
            ant->path.setGoal( candidates.front().goal );
            Debug::stream() << "      found new goal hill attack path " << ant->path << std::endl;

            // Make the first move
            Direction dir = ant->path.popNextStep();
            Debug::stream() << " attackDefend -  Moving ant " << ant->location << " : " << DIRECTION_CHAR[dir] 
                            << std::endl;
            m_state.makeMove( ant, dir );
            return true;
        }
    }

    return false;
}


void Bot::assignToFood( std::set<Ant*>& assigned_to_food )
{
    for( State::Locations::const_iterator it = m_state.food().begin(); it != m_state.food().end(); ++it )
    {
        if( m_targeted_food.find( *it ) != m_targeted_food.end() )
            continue;

        FindAnt find_ant;
        Always  always;
        FindNearestAnt find_nearest_ant( m_state.map(), *it, find_ant, always );
        find_nearest_ant.setMaxDepth( 20 );
        find_nearest_ant.traverse();
        if( find_ant.found_ant )
        {
            assigned_to_food.insert( find_ant.ant );
            find_ant.ant->path.setGoal( Path::FOOD ); // TODO: investigate design to ensure goal always valid
            
            // Make the first move
            Direction dir = find_ant.ant->path.popNextStep();
            Debug::stream() << " assignToFood -  Moving ant " << find_ant.ant->location << " : " << DIRECTION_CHAR[dir] 
                            << std::endl;
            Debug::stream() << "       inserting " << find_ant.ant << " into assigned_to_food " << std::endl;
            m_state.makeMove( find_ant.ant, dir );
        }
    }
}


void Bot::makeMove( Ant* ant )
{
    const Location cur_location = ant->location;
    Debug::stream() << "makeMove( ant: " << cur_location << " )" << std::endl;

    /*
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
    */

    //
    // If we have a valid path, execute it
    //
    if( hasValidPath( ant, m_state.map() ) )
    {
        Direction dir = ant->path.popNextStep();
        Debug::stream() << "  Yes valid goal based path moving " << ant->location << ": " << dir << std::endl;
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

        Debug::stream() << "    checking " << neighbors.size() << "neighbors" << std::endl;
        Location move_loc  = *( neighbors.begin() );
        float max_priority = m_state.map().getPriority( move_loc );
        Debug::stream() << "      starting with weight " << max_priority << " to " << move_loc << std::endl;
        for( std::vector<Location>::iterator it = neighbors.begin()+1; it != neighbors.end(); ++it )
        {
            Location neighbor_loc = *it;
            float neighbor_priority = m_state.map().getPriority( neighbor_loc );
            Debug::stream() << "      checking weight " << neighbor_priority<< " to " << neighbor_loc<< std::endl;
            if( neighbor_priority > max_priority )
            {
                max_priority = neighbor_priority;
                move_loc     = neighbor_loc;
            }
        }

        Debug::stream() << "    moving " << ant->location << " to " << move_loc << " prio " << max_priority
                        << std::endl;

        m_state.makeMove( ant, move_loc );
    }
}


void Bot::updateHillList()
{
    // Add in all visible hills -- duplicates will be ignored
    m_enemy_hills.insert( m_state.enemyHills().begin(), m_state.enemyHills().end() );

    // Check for any KNOWN razed hills and remove them from list
    std::vector< LocationSet::iterator > remove_these;
    for( LocationSet::iterator it = m_enemy_hills.begin(); it != m_enemy_hills.end(); ++it )
    {
        const Square& square = m_state.map()( *it );
        if( square.visible && square.hill_id < 0 )
            remove_these.push_back( it );
    }
    for( std::vector< LocationSet::iterator >::iterator it = remove_these.begin(); it != remove_these.end(); ++it )
        m_enemy_hills.erase( *it );
}


void Bot::updateTargetedFood()
{
    std::vector< LocationSet::iterator > remove_these;
    for( LocationSet::iterator it = m_targeted_food.begin(); it != m_targeted_food.end(); ++it )
        if( m_state.map()( *it ).food == false )
            remove_these.push_back( it );

    for( std::vector< LocationSet::iterator >::iterator it = remove_these.begin(); it != remove_these.end(); ++it )
        m_targeted_food.erase( *it );
}
