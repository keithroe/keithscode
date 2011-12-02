
//#include "BF.h"
#include "AStar.h"
#include "Battle.h"
#include "BF.h"
#include "BFS.h"
#include "Bot.h"
#include "Debug.h"
#include "Path.h"
#include "Square.h"
#include <fstream>

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


    struct FindEnemyAnts
    {
        FindEnemyAnts( std::vector<Location>& enemies ) : enemies( enemies ) {}

        bool operator()( const BFNode* node )
        {
            if( node->square->ant_id > 0 )
                enemies.push_back( node->loc );
            return true;
        }

        std::vector<Location>& enemies;
    };


    struct FindFoodAnt
    {
        FindFoodAnt( std::set<Ant*>& assigned_ants, bool override_hills ) 
          : assigned_ants( assigned_ants ), override_hills( override_hills ), ant( 0u ), found_ant( false ) {}

        bool operator()( const BFNode* node )
        {
            if( node->square->ant_id == 0 )
            {
                // TODO: allow overrides of any ant -- need to remove ant's old food goal from m_targeted_food
                Ant* cur_ant = node->square->ant;
                if(   cur_ant->path.goal() != Path::ATTACK &&
                    ( cur_ant->path.goal() != Path::FOOD || 
                      ( assigned_ants.find( cur_ant ) != assigned_ants.end() && cur_ant->path.size() > node->depth )) &&
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


    struct Always
    {
        bool operator()( const BFNode* current, const Location& location, const Square& neighbor )
        {
            return true;
        }
    };


    typedef  BF<FindFoodAnt, Always> FindNearestAnt;


    struct AntInSet
    {
        AntInSet( std::set<Ant*>& ant_set ) : ant_set( ant_set ) {}
        bool operator()( Ant* ant ) { return ant_set.find( ant ) != ant_set.end(); }

        const std::set<Ant*>& ant_set;
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
    std::cerr << " maximum time: " << m_max_time << "ms" << std::endl;
}


void Bot::playGame()
{
    Debug::stream() << "Game started" << std::endl;
    
    //std::ifstream infile( "/Users/keithm/Code/keithscode/ants/fluxid_input_16824.txt", std::ifstream::in ); 
    //if( infile.fail() ) throw "arrrggghhh";
    //infile >> m_state;
    
    //reads the game parameters and sets up
    std::cin >> m_state;
    m_state.setup();
    endTurn();

    Debug::stream() << m_state << std::endl;

    //continues making moves while the game is not over
    while( std::cin >> m_state )
    //while( infile >> m_state )

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
    std::set<Ant*> battle_ants;
    std::set<Location> enemy_ants;
    battle( m_state.map(), m_state.myAnts(), m_state.enemyAnts(), battle_ants, enemy_ants );
    available.remove_if( AntInSet( battle_ants ) );

    //
    // Assign ants to very nearby food with high priority
    // TODO: allow this to override targeted food which has been assigned farther away ant
    //
    Debug::stream() << " Assigning short distance food tasks... " << std::endl;
    std::set<Ant*> assigned_ants;
    assignToFood( assigned_ants, 4, true );
    available.remove_if( AntInSet( assigned_ants ) );
    
    //
    // Attack hills 
    //
    Debug::stream() << " Assigning hill-attack tasks... " << std::endl;
    available.remove_if( std::bind1st( std::mem_fun( &Bot::attackHills ), this ) );
    
    //
    // Assign ants to farther food with lower
    //
    Debug::stream() << " Assigning long distance food tasks... " << std::endl;
    assigned_ants.clear();
    assignToFood( assigned_ants, 20, false );
    available.remove_if( AntInSet( assigned_ants ) );

    //
    // Prioritize map
    //

    m_state.map().updatePriority( 1, notVisible );
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
        //
        if( battle_ants.find( *it ) != battle_ants.end() )
            m_state.map().setPriority( (*it)->location,  0 );
        else
            m_state.map().setPriority( (*it)->location, -5 );
    }
    
    for( LocationSet::iterator it = enemy_ants.begin(); it != enemy_ants.end(); ++it )
    {
        m_state.map().setPriority( *it, 100 );
    }

    for( LocationSet::iterator it = m_enemy_hills.begin(); it != m_enemy_hills.end(); ++it )
    {
        m_state.map().setPriority( *it, 1000 );
    }
    
    // Defend base
    for( State::Locations::const_iterator it = m_state.myHills().begin(); it != m_state.myHills().end(); ++it )
    {
        //if( m_state.turn() > 30 ) m_state.map().setPriority( *it, 10 );

        std::vector<Location> base_attackers;
        FindEnemyAnts find_enemy_ants( base_attackers );
        Always always;
        BF<FindEnemyAnts, Always> bfs( m_state.map(), *it, find_enemy_ants, always );
        bfs.setMaxDepth( 20 );
        bfs.traverse();

        // TODO: rally more intelligently, between enemies and base
        if( base_attackers.size() > 0 )
        {
            Debug::stream() << " Rallying to base at " << *it << std::endl;
            m_state.map().setPriority( *it, 100 );
        }
    }
    
    
    Debug::stream() << "Before diffusion" << std::endl
                    << m_state.map() << std::endl;
    int diffusion_steps = std::max( m_state.rows(), m_state.cols() );
    m_state.map().diffusePriority( diffusion_steps );
    Debug::stream() << "After diffusion" << std::endl
                    << m_state.map() << std::endl;

    //
    // Now make moves for individual ants
    //
    std::for_each( battle_ants.begin(), battle_ants.end(),
                   std::bind1st( std::mem_fun( &Bot::makeUncheckedMove), this ) );
    Debug::stream() << " Making moves (path or map based )... " << std::endl;
    for( State::Ants::const_iterator it = m_state.myAnts().begin(); it != m_state.myAnts().end(); ++it )
    {
        // Battle ants have already been moved
        if( battle_ants.find( *it ) == battle_ants.end() )
            makeMove( *it );
    }
    //std::for_each( m_state.myAnts().begin(), m_state.myAnts().end(),
    //               std::bind1st( std::mem_fun( &Bot::makeMove), this ) );
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


/*
void Bot::battle( std::set<Ant*>& assigned )
{
    battle( m_state.map(), m_state.myAnts(), assigned );
}
*/


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
        if( manhattan_distance < 20 )
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
        astar.setMaxDepth( 40 );
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
}


void Bot::makeUncheckedMove( Ant* ant )
{
    // Presumes this ant's next move is valid!!!
    Direction dir = ant->path.popNextStep();
    Debug::stream() << "  unchecked  moving " << ant->location << ": " << DIRECTION_CHAR[dir] << std::endl;
    if( dir != NONE )
        m_state.makeMove( ant, dir );
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
        if( dir != NONE )
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

    if( ant->path.nextStep() == NONE ) 
        return true;

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


