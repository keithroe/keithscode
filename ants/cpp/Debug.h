#ifndef DEBUG_H_
#define DEBUG_H_


#include <fstream>


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
