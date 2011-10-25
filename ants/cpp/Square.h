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
        EMPTY 
    };
    

    //
    // Methods
    //
    
    Square();

    /// Resets the information for the square except water information
    void reset();


    //
    // Members
    //
    
    Content content;           ///< What does this square contain 

    bool isVisible;            ///< Is this square visible to any ants?
    int  ant;                  ///< Ant player id, -1 if none present
    int  hill;                 ///< Hill player id, -1 if none present

    std::vector<int> deadAnts; ///< List of present dead ant's player ids

};

    
Square::Square()
    : content( EMPTY ),
      isVisible( false ),
      ant( -1 ),
      hill( -1 )
{
}


void Square::reset()
{
    if( content != WATER ) content = EMPTY;

    isVisible  =  0;
    ant        = -1;
    hill       = -1;

    deadAnts.clear();
};

#endif //SQUARE_H_
