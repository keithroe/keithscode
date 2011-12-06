
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
    typedef std::vector<Location> Locations;
    typedef std::set<Ant*>        AntSet;

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

    struct AttackAnts
    {
        AttackAnts( AntSet& assigned, int num_ants )  
            : assigned( assigned ), num_ants( num_ants ), ants_found( 0 ) {}

        bool operator()( const BFNode* node )
        {
            // TODO: might also make this probabilistic so we retain explore ants in area 
            if( node->square->ant_id == 0 )
            {
                Ant* cur_ant = node->square->ant;
                if( cur_ant->assignment != Ant::STATIC_DEFENSE &&
                    cur_ant->assignment != Ant::DEFENSE )
                {
                    assigned.insert( cur_ant );
                    cur_ant->assignment = Ant::ATTACK;
                    ++ants_found;
                }
            }
            return ants_found < num_ants;
        }

        AntSet&   assigned;
        const int num_ants;
        int       ants_found;
    };
    
    struct DefenseAnts 
    {
        DefenseAnts( AntSet& assigned, int num_ants )  
            : assigned( assigned ), num_ants( num_ants ), ants_found( 0 ) {}

        bool operator()( const BFNode* node )
        {
            if( node->square->ant_id == 0 )
            {
                Ant* cur_ant = node->square->ant;
                if( cur_ant->assignment != Ant::STATIC_DEFENSE )
                {
                    assigned.insert( cur_ant );
                    cur_ant->assignment = Ant::DEFENSE;
                    ++ants_found;
                }
            }
            return ants_found < num_ants;
        }

        AntSet&   assigned;
        const int num_ants;
        int       ants_found;
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
        AntInSet( const std::set<Ant*>& ant_set ) : ant_set( ant_set ) {}
        bool operator()( Ant* ant ) { return ant_set.find( ant ) != ant_set.end(); }

        const std::set<Ant*>& ant_set;
    };
}


//
// Bot implementation
//

Bot::Bot()
    : m_enemy_hills_changed( false ),
      m_max_time( 0.0f ),
      m_battle( 0 )
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
    m_battle = new Battle( m_state.map() );

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

    // Find all hills which are under attack 
    for( State::Locations::const_iterator it = m_state.myHills().begin(); it != m_state.myHills().end(); ++it )
    {
        std::vector<Location> base_attackers;
        FindEnemyAnts find_enemy_ants( base_attackers );
        Always always;
        BF<FindEnemyAnts, Always> bfs( m_state.map(), *it, find_enemy_ants, always );
        bfs.setMaxDepth( 20 );
        bfs.traverse();

        if( base_attackers.size() > 0 )
        {
            // TODO: store number of enemy ants seen
            Debug::stream() << " Rallying to base at " << *it << std::endl;
            m_hills_under_attack.insert( *it );
        }
    }

    makeAssignments();

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
    m_battle->solve( m_state.myAnts(), m_state.enemyAnts() );
    available.remove_if( AntInSet( m_battle->getAllies() ) );

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

    m_state.map().updatePriority( Map::EXPLORE, 1, notVisible );
    for( State::LocationSet::const_iterator it = m_state.frontier().begin(); it != m_state.frontier().end(); ++it )
    {
        std::vector<Location> neighbors;
        m_state.map().getNeighbors( *it, isLand, neighbors );
        for( std::vector<Location>::iterator it = neighbors.begin(); it != neighbors.end(); ++it )
        {
          m_state.map().setPriority( Map::EXPLORE, *it, 100 );
        }
    }

    for( State::Ants::const_iterator it = m_state.myAnts().begin(); it != m_state.myAnts().end(); ++it )
    {
        //if( m_battle->getAllies().find( *it ) == m_battle->getAllies().end() )
        m_state.map().setPriority( Map::EXPLORE, (*it)->location, -10 );
    }
    
    for( LocationSet::iterator it = m_battle->getEnemies().begin(); it != m_battle->getEnemies().end(); ++it )
    {
    //    m_state.map().setDistanceTarget( Map::ATTACK, *it, 15 );
    }

    for( LocationSet::iterator it = m_enemy_hills.begin(); it != m_enemy_hills.end(); ++it )
    {
        m_state.map().setDistanceTarget( Map::ATTACK, *it, 0 );
    }
    m_state.map().computeDistanceMap( Map::ATTACK );
    
        
    for( LocationSet::iterator it = m_hills_under_attack.begin(); it != m_hills_under_attack.end(); ++it )
    {
        // TODO: rally more intelligently, between enemies and base. also, count
        //       number of enemies and respond proportionately
    
        m_state.map().setDistanceTarget( Map::DEFENSE, *it, 50 );
    }
    m_state.map().computeDistanceMap( Map::DEFENSE );
    
    Debug::stream() << "Before moves " << std::endl
                    << m_state.map() << std::endl;
    
    int diffusion_steps = std::max( m_state.rows(), m_state.cols() );
    m_state.map().diffusePriority( Map::EXPLORE, diffusion_steps );

    //
    // Now make moves for individual ants
    //
    std::for_each( m_battle->getAllies().begin(),
                   m_battle->getAllies().end(),
                   std::bind1st( std::mem_fun( &Bot::makeUncheckedMove), this ) );
    Debug::stream() << " Making moves (path or map based )... " << std::endl;
    for( State::Ants::const_iterator it = m_state.myAnts().begin(); it != m_state.myAnts().end(); ++it )
    {
        // Battle ants have already been moved
        if( m_battle->getAllies().find( *it ) == m_battle->getAllies().end() )
            makeMove( *it );
    }
    
    Debug::stream() << "After moves " << std::endl
                    << m_state.map() << std::endl;
}


void Bot::endTurn()
{
    if( m_state.turn() > 0 )
        m_state.reset();
    m_state.endTurn();
    m_hills_under_attack.clear();

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
    //
    // TODO: can probably bump down the search radius for hills since we have hill_dist map now
    //
    
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
        m_state.makeMove( ant, dir );
    }
    //
    // Else choose path according to diffusion map priorities
    //
    else
    {
        // Leave static defenders alone
        if( ant->assignment == Ant::STATIC_DEFENSE )
            return;

        Debug::stream() << "  No valid goal based path - following weights" << std::endl;
        std::vector<Location> neighbors;
        m_state.map().getNeighbors( cur_location, isAvailable, neighbors );
        if( neighbors.empty() ) return;

        Debug::stream() << "    checking " << neighbors.size() << "neighbors" << std::endl;
        Location move_loc  = *( neighbors.begin() );

        if( ant->assignment == Ant::EXPLORE )
        {
            float max_priority = m_state.map().getPriority( Map::EXPLORE, move_loc );
            Debug::stream() << "      starting EXPLORE weight " << max_priority << " to " << move_loc << std::endl;
            for( std::vector<Location>::iterator it = neighbors.begin()+1; it != neighbors.end(); ++it )
            {
                Location neighbor_loc = *it;
                float neighbor_priority = m_state.map().getPriority( Map::EXPLORE, neighbor_loc );
                Debug::stream() << "      checking weight " << neighbor_priority<< " to " << neighbor_loc<< std::endl;
                if( neighbor_priority > max_priority )
                {
                    max_priority = neighbor_priority;
                    move_loc     = neighbor_loc;
                }
            }

            Debug::stream() << "    moving " << ant->location << " to " << move_loc << " prio " << max_priority
                            << std::endl;
        } else {
            Map::PriorityType priority_type = static_cast<Map::PriorityType>( ant->assignment );
            float min_distance = m_state.map().getPriority( priority_type, move_loc );
            Debug::stream() << "      starting distance " << min_distance << " to " << move_loc << std::endl;
            for( std::vector<Location>::iterator it = neighbors.begin()+1; it != neighbors.end(); ++it )
            {
                Location neighbor_loc = *it;
                float neighbor_distance = m_state.map().getPriority( priority_type, neighbor_loc );
                Debug::stream() << "      checking distance " << neighbor_distance << " to " << neighbor_loc << std::endl;
                if( neighbor_distance < min_distance )
                {
                    min_distance = neighbor_distance;
                    move_loc     = neighbor_loc;
                }
            }

            Debug::stream() << "    moving " << ant->location << " to " << move_loc << " dist " << min_distance 
                            << std::endl;
        }

        m_state.makeMove( ant, move_loc );
    }
}


void Bot::updateHillList()
{
    m_enemy_hills_changed = false;
    // Add in all visible hills -- duplicates will be ignored
    for( Locations::const_iterator it = m_state.enemyHills().begin(); it != m_state.enemyHills().end(); ++it )
        m_enemy_hills_changed |=  m_enemy_hills.insert( *it ).second;

    // Check for any KNOWN razed hills and remove them from list
    std::vector< LocationSet::iterator > remove_these;
    for( LocationSet::iterator it = m_enemy_hills.begin(); it != m_enemy_hills.end(); ++it )
    {
        const Square& square = m_state.map()( *it );
        if( square.visible && square.hill_id < 0 )
            remove_these.push_back( it );
    }
    m_enemy_hills_changed |= !remove_these.empty();
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

void findStaticAnt( const Map& map, const Location& hill, const Location& defense_position )
{
    if( map( defense_position ).ant_id == 0 )
    {
        map( defense_position ).ant->assignment = Ant::STATIC_DEFENSE;
        map( defense_position ).ant->path.reset();
        return;
    }

    Locations neighbors;
    map.getNeighbors( defense_position, hasAnt, neighbors );

    // Check if an ant is on neighboring square 
    for( Locations::iterator it = neighbors.begin(); it != neighbors.end(); ++it )
    {
        Location neighbor = *it;
        Ant*     ant      = map( neighbor ).ant;
        if( ant->assignment != Ant::STATIC_DEFENSE ||
            ant->path.destination() == defense_position )
        {
            ant->assignment = Ant::STATIC_DEFENSE;
            ant->path.assign( defense_position, map.getDirection( neighbor, defense_position ) );
            return;
        }
    }
    
    // Grab spawning ant if one exists
    if( map( hill ).ant_id == 0 && 
        map( hill ).ant->assignment != Ant::STATIC_DEFENSE  )
    {
        Ant* ant = map( hill ).ant;
        Direction dir0 = hill.row - defense_position.row == 1 ? NORTH : SOUTH;
        Direction dir1 = hill.col - defense_position.col == 1 ? WEST  : EAST;
        
        Location loc = map.getLocation( hill, dir0 );
        if( map( loc ).isAvailable() )
        {
            ant->assignment = Ant::STATIC_DEFENSE;
            std::vector<Direction> dirs;
            dirs.push_back( dir0 );
            dirs.push_back( dir1 );
            ant->path.assign( defense_position, dirs.begin(), dirs.end() );
        }

        loc = map.getLocation( hill, dir1 );
        if( map( loc ).isAvailable() )
        {
            ant->assignment = Ant::STATIC_DEFENSE;
            std::vector<Direction> dirs;
            dirs.push_back( dir1 );
            dirs.push_back( dir0 );
            ant->path.assign( defense_position, dirs.begin(), dirs.end() );
        }
    }
}


void Bot::makeAssignments()
{
    int num_ants = m_state.myAnts().size();
    int num_ants_per_base    = num_ants;
    int static_ants_per_base = 0;
    if( !m_state.myHills().empty() )
    {
        num_ants_per_base    = num_ants / m_state.myHills().size();
        static_ants_per_base = num_ants_per_base >= 64 ? 4 :
                               num_ants_per_base >= 48 ? 3 :
                               num_ants_per_base >= 32 ? 2 :
                               num_ants_per_base >= 16 ? 1 :
                               0;
    }


    // Static defense assignments first
    if( static_ants_per_base > 0 )
    {
        for( State::Locations::const_iterator it = m_state.myHills().begin(); it != m_state.myHills().end(); ++it )
        {
            const Location hill_east = m_state.map().getLocation( *it, EAST );
            const Location defender0 = m_state.map().getLocation( hill_east, NORTH );
            findStaticAnt( m_state.map(), *it, defender0 );

            if( static_ants_per_base <= 1 ) continue;
            
            const Location defender1 = m_state.map().getLocation( hill_east, SOUTH );
            findStaticAnt( m_state.map(), *it, defender1 );
            
            if( static_ants_per_base <= 2 ) continue;

            const Location hill_west = m_state.map().getLocation( *it, WEST );
            const Location defender2 = m_state.map().getLocation( hill_west, NORTH );
            findStaticAnt( m_state.map(), *it, defender2 );
            
            if( static_ants_per_base <= 3 ) continue;

            Location defender3 = m_state.map().getLocation( hill_west, SOUTH );
            findStaticAnt( m_state.map(), *it, defender3 );
        }
    }

    int max_explore_ants = ( m_state.rows() / 16 ) * ( m_state.cols() / 16 );
    int max_defense_ants = m_hills_under_attack.size()  * 16; 
    int explore_ants = std::min( max_explore_ants, num_ants / 2 );
    int defense_ants = std::min( max_defense_ants, num_ants / 8 );
    int attack_ants  = num_ants - explore_ants - defense_ants;
    
    int cur_explore_ants        = 0;
    int cur_defense_ants        = 0;
    int cur_attack_ants         = 0;
    int cur_static_defense_ants = 0;
    for( State::Ants::const_iterator it = m_state.myAnts().begin(); it != m_state.myAnts().end(); ++it )
    {
        switch ( (*it)->assignment )
        {
            case Ant::EXPLORE:         ++cur_explore_ants;         break;
            case Ant::DEFENSE:         ++cur_defense_ants;         break;
            case Ant::ATTACK:          ++cur_attack_ants;          break;
            case Ant::STATIC_DEFENSE:  ++cur_static_defense_ants;  break;
            default:                                               break;
        }
    }


    // 
    // TODO: - make assignments more persistant.
    //       - attack ants should know which hill they were attacking so they
    //         can be reassigned when hill falls.
    //       - better distribution of explore ants chosen.
    //       - make non-explore ants only get VERY nearby food
    //       - make explore ants avoid battles
    //       - can explore algoriithm punish cul-de-sacs?
    //       - rethink attackHills function which A*s ants all nearby ants to hills
    //         (can probably do away with it)
    //       - Make battle function ignore STATIC_DEFENSE ants
    //       - Multiple start state BF for attacking multiple enemy hills
    //
    Debug::stream() << "Assigning ants:" << std::endl
                    << "    total_ants    : " << num_ants << std::endl
                    << "    enemy_hills   : " << m_enemy_hills.size() << std::endl
                    << "    attacked_hills: " << m_hills_under_attack.size() << std::endl
                    << "    explore_ants  : " << explore_ants << std::endl
                    << "    defense_ants  : " << defense_ants << std::endl
                    << "    attack_ants   : " << attack_ants << std::endl;
    
    Debug::stream() << "Previous ants:" << std::endl
                    << "    explore_ants  : " << cur_explore_ants << std::endl
                    << "    defense_ants  : " << cur_defense_ants << std::endl
                    << "    sdefense_ants : " << cur_static_defense_ants << std::endl
                    << "    attack_ants   : " << cur_attack_ants << std::endl;

    AntSet assigned_defense;
    if( !m_hills_under_attack.empty() )
    {
        int ants_per_hill = defense_ants / m_hills_under_attack.size();
        for( LocationSet::iterator it = m_hills_under_attack.begin(); it != m_hills_under_attack.end(); ++it )
        {
                DefenseAnts defense_ants( assigned_defense, ants_per_hill );
                Always      always;
                BF<DefenseAnts, Always> find_defense_ants( m_state.map(), *it, defense_ants, always );
                find_defense_ants.setMaxDepth( 50 );
                find_defense_ants.traverse();
        }
        Debug::stream() << "        assigned " << assigned_defense.size() << " defense ants " << std::endl;
    }

    AntSet assigned_attack;
    if( !m_enemy_hills.empty() )
    {
        if( m_enemy_hills_changed )
        {
            Debug::stream() << " enemy_hills_change = true " << std::endl;

            int ants_per_enemy_hill = attack_ants / m_enemy_hills.size();

            // Find the n closest ants to the target and assign them to attack
            for( LocationSet::iterator it = m_enemy_hills.begin(); it != m_enemy_hills.end(); ++it )
            {
                AttackAnts attack_ants( assigned_attack, ants_per_enemy_hill );
                Always     always;
                BF<AttackAnts, Always> find_attack_ants( m_state.map(), *it, attack_ants, always );
                find_attack_ants.setMaxDepth( 500 );
                find_attack_ants.traverse();
            }
            Debug::stream() << "        assigned " << assigned_attack.size() << " attack ants " << std::endl;
        }
    }

    for( State::Ants::const_iterator it = m_state.myAnts().begin(); it != m_state.myAnts().end(); ++it )
    {
        Ant* ant = *it;
        if( ant->assignment != Ant::STATIC_DEFENSE                &&
            assigned_attack.find( ant ) == assigned_defense.end() && 
            assigned_attack.find( ant ) == assigned_attack.end() )
            ant->assignment = Ant::EXPLORE;
    }

    cur_explore_ants        = 0;
    cur_defense_ants        = 0;
    cur_attack_ants         = 0;
    cur_static_defense_ants = 0;
    for( State::Ants::const_iterator it = m_state.myAnts().begin(); it != m_state.myAnts().end(); ++it )
    {
        switch ( (*it)->assignment )
        {
            case Ant::EXPLORE:         ++cur_explore_ants;         break;
            case Ant::DEFENSE:         ++cur_defense_ants;         break;
            case Ant::ATTACK:          ++cur_attack_ants;          break;
            case Ant::STATIC_DEFENSE:  ++cur_static_defense_ants;  break;
            default:                                               break;
        }
    }
    Debug::stream() << "New assigned ants:" << std::endl
                    << "    explore_ants  : " << cur_explore_ants << std::endl
                    << "    defense_ants  : " << cur_defense_ants << std::endl
                    << "    sdefense_ants : " << cur_static_defense_ants << std::endl
                    << "    attack_ants   : " << cur_attack_ants << std::endl;


#ifdef VISUALIZER
    setFillColor( 0, 255, 0, 0.2 );
    for( State::Ants::const_iterator it = m_state.myAnts().begin(); it != m_state.myAnts().end(); ++it )
    {
        Ant* ant = *it;
        if( ant->assignment == Ant::EXPLORE )
            circle( ant->location, 1, true );
    }

    setFillColor( 255, 0, 0, 0.2 );
    for( State::Ants::const_iterator it = m_state.myAnts().begin(); it != m_state.myAnts().end(); ++it )
    {
        Ant* ant = *it;
        if( ant->assignment == Ant::ATTACK )
            circle( ant->location, 1, true );
    }
    
    setFillColor( 0, 0, 255, 0.2 );
    for( State::Ants::const_iterator it = m_state.myAnts().begin(); it != m_state.myAnts().end(); ++it )
    {
        Ant* ant = *it;
        if( ant->assignment == Ant::DEFENSE )
            circle( ant->location, 1, true );
    }
    
    setFillColor( 255, 255, 0, 0.2 );
    for( State::Ants::const_iterator it = m_state.myAnts().begin(); it != m_state.myAnts().end(); ++it )
    {
        Ant* ant = *it;
        if( ant->assignment == Ant::STATIC_DEFENSE )
            circle( ant->location, 1, true );
    }
#endif
}


