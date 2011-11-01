#ifndef SQUARE_H_
#define SQUARE_H_

#include <vector>
#include <string>
#include <ostream>

/*
    struct for representing a square in the grid.
*/

struct Square;

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

    static std::string contentString( Content c );

    //
    // Members
    //
    
    Content content;           ///< What does this square contain 

    bool isVisible;            ///< Is this square visible to any ants?
    int  ant;                  ///< Start of turn ant player id, -1 if none
    int  hill;                 ///< Hill player id, -1 if none
    int  priority;             ///< Priority for nearby ants

    std::vector<int> deadAnts; ///< List of present dead ant's player ids

};

    
inline Square::Square()
    : content( UNKNOWN ),
      isVisible( false ),
      ant( -1 ),
      hill( -1 ),
      priority( 0 )
{
}


inline void Square::reset()
{
    if( content != WATER && content != UNKNOWN ) content = EMPTY;

    isVisible   = false;
    ant         = -1;
    hill        = -1;
    priority    =  0;

    deadAnts.clear();
};


inline void Square::setVisible()
{
  isVisible = 1;
  if( content == UNKNOWN ) content = EMPTY;
}


inline bool Square::isAvailable()const
{
    return ant < 0 && hill != 0 && content != WATER; 
}


inline std::string Square::contentString( Content c )
{
    static const char* lookup[5] = { "WATER", "HILL", "FOOD", "EMPTY", "UNKNOWN" };
    return lookup[ c ]; 
}


inline std::ostream& operator<<( std::ostream& os, const Square& s )
{
    os << "isVisible:" << s.isVisible << " ant:" << s.ant << " hill:" << s.hill; 
    return os;
}



#endif //SQUARE_H_
