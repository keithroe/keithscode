
#ifndef BATTLE_H_
#define BATTLE_H_

#include <vector>
#include <set>

class Map;
class Ant;
struct Location;


void battle( Map& map,
             const std::vector<Ant*>& ants,
             const std::vector<Location>& enemy_ants,
             std::set<Ant*>& assigned );

#endif // BATTLE_H_
