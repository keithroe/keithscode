#include "State.h"

using namespace std;

//constructor
State::State()
    : m_game_over( 0 ),
      m_turn( 0 ),
      m_debug( "./debug.txt" )
{
};

//deconstructor
State::~State()
{
    m_debug.close();
    
    for( int i = 0; i < m_rows; ++i )
        delete [] m_grid[i];
    delete m_grid;
    m_grid = 0u;
};

//sets the state up
void State::setup()
{
    m_grid = new Square*[ m_rows ];
    for( int i = 0; i < m_rows; ++i )
        m_grid[ i ] = new Square[ m_cols ];
};

//resets all non-water squares to land and clears the bots ant vector
void State::reset()
{
    m_my_ants.clear();
    m_enemy_ants.clear();
    m_my_hills.clear();
    m_enemy_hills.clear();
    m_food.clear();
    for( int row=0; row < m_rows; row++ )
        for( int col=0; col<m_cols; col++ )
            m_grid[ row ][ col ].reset();
};

//outputs move information to the engine
void State::makeMove(const Location &loc, int direction)
{
    cout << "o " << loc.row << " " << loc.col << " " << CDIRECTIONS[direction] << endl;

    Location nLoc = getLocation(loc, direction);
    m_grid[nLoc.row][nLoc.col].ant = m_grid[loc.row][loc.col].ant;
    m_grid[loc.row][loc.col].ant = -1;
};

//returns the euclidean distance between two locations with the edges wrapped
double State::getDistance(const Location &loc1, const Location &loc2)const
{
    int d1 = abs(loc1.row-loc2.row),
        d2 = abs(loc1.col-loc2.col),
        dr = min(d1, m_rows-d1),
        dc = min(d2, m_cols-d2);
    return sqrt(dr*dr + dc*dc);
};

//returns the new location from moving in a given direction with the edges wrapped
Location State::getLocation(const Location &loc, int direction)const
{
    return Location( (loc.row + DIRECTIONS[direction][0] + m_rows) % m_rows,
                     (loc.col + DIRECTIONS[direction][1] + m_cols) % m_cols );
};

/*
    This function will update update the lastSeen value for any squares currently
    visible by one of your live ants.

    BE VERY CAREFUL IF YOU ARE GOING TO TRY AND MAKE THIS FUNCTION MORE EFFICIENT,
    THE OBVIOUS WAY OF TRYING TO IMPROVE IT BREAKS USING THE EUCLIDEAN METRIC, FOR
    A CORRECT MORE EFFICIENT IMPLEMENTATION, TAKE A LOOK AT THE GET_VISION FUNCTION
    IN ANTS.PY ON THE CONTESTS GITHUB PAGE.
*/
void State::updateVisionInformation()
{
    std::queue<Location> locQueue;
    Location sLoc, cLoc, nLoc;

    for(int a=0; a<(int) m_my_ants.size(); a++)
    {
        sLoc = m_my_ants[a];
        locQueue.push(sLoc);

        std::vector<std::vector<bool> > visited(m_rows, std::vector<bool>(m_cols, 0));
        m_grid[sLoc.row][sLoc.col].isVisible = 1;
        visited[sLoc.row][sLoc.col] = 1;

        while(!locQueue.empty())
        {
            cLoc = locQueue.front();
            locQueue.pop();

            for(int d=0; d<TDIRECTIONS; d++)
            {
                nLoc = getLocation(cLoc, d);

                if(!visited[nLoc.row][nLoc.col] && getDistance(sLoc, nLoc) <= m_view_radius)
                {
                    m_grid[nLoc.row][nLoc.col].isVisible = 1;
                    locQueue.push(nLoc);
                }
                visited[nLoc.row][nLoc.col] = 1;
            }
        }
    }
};

/*
    This is the output function for a state. It will add a char map
    representation of the state to the output stream passed to it.

    For example, you might call "cout << state << endl;"
*/
ostream& operator<<(ostream &os, const State &state)
{
    for(int row=0; row<state.m_rows; row++)
    {
        for(int col=0; col<state.m_cols; col++)
        {
            const Square& square = state.grid()[row][col];
            os << ' ';
            switch( square.content )
            {
                case Square::WATER:
                {
                    os << 'w';
                    break;
                }
                case Square::FOOD:
                {
                    os << 'f';
                    break;
                }
                case Square::HILL:
                {
                    os << static_cast<char>( 'A' + square.hill );
                    break;
                }
                case Square::EMPTY:
                {
                    if( square.ant >= 0 )
                    {
                        os << static_cast<char>( 'a' + state.grid()[row][col].ant );
                        break;
                    }

                    os << (square.isVisible ? ' ' : '?');
                    break;
                }

            }
        }
        os << endl;
    }

    return os;
};

//input function
istream& operator>>(istream &is, State &state)
{
    int row, col, player;
    string inputType, junk;

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
            else if(inputType == "viewradius2")
            {
                is >> state.m_view_radius;
                state.m_view_radius = sqrt(state.m_view_radius);
            }
            else if(inputType == "attackradius2")
            {
                is >> state.m_attack_radius;
                state.m_attack_radius = sqrt(state.m_attack_radius);
            }
            else if(inputType == "spawnradius2")
            {
                is >> state.m_spawn_radius;
                state.m_spawn_radius = sqrt(state.m_spawn_radius);
            }
            else if(inputType == "ready") //end of parameter input
            {
                state.timer().start();
                break;
            }
            else    //unknown line
                getline(is, junk);
        }
    }
    else
    {
        //reads information about the current turn
        while(is >> inputType)
        {
            if(inputType == "w") //water square
            {
                is >> row >> col;
                state.m_grid[row][col].content = Square::WATER;
            }
            else if(inputType == "f") //m_food square
            {
                is >> row >> col;
                state.m_grid[row][col].content = Square::FOOD;
                state.m_food.push_back( Location(row, col) );
            }
            else if(inputType == "a") //live ant square
            {
                is >> row >> col >> player;
                state.m_grid[row][col].ant = player;
                if(player == 0)
                    state.m_my_ants.push_back(Location(row, col));
                else
                    // TODO: need to embed enemy id (needed for intelligent battle)
                    state.m_enemy_ants.push_back(Location(row, col)); 
            }
            else if(inputType == "d") //dead ant square
            {
                is >> row >> col >> player;
                state.m_grid[row][col].deadAnts.push_back(player);
            }
            else if(inputType == "h")
            {
                is >> row >> col >> player;
                state.m_grid[row][col].content = Square::HILL;
                state.m_grid[row][col].hill    = player;
                if(player == 0)
                    state.m_my_hills.push_back(Location(row, col));
                else
                    state.m_enemy_hills.push_back(Location(row, col));

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

    return is;
};
