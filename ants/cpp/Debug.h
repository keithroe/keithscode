#ifndef DEBUG_H_
#define DEBUG_H_


#include <iostream>
#include <fstream>
#include "Location.h"

//------------------------------------------------------------------------------
//
// visualizer helpers
//
//------------------------------------------------------------------------------
inline void circle( const Location& loc, float radius, bool fill )
{
#ifdef DEBUG 
#ifdef VISUALIZER
    std::cout << "v circle " << loc.row << " " << loc.col << " " << radius 
              << " " << ( fill? "true" : "false" ) << std::endl;
#endif
#endif
}

inline void setFillColor( int r, int g, int b, float a )
{
#ifdef DEBUG
#ifdef VISUALIZER
    std::cout << "v setFillColor " << r << " " << g << " " << b << " " << a
              << std::endl;
#endif
#endif
}

inline void setLineColor( int r, int g, int b, float a )
{
#ifdef DEBUG
#ifdef VISUALIZER
    std::cout << "v setLineColor " << r << " " << g << " " << b << " " << a
              << std::endl;
#endif
#endif
}

inline void tile( const Location& loc )
{
#ifdef DEBUG
#ifdef VISUALIZER
    std::cout << "v tile " << loc.row << " " << loc.col << std::endl;
#endif
#endif
}

//------------------------------------------------------------------------------
//
// Logging helpers
//
//------------------------------------------------------------------------------
struct Debug
{

    static Debug& stream() 
    { 
        static Debug debug;
        return debug;
    }

//#define PRINT_TO_STDERR 
#ifdef PRINT_TO_STDERR

    std::ostream file;

    Debug()
        : file( std::cerr.rdbuf() )
    {
    }
    
    inline void open(const std::string &filename)
    {
    };

    //closes the ofstream
    inline void close()
    {
    };

#else
    std::ofstream file;
    
    Debug()
    {
        open( "./debug.txt" );
    };

    Debug( const std::string& filename )
    {
        open( filename );
    };

    //opens the specified file
    inline void open(const std::string &filename)
    {
        #ifdef DEBUG
            file.open(filename.c_str());
        #endif
    };

    //closes the ofstream
    inline void close()
    {
        #ifdef DEBUG
            file.close();
        #endif
    };
#endif
};


inline Debug& operator<<(Debug &bug, std::ostream& (*manipulator)(std::ostream&))
{
    #ifdef DEBUG
        bug.file << manipulator;
    #endif

    return bug;
};


template <class T>
inline Debug& operator<<(Debug &bug, const T &t)
{
    #ifdef DEBUG
        bug.file << t;
    #endif

    return bug;
};


#endif //DEBUG_H_
