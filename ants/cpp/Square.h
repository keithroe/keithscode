#ifndef SQUARE_H_
#define SQUARE_H_

#include <vector>

/*
    struct for representing a square in the grid.
*/
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

    //
    // Members
    //
    
    Content content;           ///< What does this square contain 

    bool isVisible;            ///< Is this square visible to any ants?
    int  ant;                  ///< Start of turn ant player id, -1 if none
    int  newAnt;               ///< End of turn ant player id, -1 if none
    int  hill;                 ///< Hill player id, -1 if none
    int  priority;             ///< Priority for nearby ants

    std::vector<int> deadAnts; ///< List of present dead ant's player ids

};

    
inline Square::Square()
    : content( UNKNOWN ),
      isVisible( false ),
      ant( -1 ),
      newAnt( -1 ),
      hill( -1 ),
      priority( 0 )
{
}


inline void Square::reset()
{
    if( content != WATER && content != UNKNOWN ) content = EMPTY;

    isVisible   = false;
    ant         = -1;
    newAnt      = -1;
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
    return newAnt < 0 && hill != 0 && content != WATER; 
}

#endif //SQUARE_H_
