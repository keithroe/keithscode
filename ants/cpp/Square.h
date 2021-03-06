#ifndef SQUARE_H_
#define SQUARE_H_


#include "Ant.h"

#include <vector>
#include <string>
#include <ostream>

/*
    struct for representing a square in the grid.
*/

struct Square;

//
//  Helpers
//

typedef bool (*SquarePredicate)( const Square& );

bool hasAnt( const Square& s );
bool hasAvailableAnt( const Square& s );
bool hasEnemyAnt( const Square& s );
bool hasFood( const Square& s );


std::ostream& operator<<( std::ostream& os, const Square& s );

struct Square
{
    //
    // Represents possible square types
    //
    enum Type 
    { 
        WATER=0,
        LAND,
        UNKNOWN
    };
    
    //
    // Methods
    //
    
    Square();

    /// Resets the information for the square except type information
    void reset();

    void setVisible();

    bool isAvailable()const;
    bool isWater()const;
    bool isLand()const;
    bool isTraversable()const;
    bool isUnknown()const;

    static std::string typeString( Type c );

    //
    // Members
    //
    
    Type  type;                 ///< What type of square is this 
    int   hill_id;              ///< Hill player id, -1 if none
    int   ant_id;               ///< Ant player id, -1 if none
    int   new_ant_id;           ///< Ant player id, -1 if none
    Ant*  ant;                  ///< Ant data if present, NULL otherwise
    Ant*  new_ant;              ///< Ant data if present, NULL otherwise
    bool  assigned;
    bool  visible;              ///< Is this square visible to any ants?
    bool  food;                 ///< Does this square contain food 
    bool  in_enemy_range;

    std::vector<int> deadAnts; ///< List of present dead ant's player ids
};

    
inline Square::Square()
    : type( UNKNOWN ),
      hill_id( -1 ),
      ant_id( -1 ),
      new_ant_id( -1 ),
      ant( NULL ),
      new_ant( NULL ),
      assigned( false ),
      visible( false ),
      food( false ),
      in_enemy_range( false )
{
}


inline void Square::reset()
{
    // Leave type alone as this is static between turns
    hill_id     = -1;
    ant_id      = -1;
    new_ant_id  = -1;
    ant         = NULL;
    new_ant     = NULL;
    assigned    = false;
    visible     = false;
    food        = false;
    in_enemy_range = false;

    deadAnts.clear();
};


inline void Square::setVisible()
{
    visible = true;
    if( type == UNKNOWN ) type = LAND;
}


inline bool Square::isAvailable()const
{
    return ant_id < 0 && new_ant_id < 0 && hill_id != 0 && !assigned && !food && type != WATER; 
}


inline bool Square::isWater()const
{
    return type == WATER; 
}


inline bool Square::isLand()const
{
    return type == LAND; 
}


inline bool Square::isTraversable()const
{
    return type == LAND && hill_id != 0; 
}


inline bool Square::isUnknown()const
{
    return type == UNKNOWN; 
}


inline std::string Square::typeString( Type c )
{
    static const char* lookup[5] = { "WATER", "LAND", "UNKNOWN" };
    return lookup[ c ]; 
}


inline std::ostream& operator<<( std::ostream& os, const Square& s )
{
    os << Square::typeString( s.type )
       << " visible:" << ( s.visible ? "true" : "false" )
       << " food:" << ( s.food    ? "true" : "false" )
       << " ant_id:" << s.ant_id 
       << " new_ant_id:" << s.new_ant_id 
       << " hill_id:" << s.hill_id 
       << " assigned:" << s.assigned; 
    return os;
}


inline bool hasAnt( const Square& s )          { return s.ant_id == 0 || s.new_ant_id == 0; }
inline bool hasEnemyAnt( const Square& s )     { return s.ant_id >  0;           }
inline bool hasFood( const Square& s )         { return s.food;                  }
inline bool hasEnemyHill( const Square& s )    { return s.hill_id > 0;           }
inline bool isLand( const Square& s )          { return s.type == Square::LAND;  }
inline bool isWater( const Square& s )         { return s.type == Square::WATER; }
inline bool notWater( const Square& s )        { return s.type != Square::WATER; }
inline bool isAvailable( const Square& s )     { return s.isAvailable();         }
inline bool isAvailableAndNotInCombat( const Square& s )     { return s.isAvailable() && !s.in_enemy_range;  }
inline bool isVisible( const Square& s )       { return s.visible;               }
inline bool notVisible( const Square& s )      { return !s.visible;              }
inline bool isWaterOrFood( const Square& s )   { return s.food || s.isWater();   }


#endif //SQUARE_H_
