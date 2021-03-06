
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
#include <cmath>

//
// Helpers
//

namespace 
{
    typedef std::vector<Location> Locations;
    typedef std::set<Location>    LocationSet;
    typedef std::vector<Ant*>     Ants;
    typedef std::set<Ant*>        AntSet;
    typedef std::map<Location, Ant*> AssignedAnts;

    
    void printAssignedAnts( const AssignedAnts& assigned_ants )
    {
        Debug::stream() << "************** assigned ants " << std::endl;
        for( AssignedAnts::const_iterator it = assigned_ants.begin(); it != assigned_ants.end(); ++it )
        {
            Debug::stream() << "  " << it->first << " " << *(it->second) << std::endl;
        }
        Debug::stream() << "*************** " << std::endl;
    }




    struct FindEnemyAnts
    {
        FindEnemyAnts( std::vector<Location>& enemies ) : enemies( enemies ), found_enemy( false ) {}

        bool operator()( const BFNode* node )
        {
            if( node->square->ant_id > 0 )
            {
                found_enemy = true;
                enemies.push_back( node->loc );
            }
            return true;
        }

        std::vector<Location>& enemies;
        bool found_enemy;
    };


    struct HillAttackAnts
    {
        bool operator()( const BFNode* node )
        {
            if( node->square->ant_id == 0 &&
                node->square->ant->path.goal() != Path::HILL &&
                node->square->ant->path.goal() != Path::FOOD &&
                node->child->square->isAvailable() )
            {
                Ant* cur_ant = node->square->ant;
                node->getRPath( cur_ant->path );
                cur_ant->path.setGoal( Path::HILL );
                node->child->square->assigned = true;
            }
            return true;
        }
    };


    struct FindFoodAnt
    {
        FindFoodAnt( AssignedAnts& food_ants, bool allow_overrides, Ant* already_assigned ) 
            : food_ants( food_ants ),
              allow_overrides( allow_overrides ),
              already_assigned( already_assigned ),
              ant( 0u )
        {
        }

        bool operator()( const BFNode* node )
        {
            if( node->square->ant_id == 0 )
            {
                Ant* cur_ant = node->square->ant;
                if( ( cur_ant->assignment  != Ant::STATIC_DEFENSE ) &&
                    ( cur_ant->path.goal() != Path::ATTACK || allow_overrides ) &&
                    ( !already_assigned                    || already_assigned->path.size() > node->depth ) && 
                    ( cur_ant->path.goal() != Path::FOOD   || cur_ant->path.size() > node->depth ) &&
                      node->child->square->isAvailable() )
                {
                    //Debug::stream() << " prev ant: " << 
                    Debug::stream() << " Assigning ant " << *cur_ant << " to food:" << std::endl;
                    Debug::stream() << "      already assigned: ";
                    if( already_assigned )
                        Debug::stream() << (*already_assigned) << std::endl;
                    else
                        Debug::stream() << 0 << std::endl;
                    Debug::stream() << "      before: " << std::endl;
                    printAssignedAnts( food_ants );
                    if( cur_ant->path.goal() == Path::FOOD )
                    {
                        AssignedAnts::iterator it = food_ants.find( cur_ant->path.destination() );
                        assert( it != food_ants.end() );
                        it->second->path.reset();
                        food_ants.erase( it );
                    }

                    node->getRPath( cur_ant->path );
                    cur_ant->path.setGoal( Path::FOOD );
                    node->child->square->assigned = true;

                    // If an ant is already assigned to this food, free it up
                    if( already_assigned )
                    {
                        already_assigned->path.reset();
                        food_ants.erase( cur_ant->path.destination() );
                    }

                    ant = cur_ant;
                    food_ants.insert( std::make_pair( cur_ant->path.destination(), cur_ant ) );
                    Debug::stream() << " Assigning ant to food: after: " << std::endl;
                    printAssignedAnts( food_ants );
                    return false;
                }
            }
            return true;
        }

        AssignedAnts& food_ants;
        bool          allow_overrides;
        Ant*          already_assigned;
        Ant*          ant;
    };


    struct AttackAnts
    {
        AttackAnts( AntSet& assigned, int num_ants )  
            : assigned( assigned ), num_ants( num_ants ), ants_found( 0 ) {}

        bool operator()( const BFNode* node )
        {
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

    struct NotHill 
    {
        NotHill( const Map& map ) : map( map ) {}

        bool operator()( const BFNode* current, const Location& location, const Square& neighbor )
        {
            return map( current->loc ).hill_id != 0;
        }

        const Map& map;
    };

    struct NotHillOrCombat
    {
        NotHillOrCombat( const Map& map ) : map( map ) {}

        bool operator()( const BFNode* current, const Location& location, const Square& neighbor )
        {
            return map( current->loc ).hill_id != 0 && !neighbor.in_enemy_range;
        }

        const Map& map;
    };

    struct Available 
    {
        Available( const Map& map ) : map( map ) {}

        bool operator()( const BFNode* current, const Location& location, const Square& neighbor )
        {
            // We dont want any non-available nodes internal to our path.
            return  map( current->loc ).isAvailable();
        }

        const Map& map;
    };


    struct HillDefensePriority
    {
        HillDefensePriority( Map& map ) : map( map ) {}

        bool operator()( const BFNode* node )
        {
          /*
            const float peak_priority = 1.0f;
            const float max_distance  = 40;
            const float peak_distance = 4.0f;
            float distance = static_cast<float>( node->depth );
            float priority = map.getPriority( Map::DEFENSE, node->loc );
            float hill_priority = distance < peak_distance ?
                                  peak_priority - 0.5f * (peak_distance-distance) / peak_distance :
                                  peak_priority - (distance-peak_distance) / ( max_distance - peak_distance );
          */

            const float peak_priority = 1.0f;
            const float max_distance  = 40.0f;
            float distance = static_cast<float>( node->depth );
            float priority = map.getPriority( Map::DEFENSE, node->loc );
            float hill_priority = peak_priority - sqrtf( distance / max_distance );

            map.setPriority( Map::DEFENSE, node->loc, priority + hill_priority );
            return true;
        }

        Map& map;
    };

    void addHillDefensePriority( Map& map, const Location& location )
    {
        HillDefensePriority hill_defense_prority( map );
        Always              always;
        BF<HillDefensePriority, Always> add_hill_defense_priority( map, location, hill_defense_prority, always );
        add_hill_defense_priority.setMaxDepth( 40 );
        add_hill_defense_priority.traverse();
    }


    void splatEnemyCombatRange( const Location& location, Map& map )
    {
        const unsigned height = map.height();
        const unsigned width  = map.width();

        Location x = clamp( Location( location.row-2, location.col-1 ), height, width );
        map( x ).in_enemy_range = true; 
        x = clamp( Location( location.row-2, location.col-0 ), height, width );
        map( x ).in_enemy_range = true; 
        x = clamp( Location( location.row-2, location.col+1 ), height, width );
        map( x ).in_enemy_range = true; 

        x = clamp( Location( location.row-1, location.col-2 ), height, width );
        map( x ).in_enemy_range = true; 
        x = clamp( Location( location.row-1, location.col-1 ), height, width );
        map( x ).in_enemy_range = true; 
        x = clamp( Location( location.row-1, location.col-0 ), height, width );
        map( x ).in_enemy_range = true; 
        x = clamp( Location( location.row-1, location.col+1 ), height, width );
        map( x ).in_enemy_range = true; 
        x = clamp( Location( location.row-1, location.col+2 ), height, width );
        map( x ).in_enemy_range = true; 

        x = clamp( Location( location.row-0, location.col-2 ), height, width );
        map( x ).in_enemy_range = true; 
        x = clamp( Location( location.row-0, location.col-1 ), height, width );
        map( x ).in_enemy_range = true; 
        x = clamp( Location( location.row-0, location.col-0 ), height, width );
        map( x ).in_enemy_range = true; 
        x = clamp( Location( location.row-0, location.col+1 ), height, width );
        map( x ).in_enemy_range = true; 
        x = clamp( Location( location.row-0, location.col+2 ), height, width );
        map( x ).in_enemy_range = true; 

        x = clamp( Location( location.row+1, location.col-2 ), height, width );
        map( x ).in_enemy_range = true; 
        x = clamp( Location( location.row+1, location.col-1 ), height, width );
        map( x ).in_enemy_range = true; 
        x = clamp( Location( location.row+1, location.col-0 ), height, width );
        map( x ).in_enemy_range = true; 
        x = clamp( Location( location.row+1, location.col+1 ), height, width );
        map( x ).in_enemy_range = true; 
        x = clamp( Location( location.row+1, location.col+2 ), height, width );
        map( x ).in_enemy_range = true; 

        x = clamp( Location( location.row+2, location.col-1 ), height, width );
        map( x ).in_enemy_range = true; 
        x = clamp( Location( location.row+2, location.col-0 ), height, width );
        map( x ).in_enemy_range = true; 
        x = clamp( Location( location.row+2, location.col+1 ), height, width );
        map( x ).in_enemy_range = true; 

        // The four move locations
        Location x_n = clamp( Location( location.row-1, location.col+0 ), height, width );
        Location x_s = clamp( Location( location.row+1, location.col+0 ), height, width );
        Location x_e = clamp( Location( location.row+0, location.col+1 ), height, width );
        Location x_w = clamp( Location( location.row+0, location.col-1 ), height, width );

        if( !isWaterOrFood( map( x_n ) ) )
        {
            Location x = clamp( Location( location.row-3, location.col-1 ), height, width );
            map( x ).in_enemy_range = true; 
            x = clamp( Location( location.row-3, location.col-0 ), height, width );
            map( x ).in_enemy_range = true; 
            x = clamp( Location( location.row-3, location.col+1 ), height, width );
            map( x ).in_enemy_range = true; 
        }

        if( !isWaterOrFood( map( x_s ) ) )
        {
            Location x = clamp( Location( location.row+3, location.col-1 ), height, width );
            map( x ).in_enemy_range = true; 
            x = clamp( Location( location.row+3, location.col-0 ), height, width );
            map( x ).in_enemy_range = true; 
            x = clamp( Location( location.row+3, location.col+1 ), height, width );
            map( x ).in_enemy_range = true; 
        }

        if( !isWaterOrFood( map( x_w ) ) )
        {
            Location x = clamp( Location( location.row-1, location.col-3 ), height, width );
            map( x ).in_enemy_range = true; 
            x = clamp( Location( location.row+0, location.col-3 ), height, width );
            map( x ).in_enemy_range = true; 
            x = clamp( Location( location.row+1, location.col-3 ), height, width );
            map( x ).in_enemy_range = true; 
        }

        if( !isWaterOrFood( map( x_e ) ) )
        {
            Location x = clamp( Location( location.row-1, location.col+3 ), height, width );
            map( x ).in_enemy_range = true; 
            x = clamp( Location( location.row+0, location.col+3 ), height, width );
            map( x ).in_enemy_range = true; 
            x = clamp( Location( location.row+1, location.col+3 ), height, width );
            map( x ).in_enemy_range = true; 
        }

        if( !isWaterOrFood( map( x_e ) ) || !isWaterOrFood( map( x_n ) ) )
        {
            Location x = clamp( Location( location.row+2, location.col+2 ), height, width );
            map( x ).in_enemy_range = true; 
        }

        if( !isWaterOrFood( map( x_w ) ) || !isWaterOrFood( map( x_n ) ) )
        {
            Location x = clamp( Location( location.row-2, location.col+2 ), height, width );
            map( x ).in_enemy_range = true; 
        }

        if( !isWaterOrFood( map( x_w ) ) || !isWaterOrFood( map( x_s ) ) )
        {
            Location x = clamp( Location( location.row-2, location.col-2 ), height, width );
            map( x ).in_enemy_range = true; 
        }

        if( !isWaterOrFood( map( x_e ) ) || !isWaterOrFood( map( x_s ) ) )
        {
            Location x = clamp( Location( location.row+2, location.col-2 ), height, width );
            map( x ).in_enemy_range = true; 
        }
    }

    typedef  BF<FindFoodAnt, NotHillOrCombat> FindNearestFoodAnt;
    
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
    m_battle = new Battle( m_state.map(), m_food_ants );

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
    Debug::stream() << " state: " << m_state << std::endl;

    for( Locations::const_iterator it = m_state.enemyAnts().begin(); it != m_state.enemyAnts().end(); ++it )
    {
        splatEnemyCombatRange( *it, m_state.map() );
    }

    //
    // Check for validity of pre-existing paths
    //
    Debug::stream() << " Processing ants: " << std::endl;
    for( Ants::iterator it = m_state.myAnts().begin(); it != m_state.myAnts().end(); ++it )
    {
        Debug::stream() << "    " << **it << std::endl;
        checkValidPath( *it );
    }

    // Find all hills which are under attack 
    std::vector<Location> base_attackers;
    for( State::Locations::const_iterator it = m_state.myHills().begin(); it != m_state.myHills().end(); ++it )
    {
        FindEnemyAnts find_enemy_ants( base_attackers );
        Always always;
        BF<FindEnemyAnts, Always> bfs( m_state.map(), *it, find_enemy_ants, always );
        bfs.setMaxDepth( 20 );
        bfs.traverse();

        if( find_enemy_ants.found_enemy )
        {
            // TODO: store number of enemy ants seen at each hill
            Debug::stream() << " Rallying to base at " << *it << std::endl;
            m_hills_under_attack.insert( *it );
        }
    }

    //
    // Assign ants to jobs (explore, attack, defend)
    //
    makeAssignments();
    
    
    //
    // Attack hills directly for close by ants
    //
    Debug::stream() << " Assigning hill-attack tasks... " << std::endl;
    assignToHillAttack( 32 );

    //
    // Assign ants to very nearby food with high priority
    //
    Debug::stream() << " Assigning short distance food tasks... " << std::endl;
    assignToFood( 4, true );
    
    //
    // Assign ants to farther food with lower priority
    //
    Debug::stream() << " Assigning long distance food tasks... " << std::endl;
    assignToFood( 20, false );

    //
    // Set up enemy_hill distance attack map
    //
    for( LocationSet::iterator it = m_enemy_hills.begin(); it != m_enemy_hills.end(); ++it )
        m_state.map().setDistanceTarget( Map::ATTACK, *it, 0 );
    m_state.map().computeDistanceMap( Map::ATTACK );
    
    //
    // Set up attacked ally_hill defense map
    // TODO: rally more intelligently, between enemies and base. also, count
    //       number of enemies and respond proportionately
    //
    for( Locations::iterator it = base_attackers.begin(); it != base_attackers.end(); ++it )
        m_state.map().setPriority( Map::DEFENSE, *it, 100 );
    m_state.map().diffusePriority( Map::DEFENSE, 12 );
    
    for( LocationSet::iterator it = m_hills_under_attack.begin(); it != m_hills_under_attack.end(); ++it )
    {
        addHillDefensePriority( m_state.map(), *it );
    }
    

    //
    // Update map for explorations 
    // TODO: persistant notvisible info so squares which have been non-visible
    //       for longer are higher priority
    //
    m_state.map().updatePriority( Map::EXPLORE, 100, notVisible );
    for( State::LocationSet::const_iterator it = m_state.frontier().begin(); it != m_state.frontier().end(); ++it )
    {
        std::vector<Location> neighbors;
        m_state.map().getNeighbors( *it, isLand, neighbors );
        for( std::vector<Location>::iterator it = neighbors.begin(); it != neighbors.end(); ++it )
        {
            m_state.map().setPriority( Map::EXPLORE, *it, 1000 );
        }
    }

    for( State::Ants::const_iterator it = m_state.myAnts().begin(); it != m_state.myAnts().end(); ++it )
        m_state.map().setPriority( Map::EXPLORE, (*it)->location, -100 );
    
    Debug::stream() << "Before moves " << std::endl << m_state.map() << std::endl;

    int diffusion_steps = std::max( m_state.rows(), m_state.cols() );
    m_state.map().diffusePriority( Map::EXPLORE, diffusion_steps );


    
    //
    // Any ants without explicit path will follow map priorities
    //
    std::for_each( m_state.myAnts().begin(),
                   m_state.myAnts().end(),
                   std::bind1st( std::mem_fun( &Bot::assignToMapPath ), this ) );

    //
    // Visualize ant paths for debugging
    //
    for( State::Ants::const_iterator it = m_state.myAnts().begin(); it != m_state.myAnts().end(); ++it )
    {
        (*it)->path.visualize( (*it)->location, m_state.map() );
    }

    //
    // Assign ants to attack/defend locally.  Will override path if necessary
    //
    Debug::stream() << " Assigning battle tasks..." << std::endl;
    m_battle->solve( m_state.myAnts(), m_state.enemyAnts() );

    std::for_each( m_state.myAnts().begin(),
                   m_state.myAnts().end(),
                   std::bind1st( std::mem_fun( &Bot::makeUncheckedMove), this ) );
    
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


void Bot::assignToHillAttack( unsigned max_dist )
{
    for( LocationSet::iterator it = m_enemy_hills.begin(); it != m_enemy_hills.end(); ++it )
    {
        Debug::stream() << " Searching for nearby ants to attack hill: " << *it << std::endl;

        HillAttackAnts hill_attack_ants;
        //Always         always;
        //BF<HillAttackAnts, Always> find_hill_attack_ants( m_state.map(), *it, hill_attack_ants, always );
        Available      available( m_state.map() );
        BF<HillAttackAnts, Available> find_hill_attack_ants( m_state.map(), *it, hill_attack_ants, available );
        find_hill_attack_ants.setMaxDepth( max_dist );
        find_hill_attack_ants.traverse();
    }
}


void Bot::assignToFood( unsigned max_dist, bool allow_overrides )
{
    Debug::stream() << " food ants at start of food assignment:" << std::endl;
    printAssignedAnts( m_food_ants );

    for( State::Locations::const_iterator it = m_state.food().begin(); it != m_state.food().end(); ++it )
    {
        Debug::stream() << " Searching for ant to collect food: " << *it << std::endl;

        AssignedAnts::iterator prev  = m_food_ants.find( *it );
        Ant* previous_ant = ( prev != m_food_ants.end() ) ? prev->second : 0u;

        FindFoodAnt        find_ant( m_food_ants, allow_overrides, previous_ant );
        NotHillOrCombat    not_hill_or_combat( m_state.map() );
        FindNearestFoodAnt find_nearest_ant( m_state.map(), *it, find_ant, not_hill_or_combat );
        find_nearest_ant.setMaxDepth( max_dist );
        find_nearest_ant.traverse();

        if( find_ant.ant )
            Debug::stream() << "    assigning ant " << *(find_ant.ant) << std::endl;
        else
            Debug::stream() << "    no path found" << std::endl;
    }
}


void Bot::assignToMapPath( Ant* ant )
{
    const Location cur_location = ant->location;
    Debug::stream() << "assignToMapPath( " << *ant << "  )" << std::endl;

    //
    // If we have a valid path, use it
    //
    if( !ant->path.empty() )
    {
        Direction dir = ant->path.nextStep();
        Debug::stream() << "  Yes valid goal based path moving " << ant->location << ": " << DIRECTION_CHAR[dir]
                        << std::endl;
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

        if( ant->assignment == Ant::EXPLORE || ant->assignment == Ant::DEFENSE )
        {
            Map::PriorityType priority_type = static_cast<Map::PriorityType>( ant->assignment );

            float max_priority = m_state.map().getPriority( priority_type, move_loc );

            Debug::stream() << "      starting " << Map::priorityTypeString( priority_type )
                            << " priority " << max_priority << " to " << move_loc << std::endl;
            for( std::vector<Location>::iterator it = neighbors.begin()+1; it != neighbors.end(); ++it )
            {
                Location neighbor_loc = *it;
                float neighbor_priority = m_state.map().getPriority( priority_type, neighbor_loc );
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
            float min_distance = m_state.map().getPriority( Map::ATTACK, move_loc );
            Debug::stream() << "      starting ATTACK distance " << min_distance << " to " << move_loc << std::endl;
            for( std::vector<Location>::iterator it = neighbors.begin()+1; it != neighbors.end(); ++it )
            {
                Location neighbor_loc = *it;
                float neighbor_distance = m_state.map().getPriority( Map::ATTACK, neighbor_loc );
                Debug::stream() << "      checking distance " << neighbor_distance
                                << " to " << neighbor_loc << std::endl;
                if( neighbor_distance < min_distance )
                {
                    min_distance = neighbor_distance;
                    move_loc     = neighbor_loc;
                }
            }

            Debug::stream() << "    moving " << ant->location << " to " << move_loc << " dist " << min_distance 
                            << std::endl;
        }

        m_state.map()( move_loc ).assigned = true; 
        ant->path.assign( move_loc, m_state.map().getDirection( cur_location, move_loc ) );
    }
}


void Bot::makeUncheckedMove( Ant* ant )
{
    // Presumes this ant's next move is valid!!!
    Direction dir = ant->path.popNextStep();
    Debug::stream() << "  unchecked  moving " << ant->location << ": " << DIRECTION_CHAR[dir] << std::endl;
    m_state.makeMove( ant, dir );
}


/*
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
        Debug::stream() << "  ********************************************* " << std::endl
                        << "  WARNING: NO valid goal based path: " << *ant    << std::endl
                        << "  ********************************************* " << std::endl;
        
        // Leave static defenders alone
        if( ant->assignment == Ant::STATIC_DEFENSE )
            return;

        Debug::stream() << "  No valid goal based path - following weights" << std::endl;
        std::vector<Location> neighbors;
        m_state.map().getNeighbors( cur_location, isAvailable, neighbors );
        if( neighbors.empty() ) return;

        Debug::stream() << "    checking " << neighbors.size() << "neighbors" << std::endl;
        Location move_loc  = *( neighbors.begin() );

        if( ant->assignment == Ant::EXPLORE || ant->assignment == Ant::DEFENSE )
        {
            Map::PriorityType priority_type = static_cast<Map::PriorityType>( ant->assignment );

            float max_priority = m_state.map().getPriority( priority_type, move_loc );

            Debug::stream() << "      starting " << Map::priorityTypeString( priority_type )
                            << " priority " << max_priority << " to " << move_loc << std::endl;
            for( std::vector<Location>::iterator it = neighbors.begin()+1; it != neighbors.end(); ++it )
            {
                Location neighbor_loc = *it;
                float neighbor_priority = m_state.map().getPriority( priority_type, neighbor_loc );
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
            float min_distance = m_state.map().getPriority( Map::ATTACK, move_loc );
            Debug::stream() << "      starting ATTACK distance " << min_distance << " to " << move_loc << std::endl;
            for( std::vector<Location>::iterator it = neighbors.begin()+1; it != neighbors.end(); ++it )
            {
                Location neighbor_loc = *it;
                float neighbor_distance = m_state.map().getPriority( Map::ATTACK, neighbor_loc );
                Debug::stream() << "      checking distance " << neighbor_distance
                                << " to " << neighbor_loc << std::endl;
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

*/



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
    // Remove ants which have been killed
    // TODO: n^2!
    std::vector< AssignedAnts::iterator > remove_these;
    for( AssignedAnts::iterator it = m_food_ants.begin(); it != m_food_ants.end(); ++it )
        if( std::find( m_state.myAnts().begin(), m_state.myAnts().end(), it->second ) == m_state.myAnts().end() )
            remove_these.push_back( it );
    
    for( std::vector< AssignedAnts::iterator >::iterator it = remove_these.begin(); it != remove_these.end(); ++it )
        m_food_ants.erase( *it );

    // Remove ants whose food has disappeared
    remove_these.clear();
    for( AssignedAnts::iterator it = m_food_ants.begin(); it != m_food_ants.end(); ++it )
        if( m_state.map()( it->first ).food == false && m_state.map()( it->first ).visible )
            remove_these.push_back( it );

    for( std::vector< AssignedAnts::iterator >::iterator it = remove_these.begin(); it != remove_these.end(); ++it )
    {
        (*it)->second->path.reset();
        m_food_ants.erase( *it );
    }

}


bool Bot::checkValidPath( Ant* ant )
{
    // Checks if this ant has a valid path and resets the path if not
    Debug::stream() << " Checking validity of path " << ant->path << std::endl;
    if( ant->path.empty() ) 
        return false;

    if( ant->path.nextStep() == NONE ) 
        return true;

    Map& map = m_state.map();
    Location next_loc = map.getLocation( ant->location, ant->path.nextStep() );
    if( !map( next_loc ).isAvailable() ) 
    {
        Debug::stream() << "     resetting path: next sq not avail" << std::endl;
        if( ant->path.goal() == Path::FOOD )
        {
            AssignedAnts::iterator it = m_food_ants.find( ant->path.destination() );
            assert( it != m_food_ants.end() );
            it->second->path.reset();
            m_food_ants.erase( it );
        }
        ant->path.reset();
        return false;
    }

    Location goal_loc = ant->path.destination();
    if( ant->path.goal() == Path::FOOD &&
      ( !hasFood( map( goal_loc ) ) || map( next_loc ).in_enemy_range ) )
    {
        Debug::stream() << "     resetting path: food gone or next step in combat range" << std::endl;
        AssignedAnts::iterator it = m_food_ants.find( ant->path.destination() );
        assert( it != m_food_ants.end() );
        it->second->path.reset();
        m_food_ants.erase( it );
        return false;
    } 

    if ( ant->path.goal() == Path::HILL && m_enemy_hills.find( goal_loc ) == m_enemy_hills.end() )
    {
        Debug::stream() << "     resetting path: hill gone" << std::endl;
        Debug::stream() << "        " << map( goal_loc ) << std::endl; 

        ant->path.reset();
        return false;
    }

    map( next_loc ).assigned = true; 
    return true;
}


void Bot::findStaticAnt( const Location& hill, const Location& defense_position )
{
    Map& map = m_state.map();

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
            map( defense_position ).assigned = true;
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
            map( loc ).assigned = true;
            ant->path.assign( defense_position, dirs.begin(), dirs.end() );
            return;
        }

        loc = map.getLocation( hill, dir1 );
        if( map( loc ).isAvailable() )
        {
            ant->assignment = Ant::STATIC_DEFENSE;
            std::vector<Direction> dirs;
            dirs.push_back( dir1 );
            dirs.push_back( dir0 );
            map( loc ).assigned = true;
            ant->path.assign( defense_position, dirs.begin(), dirs.end() );
            return;
        }
    }
}


void Bot::makeAssignments()
{
    int num_ants = m_state.myAnts().size();
    int static_ants_per_base = 0;
    if( !m_state.myHills().empty() )
    {
        if( m_state.myHills().size() > 1 )
            static_ants_per_base = num_ants >= 70 ? 4 :
                                   num_ants >= 64 ? 3 :
                                   num_ants >= 48 ? 2 :
                                   num_ants >= 32 ? 1 :
                                   0;
         else
            static_ants_per_base = num_ants >= 64 ? 4 :
                                   num_ants >= 48 ? 3 :
                                   num_ants >= 32 ? 2 :
                                   num_ants >= 16 ? 1 :
                                   0;
    }

    int max_explore_ants = ( m_state.rows() / 16 ) * ( m_state.cols() / 16 );
    int max_defense_ants = m_hills_under_attack.size()  * 16; 
    int explore_ants = std::min( max_explore_ants, num_ants / 2 );
    int defense_ants = std::min( max_defense_ants, num_ants / 4 );
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
    // TODO: - better distribution of explore ants chosen.
    //       - can explore algoriithm punish cul-de-sacs?
    //
    Debug::stream() << "Assigning ants:" << std::endl
                    << "    num_hills     : " << m_state.myHills().size() << std::endl
                    << "    total_ants    : " << num_ants << std::endl
                    << "    enemy_hills   : " << m_enemy_hills.size() << std::endl
                    << "    attacked_hills: " << m_hills_under_attack.size() << std::endl
                    << "    explore_ants  : " << explore_ants << std::endl
                    << "    defense_ants  : " << defense_ants << std::endl
                    << "    sdefense_ants : " << static_ants_per_base << std::endl
                    << "    attack_ants   : " << attack_ants << std::endl;
    
    Debug::stream() << "Previous ants:" << std::endl
                    << "    explore_ants  : " << cur_explore_ants << std::endl
                    << "    defense_ants  : " << cur_defense_ants << std::endl
                    << "    sdefense_ants : " << cur_static_defense_ants << std::endl
                    << "    attack_ants   : " << cur_attack_ants << std::endl;
    
    // Static defense assignments first
    if( static_ants_per_base > 0 )
    {
        for( State::Locations::const_iterator it = m_state.myHills().begin(); it != m_state.myHills().end(); ++it )
        {
            const Location hill_east = m_state.map().getLocation( *it, EAST );
            const Location defender0 = m_state.map().getLocation( hill_east, NORTH );
            findStaticAnt( *it, defender0 );

            if( static_ants_per_base <= 1 ) continue;
            
            const Location defender1 = m_state.map().getLocation( hill_east, SOUTH );
            findStaticAnt( *it, defender1 );
            
            if( static_ants_per_base <= 2 ) continue;

            const Location hill_west = m_state.map().getLocation( *it, WEST );
            const Location defender2 = m_state.map().getLocation( hill_west, NORTH );
            findStaticAnt( *it, defender2 );
            
            if( static_ants_per_base <= 3 ) continue;

            Location defender3 = m_state.map().getLocation( hill_west, SOUTH );
            findStaticAnt( *it, defender3 );
        }
    }

    // Now dynamic defense
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

    // Hill attackers
    AntSet assigned_attack;
    bool attack_ants_updated = m_enemy_hills_changed;
    if( !m_enemy_hills.empty() && ( m_enemy_hills_changed || cur_attack_ants < 2*attack_ants/3 ) )
    {
        attack_ants_updated = true;
        Debug::stream() << " m_enemy_hills_changed  " << m_enemy_hills_changed << std::endl;

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

    // Exploration
    for( State::Ants::const_iterator it = m_state.myAnts().begin(); it != m_state.myAnts().end(); ++it )
    {
        Ant* ant = *it;

        // Ignore defense ants
        if( ant->assignment == Ant::STATIC_DEFENSE || assigned_defense.find( ant ) != assigned_defense.end() )
            continue; 

        // Only update attack ants if enemy_hills_changed or we had a reassignment of attack ants
        if( ant->assignment == Ant::ATTACK && 
            ( !attack_ants_updated || assigned_attack.find( ant ) != assigned_attack.end() ) )
            continue;
        
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


    /*
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
*/
}


