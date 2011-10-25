#ifndef STATE_H_
#define STATE_H_

#include <iostream>
#include <stdio.h>
#include <cstdlib>
#include <cmath>
#include <string>
#include <vector>
#include <set>
#include <stack>

#include "Timer.h"
#include "Bug.h"
#include "Square.h"
#include "Location.h"

/*
    constants
*/

enum Direction
{
    NORTH=0,
    EAST,
    SOUTH,
    WEST,
    NUM_DIRECTIONS
};

const char DIRECTION_CHAR[NUM_DIRECTIONS] = { 'N', 'E', 'S', 'W' };
const int  DIRECTION_OFFSET[4][2]         = { {-1,0}, {0,1}, {1,0}, {0,-1} };

/*
    struct to store current state information
*/
class State
{
public:
    typedef Square**                           Grid;
    typedef std::vector< Location >            Locations;
    //typedef std::Set< Location >               LocationSet;

    State();
    ~State();

    void setup();
    void reset();

    void makeMove(const Location &loc, Direction direction);
    
    void updateVisionInformation();

    Bug&               debug()                  { return m_debug; }
    Timer&             timer()                  { return m_timer; }

    const Grid&        grid() const             { return m_grid; }
    const Locations&   myAnts()const            { return m_my_ants; }
    const Locations&   enemyAnts()const         { return m_enemy_ants; }
    const Locations&   myHills()const           { return m_my_hills; }
    const Locations&   enemyHills()const        { return m_enemy_hills; }
    const Locations&   food()const              { return m_food; }
    int                turn() const             { return m_turn; }

    void               endTurn()                { ++m_turn; }
    

    Location getLocation(const Location &startLoc, Direction direction)const;

    double   getDistance(const Location &loc1, const Location &loc2)const;

    friend std::ostream& operator<<(std::ostream &os, const State &state);
    friend std::istream& operator>>(std::istream &is, State &state);
private:

    int                       m_rows;
    int                       m_cols;
    int                       m_turn;
    int                       m_turns;
    int                       m_num_players;
    double                    m_attack_radius;
    double                    m_spawn_radius;
    double                    m_view_radius;
    double                    m_load_time;
    double                    m_turn_time;
    bool                      m_game_over;

    std::vector<double>       m_scores;

    Grid                      m_grid;
    Locations                 m_my_ants;
    Locations                 m_enemy_ants;
    Locations                 m_my_hills;
    Locations                 m_enemy_hills;
    Locations                 m_food;

    //LocationSet               m_destinations;

    Bug                       m_debug;
    Timer                     m_timer;


};


#endif //STATE_H_
