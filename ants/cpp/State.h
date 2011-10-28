#ifndef STATE_H_
#define STATE_H_

#include <iostream>
#include <vector>
#include <stdint.h>
#include <set>

#include "Timer.h"
#include "Bug.h"
#include "Map.h"
#include "Square.h"
#include "Location.h"
#include "Direction.h"

/*
    struct to store current state information
*/
class State
{
public:
    typedef std::vector< Location >            Locations;
    typedef std::set< Location >               LocationSet;

    State();
    ~State();

    void setup();
    void reset();

    void makeMove(const Location &loc, Direction direction);
    
    void updateVisionInformation();

    Bug&               debug()                  { return m_debug;         }
    Timer&             timer()                  { return m_timer;         }

    const Map&         map() const              { return m_map;           }
    const Locations&   myAnts()const            { return m_my_ants;       }
    const Locations&   enemyAnts()const         { return m_enemy_ants;    }
    const Locations&   myHills()const           { return m_my_hills;      }
    const Locations&   enemyHills()const        { return m_enemy_hills;   }
    const Locations&   food()const              { return m_food;          }
    const LocationSet& frontier()const          { return m_frontier;      }
    int                turn() const             { return m_turn;          }
    float              attackRadius()           { return m_attack_radius; }
    float              spawnRadius()            { return m_spawn_radius;  }
    float              viewRadius()             { return m_view_radius;   }

    void               endTurn()                { ++m_turn;               }

    void               prioritizeMap()          { m_map.prioritize();     }
    

    Location getLocation(const Location &startLoc, Direction direction)const;

    float    getDistance(const Location &loc1, const Location &loc2)const;

    friend std::ostream& operator<<(std::ostream &os, const State &state);
    friend std::istream& operator>>(std::istream &is, State &state);
private:

    int                       m_rows;
    int                       m_cols;
    int                       m_turn;
    int                       m_turns;
    int                       m_num_players;
    float                     m_attack_radius;
    float                     m_spawn_radius;
    float                     m_view_radius;
    float                     m_load_time;
    float                     m_turn_time;
    bool                      m_game_over;
    int64_t                   m_seed;

    std::vector<float>        m_scores;

    Map                       m_map;
    Locations                 m_my_ants;
    Locations                 m_enemy_ants;
    Locations                 m_my_hills;
    Locations                 m_enemy_hills;
    Locations                 m_food;
    LocationSet               m_frontier;

    //LocationSet               m_destinations;

    Bug                       m_debug;
    Timer                     m_timer;


};


#endif //STATE_H_
