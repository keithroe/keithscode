
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


    struct FindFoodAnt
    {
        FindFoodAnt( std::set<Ant*>& assigned_ants, bool override_hills ) 
          : assigned_ants( assigned_ants ), override_hills( override_hills ), ant( 0u ), found_ant( false ) {}

        // TODO: clean this up!!!!!!!
        bool operator()( const BFNode* node )
        {
            if( node->square->ant_id == 0 )
            {
                Ant* cur_ant = node->square->ant;
                if(   cur_ant->path.goal() != Path::ATTACK &&
                    ( cur_ant->path.goal() != Path::FOOD || 
                      ( assigned_ants.find( cur_ant ) != assigned_ants.end() && cur_ant->path.size()>node->depth ) ) &&
                    ( cur_ant->path.goal() != Path::HILL || override_hills ) &&
                    node->child->square->isAvailable() )
                {
                    node->getRPath( cur_ant->path );
                    cur_ant->path.setGoal( Path::FOOD );
                    ant       = cur_ant;
                    found_ant = true;
                    return false;
                }
            }
            return true;
        }

        const std::set<Ant*>& assigned_ants;
        bool   override_hills;
        Ant*   ant;
        bool   found_ant;
    };


    struct FindAllAnts
    {
        bool operator()( const BFNode* node )
        {
            if( node->square->ant_id == 0 ) allies.push_back( node->square->ant );
            if( node->square->ant_id >  0 ) enemies.push_back( std::make_pair( node->square->ant_id, node->loc ) );
            return true;
        }
        std::vector<Ant*> allies;
        std::vector< std::pair<int, Location> > enemies;
    };


    struct Always
    {
        bool operator()( const BFNode* current, const Square& neighbor )
        {
            return true;
        }
    };


    typedef  BF<FindFoodAnt, Always> FindNearestAnt;
    typedef  BF<FindAllAnts, Always> FindAnts;
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
        Debug::stream() << "    " << **it << std::endl;
        checkValidPath( *it );
    }

    //
    // Assign ants to attack/defend locally 
    //
    Debug::stream() << " Assigning battle tasks..." << std::endl;
    std::set<Ant*> assigned_ants;
    battle( assigned_ants );
    for( std::list<Ant*>::iterator it = available.begin(); it != available.end(); )
    {
        if( assigned_ants.find( *it ) != assigned_ants.end() )
            it = available.erase( it );
        else
            ++it;
    }

    //
    // Assign ants to very nearby food with high priority
    //
    Debug::stream() << " Assigning short distance food tasks... " << std::endl;
    assignToFood( assigned_ants, 4, true );
    for( std::list<Ant*>::iterator it = available.begin(); it != available.end(); )
    {
        if( assigned_ants.find( *it ) != assigned_ants.end() )
            it = available.erase( it );
        else
            ++it;
    }
    
    //
    // Attack hills 
    //
    Debug::stream() << " Assigning hill-attack tasks... " << std::endl;
    for( std::list<Ant*>::iterator it = available.begin(); it != available.end();  )
    {
        if( attackHills( *it ) )
            it = available.erase( it );
        else
            ++it;
    }
    
    //
    // Assign ants to farther food with lower
    //
    Debug::stream() << " Assigning long distance food tasks... " << std::endl;
    assigned_ants.clear();
    assignToFood( assigned_ants, 20, false );
    for( std::list<Ant*>::iterator it = available.begin(); it != available.end(); )
    {
        if( assigned_ants.find( *it ) != assigned_ants.end() )
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
    Debug::stream() << m_state.map() << std::endl;

    //
    // Now make move choices for individual ants
    //
    Debug::stream() << " Making moves (path or map based )... " << std::endl;
    std::for_each( m_state.myAnts().begin(), m_state.myAnts().end(),
                   std::bind1st( std::mem_fun( &Bot::makeMove), this ) );
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


void Bot::battle( std::set<Ant*>& assigned )
{
    // TODO: add positive weights for battling ants
    for( State::Ants::const_iterator it = m_state.myAnts().begin(); it != m_state.myAnts().end(); ++it )
    {
        FindAllAnts find_all;
        Always      always;
        FindAnts bfs( m_state.map(), (*it)->location, find_all, always );
        bfs.setMaxDepth( 8 );
        bfs.traverse();
        Debug::stream() << "finding all ants around " << (*it)->location << std::endl;
        Debug::stream() << "    allies:" <<  std::endl;
        for( std::vector<Ant*>::iterator it = find_all.allies.begin(); it != find_all.allies.end(); ++it )
            Debug::stream() << "        " <<  **it <<  std::endl;
       
        Debug::stream() << "    enemies:" <<  std::endl;
        for( std::vector< std::pair<int, Location> >::iterator it = find_all.enemies.begin();
             it != find_all.enemies.end();
             ++it )
            Debug::stream() << "        " <<  it->first << ":" << it->second <<  std::endl;


        if( !find_all.enemies.empty() )
        {
            std::vector<Location> locs;
            for( std::vector< std::pair<int, Location> >::iterator it = find_all.enemies.begin();
                 it != find_all.enemies.end();
                 ++it )
                locs.push_back( it->second );

            Location enemy_centroid = m_state.map().computeCentroid( locs );
            if( find_all.enemies.size() >= find_all.allies.size() )
            { 
                for( std::vector<Ant*>::iterator it = find_all.allies.begin(); it != find_all.allies.end(); ++it )
                {
                    Ant* ant = *it;
                    Direction d = m_state.map().getDirection( enemy_centroid, ant->location );
                    // TODO: check if the dir is blocked
                    ant->path.assign( m_state.map().getLocation( ant->location, d  ), &d, &d+1 );
                    ant->path.setGoal( Path::ATTACK );
                }
            }
            else
            {
                for( std::vector<Ant*>::iterator it = find_all.allies.begin(); it != find_all.allies.end(); ++it )
                {
                    Ant* ant = *it;
                    Direction d = m_state.map().getDirection( ant->location, enemy_centroid );
                    ant->path.assign( m_state.map().getLocation( ant->location, d ), &d, &d+1 );
                    ant->path.setGoal( Path::ATTACK );
                }
            }
        }
    }
}


bool Bot::attackHills( Ant* ant )
{
    Debug::stream() << "  Checking for battle " << std::endl;

    // TODO: experiment to see if we want to ignore previous paths (always keep paths to hills)
    if( checkValidPath( ant ) )
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
        astar.setMaxDepth( 50 );
        if( astar.search() )
        {
            // Set the ants path
            astar.getPath( ant->path );
            ant->path.setGoal( it->goal );
            Debug::stream() << "      found new goal hill attack path " << ant->path << std::endl;
            return true;
        }
    }

    return false;
}


void Bot::assignToFood( std::set<Ant*>& assigned_to_food, unsigned max_dist, bool override_hills )
{
    assert( assigned_to_food.empty() );

    for( State::Locations::const_iterator it = m_state.food().begin(); it != m_state.food().end(); ++it )
    {
        Debug::stream() << " Searching for ant to collect food: " << *it << std::endl;
        if( m_targeted_food.find( *it ) != m_targeted_food.end() )
        {
            Debug::stream() << "   already targeted" << std::endl; 
            continue;
        }
        FindFoodAnt    find_ant( assigned_to_food, override_hills );
        Always         always;
        FindNearestAnt find_nearest_ant( m_state.map(), *it, find_ant, always );
        find_nearest_ant.setMaxDepth( max_dist );
        find_nearest_ant.traverse();
        if( find_ant.found_ant )
        {
            Debug::stream() << "    assigning ant " << *(find_ant.ant) << std::endl;
            assigned_to_food.insert( find_ant.ant );
        }
        else
        {
            Debug::stream() << "    no path found" << std::endl;
        }
    }
            
    for( std::set<Ant*>::iterator it = assigned_to_food.begin(); it != assigned_to_food.end(); ++it )
    {
        m_targeted_food.insert( (*it)->path.destination() );
    }
    /*
    for( State::Locations::const_iterator it = m_state.food().begin(); it != m_state.food().end(); ++it )
    {
        Debug::stream() << " Searching for ant to collect food: " << *it << std::endl;
        if( m_targeted_food.find( *it ) != m_targeted_food.end() )
        {
            Debug::stream() << "   already targeted" << std::endl; 
            continue;
        }

        FindFoodAnt find_ant( override_hills );
        Always  always;
        FindNearestAnt find_nearest_ant( m_state.map(), *it, find_ant, always );
        find_nearest_ant.setMaxDepth( max_dist );
        find_nearest_ant.traverse();
        if( find_ant.found_ant )
        {
            assigned_to_food.insert( find_ant.ant );
            find_ant.ant->path.setGoal( Path::FOOD ); // TODO: investigate design to ensure goal always valid
            m_targeted_food.insert( *it );
        }
        else
        {
            Debug::stream() << "    no path found" << std::endl;
        }
    }
    */
}


void Bot::makeMove( Ant* ant )
{
    const Location cur_location = ant->location;
    Debug::stream() << "makeMove( ant: " << cur_location << " )" << std::endl;

    //
    // If we have a valid path, execute it
    //
    if( checkValidPath( ant ) )
    {
        Direction dir = ant->path.popNextStep();
        Debug::stream() << "  Yes valid goal based path moving " << ant->location << ": " << DIRECTION_CHAR[dir]
                        << std::endl;
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


// Checks if this ant has a valid path and resets the path if not
bool Bot::checkValidPath( Ant* ant )
{
    Debug::stream() << " Checking validity of path " << ant->path << std::endl;
    if( ant->path.empty() ) 
        return false;

    const Map& map = m_state.map();
    Location next_loc = map.getLocation( ant->location, ant->path.nextStep() );
    if( !map( next_loc ).isAvailable() ) 
    {
        Debug::stream() << "     resetting path: next sq not avail" << std::endl;
        if( ant->path.goal() == Path::FOOD )
            m_targeted_food.erase( ant->path.destination() );
        ant->path.reset();
        return false;
    }

    Location goal_loc = ant->path.destination();
    if(  ant->path.goal() == Path::FOOD && !hasFood( map( goal_loc ) ) )
    {
        Debug::stream() << "     resetting path: food gone" << std::endl;
        m_targeted_food.erase( ant->path.destination() );
        ant->path.reset();
        return false;
    } 

    if ( ant->path.goal() == Path::HILL && m_enemy_hills.find( goal_loc ) == m_enemy_hills.end() )
    {
        Debug::stream() << "     resetting path: hill gone" << std::endl;
        Debug::stream() << "        " << map( goal_loc ) << std::endl; 

        ant->path.reset();
        return false;
    }

    return true;
}


