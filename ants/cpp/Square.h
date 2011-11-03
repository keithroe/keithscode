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

// These are predicates for BFS searches
bool hasAnt( const Square& s );
bool hasAvailableAnt( const Square& s );
bool hasEnemyAnt( const Square& s );
bool hasFood( const Square& s );


std::ostream& operator<<( std::ostream& os, const Square& s );

struct Square
{
    //
    // Represents possible square contents
    //
    enum Content 
    { 
        WATER=0,
        HILL,
        FOOD,
        EMPTY,
        UNKNOWN
    };
    
    //
    // Methods
    //
    
    Square();

    /// Resets the information for the square except water information
    void reset();

    void setVisible();

    bool isAvailable()const;
    bool isWater()const;

    static std::string contentString( Content c );

    //
    // Members
    //
    
    Content content;           ///< What does this square contain 

    bool isVisible;            ///< Is this square visible to any ants?
    int  ant_id;               ///< Ant player id, -1 if none
    int  hill_id;              ///< Hill player id, -1 if none
    Ant* ant;                  ///< Ant data if present, NULL otherwise

    std::vector<int> deadAnts; ///< List of present dead ant's player ids
};

    
inline Square::Square()
    : content( UNKNOWN ),
      isVisible( false ),
      ant_id( -1 ),
      hill_id( -1 ),
      ant( NULL )
{
}


inline void Square::reset()
{
    if( content != WATER && content != UNKNOWN ) content = EMPTY;

    isVisible   = false;
    ant_id      = -1;
    hill_id     = -1;
    ant         =  NULL;

    deadAnts.clear();
};


inline void Square::setVisible()
{
  isVisible = 1;
  if( content == UNKNOWN ) content = EMPTY;
}


inline bool Square::isAvailable()const
{
    return ant_id < 0 && hill_id != 0 && content != WATER; 
}


inline bool Square::isWater()const
{
    return content == WATER; 
}


inline std::string Square::contentString( Content c )
{
    static const char* lookup[5] = { "WATER", "HILL", "FOOD", "EMPTY", "UNKNOWN" };
    return lookup[ c ]; 
}


inline std::ostream& operator<<( std::ostream& os, const Square& s )
{
    os << "isVisible:" << s.isVisible << " ant_id:" << s.ant_id << " hill_id:" << s.hill_id; 
    return os;
}


inline bool hasAnt( const Square& s )               { return s.ant_id == 0; }
inline bool hasAvailableAnt( const Square& s )      { return s.ant_id == 0 && s.ant->available(); }
inline bool hasEnemyAnt( const Square& s )          { return s.ant_id >  0; }
inline bool hasFood( const Square& s )              { return s.content == Square::FOOD; }


#endif //SQUARE_H_
