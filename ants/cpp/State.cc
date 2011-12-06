
#include "Debug.h"
#include "State.h"

#include <algorithm>
#include <cmath>
#include <queue>

using namespace std;

State::State()
    : m_rows(0),
      m_cols(0),
      m_turn(0),
      m_turns(0),
      m_num_players(0),
      m_attack_radius(0),
      m_spawn_radius(0),
      m_view_radius(0),
      m_load_time(0),
      m_turn_time(0),
      m_game_over(0),
      m_seed(0)
{
}


State::~State()
{
}


void State::setup()
{
}


void State::reset()
{
    // 
    // Do not reset m_my_prev_ants
    //
    m_my_ants.clear();
    m_enemy_ants.clear();
    m_enemy_hills.clear();
    m_my_hills.clear();
    m_food.clear();
    m_map.reset();
}


void State::makeMove( Ant* ant, Direction direction )
{
    if( direction == NONE )
    {
        m_my_prev_ants[ ant->location ] = ant;
        Debug::stream() << " setting my_prev_ants[ " << ant->location << "] to " << *ant << std::endl;
        return;
    }

    cout << "o " << ant->location.row << " " << ant->location.col << " " << DIRECTION_CHAR[direction] << endl;
    m_map.makeMove( ant->location, direction );
    
    Location new_loc = m_map.getLocation( ant->location, direction );
    m_my_prev_ants[ new_loc ] = ant;
    ant->location = new_loc;
    
    Debug::stream() << " setting my_prev_ants[ " << new_loc << "] to " << *ant << std::endl;
}


void State::makeMove( Ant* ant, const Location& loc )
{
    Direction dir = m_map.getDirection( ant->location, loc );
    if( dir == NONE )
    {
        m_my_prev_ants[ ant->location ] = ant;
        Debug::stream() << " setting my_prev_ants[ " << ant->location << "] to " << *ant << std::endl;
        return;
    }

    cout << "o " << ant->location.row << " " << ant->location.col << " " << DIRECTION_CHAR[dir] << endl;
    m_map.makeMove( ant->location, loc );

    m_my_prev_ants[ loc ] = ant;
    ant->location = loc;
    
    Debug::stream() << " setting my_prev_ants[ " << loc << "] to " << *ant << std::endl;
}


void State::updateVisionInformation()
{
    // This function will update update the lastSeen value for any squares currently
    // visible by one of your live ants.

    // BE VERY CAREFUL IF YOU ARE GOING TO TRY AND MAKE THIS FUNCTION MORE EFFICIENT,
    // THE OBVIOUS WAY OF TRYING TO IMPROVE IT BREAKS USING THE EUCLIDEAN METRIC, FOR
    // A CORRECT MORE EFFICIENT IMPLEMENTATION, TAKE A LOOK AT THE GET_VISION FUNCTION
    // IN ANTS.PY ON THE CONTESTS GITHUB PAGE.
    
    std::queue<Location> locQueue;
    Location sLoc, cLoc, nLoc;

    for(int a=0; a<(int) m_my_ants.size(); a++)
    {
        sLoc = m_my_ants[a]->location;
        locQueue.push(sLoc);

        std::vector<std::vector<bool> > visited( m_rows, std::vector<bool>(m_cols, 0));
        m_map( sLoc ).setVisible();
        visited[sLoc.row][sLoc.col] = 1;

        while(!locQueue.empty())
        {
            cLoc = locQueue.front();
            locQueue.pop();

            for( int d = 0; d< NUM_DIRECTIONS; ++d )
            {
                nLoc = m_map.getLocation( cLoc, static_cast<Direction>( d ) );

                if(!visited[nLoc.row][nLoc.col] )
                {
                    if( m_map.distance(sLoc, nLoc) <= m_view_radius )
                    {
                        m_map( nLoc ).setVisible();
                        locQueue.push(nLoc);
                        m_frontier.erase( nLoc );
                    }
                    else
                    {
                        if( m_map( nLoc ).isUnknown() )
                        {
                            m_frontier.insert( nLoc );
                        }
                    }
                }
                visited[nLoc.row][nLoc.col] = 1;
            }
        }
    }
}


ostream& operator<<(ostream &os, const State &state)
{
    os << "Game state:\n"
       << "  Map dim     : " << state.m_rows << "x" << state.m_cols << std::endl
       << "  Max turns   : " << state.m_turns << std::endl
       << "  Num players : " << state.m_num_players << std::endl
       << "  Attack rad  : " << state.m_attack_radius << std::endl
       << "  Spawn rad   : " << state.m_spawn_radius << std::endl
       << "  View rad    : " << state.m_view_radius << std::endl
       << "  Load time   : " << state.m_load_time << std::endl
       << "  Turn time   : " << state.m_turn_time << std::endl;

    /*
    // Print out frontier map
    os << "============================" << std::endl;
    for( int i = 0; i < state.m_rows; ++i )
    {
        for( int j = 0; j < state.m_cols; ++j )
        {
            if( state.m_frontier.find( Location( i, j ) ) != state.m_frontier.end() )
                os << "* ";

            else
                os << "  ";
        }
        os << std::endl;
    }
    os << "============================" << std::endl;
    */
    return os;
}


istream& operator>>(istream &is, State &state)
{
    Location loc;
    int player;
    std::string inputType, junk;

    //finds out which turn it is
    while(is >> inputType)
    {
        if(inputType == "end")
        {
            state.m_game_over = 1;
            break;
        }
        else if(inputType == "turn")
        {
            is >> state.m_turn;
            break;
        }
        else //unknown line
            getline(is, junk);
    }

    if( state.m_turn == 0 )
    {
        //reads game parameters
        while(is >> inputType)
        {
            if(inputType == "loadtime")
                is >> state.m_load_time;
            else if(inputType == "turntime")
                is >> state.m_turn_time;
            else if(inputType == "rows")
                is >> state.m_rows;
            else if(inputType == "cols")
                is >> state.m_cols;
            else if(inputType == "turns")
                is >> state.m_turns;
            else if(inputType == "turns")
                is >> state.m_seed;
            else if(inputType == "player_seed")
                is >> state.m_seed;
            else if(inputType == "viewradius2")
            {
                is >> state.m_view_radius;
                state.m_view_radius = sqrtf(state.m_view_radius);
            }
            else if(inputType == "attackradius2")
            {
                is >> state.m_attack_radius;
                state.m_attack_radius = sqrtf(state.m_attack_radius);
            }
            else if(inputType == "spawnradius2")
            {
                is >> state.m_spawn_radius;
                state.m_spawn_radius = sqrtf(state.m_spawn_radius);
            }
            else if(inputType == "ready") //end of parameter input
            {
                state.timer().start();
                break;
            }
            else    //unknown line
                getline(is, junk);
        }
        state.m_map.resize( state.m_rows, state.m_cols );
    }
    else
    {
        //reads information about the current turn
        while(is >> inputType)
        {
            if(inputType == "w") //water square
            {
                is >> loc.row >> loc.col;
                state.m_map( loc ).type = Square::WATER;
            }
            else if(inputType == "f") //m_food square
            {
                is >> loc.row >> loc.col;
                state.m_map( loc ).food = true;
                state.m_food.push_back( loc );
            }
            else if(inputType == "a") //live ant square
            {
                is >> loc.row >> loc.col >> player;
                state.m_map( loc ).ant_id = player;
                if( player == 0 )
                {
                    Ant* ant = 0u;
                    State::AntHash::iterator prev_ant = state.m_my_prev_ants.find( loc );
                    if(  prev_ant == state.m_my_prev_ants.end() )
                    {
                        Debug::stream() << " CREATING NEW ANT!!! at " << loc << std::endl;
                        ant = new Ant( loc );
                    }
                    else
                    {
                        assert( prev_ant->second->location == loc );
                        ant = prev_ant->second; 
                        state.m_my_prev_ants.erase( prev_ant );
                    }
                    state.m_my_ants.push_back( ant );
                    state.m_map( loc ).ant =  ant;
                    Debug::stream() << "  my_ants[" << state.m_my_ants.size()-1 << "] set to " << *ant << std::endl;
                }
                else
                {
                    state.m_enemy_ants.push_back( loc ); 
                }

            }
            else if(inputType == "d") //dead ant square
            {
                is >> loc.row >> loc.col >> player;
                state.m_map( loc ).deadAnts.push_back(player);
            }
            else if(inputType == "h")
            {
                is >> loc.row >> loc.col >> player;
                state.m_map( loc ).hill_id = player;
                if(player == 0)
                    state.m_my_hills.push_back( loc );
                else
                    state.m_enemy_hills.push_back( loc );
            }
            else if(inputType == "players") //player information
                is >> state.m_num_players;
            else if(inputType == "scores") //score information
            {
                state.m_scores.resize( state.m_num_players );
                for( int p=0; p<state.m_num_players; p++ )
                    is >> state.m_scores[p];
            }
            else if(inputType == "go") //end of turn input
            {
                if( state.m_game_over )
                    is.setstate(std::ios::failbit);
                else
                    state.timer().start();
                break;
            }
            else //unknown line
                getline(is, junk);
        }
    }

    // Clear out previous ants list since any ants left are dead :(
    for( State::AntHash::iterator it = state.m_my_prev_ants.begin(); it != state.m_my_prev_ants.end(); ++it )
    {
        Debug::stream() << "deleting ant " << it->first << " -- " << it->second << std::endl;
        delete it->second;
    }
    state.m_my_prev_ants.clear();

    return is;
};
