//
// MIT License
//
// Copyright (c) 2008 r. keith morley 
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE
//


#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iterator>
#include <iostream>
#include <limits>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>


// TODO:
//   * Fixed sized allocator for Board
//   * Profiling pass
//   * color and board should be assigned to AI at construction
//

class Board;
class AI;
class Player;


//
// Logger class which strives for:
//   - Efficient handling of below-threshold logging events
//   - Simplicity
//
// Usage:
//   - Set compile-time debug level via preprocessor macro KLOG_MAX_LEVEL
//     All logging events > KLOG_MAX_LEVEL will be eliminated by compiler
//   - Set runtime debug level via Log::setReportingLevel()
//   - Use  KLOG macro to log at desired level:
//     KLOG( Log::WARNING ) << "Some warning message";
//     The logger will insert newlines after each message
//   - Defaults to writing to cerr. Use setStream() to write to cout, file, etc
//
// Inspired by http://drdobbs.com/cpp/201804215
// 
// (c) r. keith morley
//

#include <ctime>
#include <sys/time.h>

#ifndef KLOG_MAX_LEVEL
#  ifdef LOCAL
#     define KLOG_MAX_LEVEL Log::DEBUG
#  else
#     define KLOG_MAX_LEVEL Log::NONE
#  endif
#endif


#define KLOG( level )                                                          \
    if( level > KLOG_MAX_LEVEL)                 ;                              \
    else if( level > Log::getReportingLevel() ) ;                              \
    else Log().get( level )

#define LINFO   KLOG( Log::INFO )
#define LDEBUG  KLOG( Log::DEBUG )
#define LDEBUG1 KLOG( Log::DEBUG1 )

class Log
{
public:
    enum Level
    {
        NONE=0,
        ERROR,
        WARNING,
        INFO,
        DEBUG,
        DEBUG1,
        DEBUG2,
        DEBUG3,
        DEBUG4
    };

    //**************************************************************************
    // INFO: Do not use this public interface directly: use KLOG macro
    Log();
    ~Log();
    std::ostream& get( Level level = INFO );
    //
    //**************************************************************************

    static Level         getReportingLevel();
    static void          setReportingLevel( Level level );
    static void          setStream( std::ostream& out );

private:
    Log(const Log&);
    Log& operator=(const Log&);

    static std::string time();
    static std::string toString( Level level );
    
    static Level         s_reporting_level;
    static std::ostream* s_out;
};

std::ostream* Log::s_out             = &std::cerr;
Log::Level    Log::s_reporting_level = Log::WARNING;


//
inline Log::Log()
{
}


inline Log::~Log()
{
    *s_out << "\n";
}


inline std::ostream& Log::get( Log::Level level )
{
    *s_out << "[" << time() << "] " << toString( level ) << ": "
          << std::string( (level > INFO ? level - INFO : 0 ), '\t' );
    return *s_out;
}


inline Log::Level Log::getReportingLevel()
{
    return s_reporting_level;
}


inline void Log::setReportingLevel( Level level )
{
    s_reporting_level = level;
}

inline void Log::setStream( std::ostream& out )
{
    s_out = &out;
}

inline std::string Log::toString( Log::Level level )
{
    static const char* const level2string[] = 
    {
        "NONE",
        "ERROR",
        "WARNING",
        "INFO",
        "DEBUG",
        "DEBUG1",
        "DEBUG2",
        "DEBUG3",
        "DEBUG4"
    };
    return level2string[ level ];
}


inline std::string Log::time()
{
    char buffer[11];
    time_t t;
    std::time(&t);
    tm r = {0};
    strftime(buffer, sizeof(buffer), "%X", localtime_r(&t, &r));

    struct timeval tv;
    gettimeofday(&tv, 0);

    char result[100];
    std::sprintf(result, "%s.%03ld", buffer, (long)tv.tv_usec / 1000); 
    return result;
}






///
/// Simple timer class which allows client to keep track of elapsed time within
/// a program.  Mimics stopwatch functionality
///
class Timer
{
public:
    typedef unsigned long long Ticks;

    /// Initialize timer
    Timer();

    /// Start the timer running
    void start();

    /// Stop the timer 
    void stop();

    /// Resets timer, clearing previous time elapsed 
    void reset();

    /// Get time elapsed in seconds
    double getTimeElapsed()const;

private:

    bool   m_is_running;
    double m_time_elapsed;
    Ticks  m_start_ticks;
};




#include <cassert>


#if defined( __linux__ )

//
// gettimeofday based implementation for linux
//

#include <sys/Time.h>

namespace
{
    inline Timer::Ticks getTicks()
    {
        struct timeval tv;
        gettimeofday( &tv, 0L );
        return static_cast<Timer::Ticks>( tv.tv_sec  ) * 
               static_cast<Timer::Ticks>( 1000000    ) +
               static_cast<Timer::Ticks>( tv.tv_usec );

    }

    inline double secondsPerTick()
    {
        return 1.0e-6;
    }
}

#elif defined( __APPLE__ )

//
// mach_absolute_time based implementation for mac
//

#include <mach/mach_time.h>

namespace
{
    inline Timer::Ticks getTicks()
    {
        return mach_absolute_time();
    }

    inline double secondsPerTick()
    {
        static double seconds_per_tick = 0.0;

        if( seconds_per_tick == 0.0 )
        { 
            // mach_timebase_info gives conversion to nanoseconds
            mach_timebase_info_data_t info;
            mach_timebase_info( &info );
            seconds_per_tick = 1e-9 * static_cast<double>( info.numer ) /
                                      static_cast<double>( info.denom );
        }
        return seconds_per_tick;
    }
}

#else
#    error "Timer: Unsupported OS!"
#endif



Timer::Timer()
    : m_is_running( false ),
      m_time_elapsed( 0.0 ),
      m_start_ticks( 0 )
{
}


void Timer::start()
{
    assert( !m_is_running );
    m_start_ticks = getTicks();
    m_is_running  = true;
}


void Timer::stop()
{
    assert( m_is_running );
    m_time_elapsed = getTimeElapsed();
    m_is_running   = false;
}


void Timer::reset()
{
    m_time_elapsed = 0.0;
    m_is_running   = false;
}


double Timer::getTimeElapsed()const
{
    if( !m_is_running )
        return m_time_elapsed;

    const Ticks cur_ticks = getTicks();
    return ( cur_ticks - m_start_ticks ) * secondsPerTick() + m_time_elapsed;
}

///
/// Convert seconds to milliseconds
///
inline double secondsToMilliseconds( double seconds )
{ return seconds * 1.0e3; }


///
/// Convert seconds to microseconds
///
inline double secondsToMicroseconds( double seconds )
{ return seconds * 1.0e6; }


///
/// Calls client provided callback upon destruction, passing in time elapsed
/// since AutoTimer creation
///
template <typename Action>
class AutoTimer
{
public:
    explicit AutoTimer( Action action ) : m_action( action )
    { m_timer.start(); }

    ~AutoTimer()
    { m_action( m_timer.getTimeElapsed() ); }

private:
    Action m_action;
    Timer  m_timer;
};


///
/// Same semantics as AutoTimer but stores reference to action, rather than
/// copying
///
template <typename Action>
class AutoTimerRef
{
public:
    explicit AutoTimerRef( Action& action ) : m_action( action )
    { m_timer.start(); }

    ~AutoTimerRef()
    { m_action( m_timer.getTimeElapsed() ); }

private:
    Action& m_action;
    Timer   m_timer;
};


struct LoopTimerInfo
{
    LoopTimerInfo( const std::string& name )
        : name( name ),
          iterations( 0u ),
          max_time( 0.0 ),
          total_time( 0.0 )
    {}

    void operator()( double time_elapsed )
    {
        ++iterations;
        max_time = std::max( max_time, time_elapsed );
        total_time += time_elapsed;
    }

    void log()
    {
        std::cerr << std::fixed
                  << name << "\n"
                  << "  sum: " << total_time    << "s\n"
                  << "  max: " << max_time      << "s\n"
                  << "  avg: " << averageTime() << "s\n" << std::endl;
    }

    void reset()
    {
        iterations = 0u;
        max_time   = 0.0;
        total_time = 0.0;
    }

    double averageTime()const
    {
        return total_time / static_cast<double>( iterations );
    }

    std::string name;
    unsigned    iterations;
    double      max_time;
    double      total_time;
};



//------------------------------------------------------------------------------
//
//  Helper decls
//
//------------------------------------------------------------------------------


const int GRID_SIZE      = 15;
const int NUM_GRID_CELLS = GRID_SIZE*GRID_SIZE;
const int INVALID_IDX    = 255;
const int GROUP_PENALTY  = 6;


typedef std::vector< std::pair<int, int> > Move;

enum Color { NONE=0, WHITE, BLACK, };

char toChar( Color c );

// Currently no validation
int extractRow( const std::string& position );

// Currently no validation
int extractCol( const std::string& position );

void toCoord( const std::string& position, int& x, int& y );

std::string toString( int x, int y );

bool legalExploration( const Board& board, Color color, int x, int y );
bool legalExploration( const Board& board, Color color, int idx );

int to1D( int x, int y );
void to2D( int i, int& x, int& y );

bool chooseExpansion(
        Color color,
        const Board& board,
        const std::vector<unsigned char>& random_points,
        Move& move 
        );

bool chooseExploration(
        Color color,
        const Board& board,
        std::vector<unsigned char>& potential_expolorations,
        Move& move 
        );

float uct( float score, float num_visits, float num_visits_parent );

//------------------------------------------------------------------------------
//
// Point
//
//------------------------------------------------------------------------------

struct Point
{
    unsigned char x; //  These can be packed into a single byte if necessary
    unsigned char y; // 

    unsigned char color;
    unsigned char group; 

    unsigned char neighbors[4]; // { L, R, B, T }
};

std::ostream& operator<<( std::ostream& out, const Point& p )
{
    out << "[" << static_cast<int>( p.x ) << "," 
               << static_cast<int>( p.y ) << "] c: " 
               << static_cast<int>( p.color ) << " g: " 
               << static_cast<int>( p.group );
    return out;
}

//------------------------------------------------------------------------------
//
// Game board
//
//------------------------------------------------------------------------------

class Board
{
public:
    Board();

    Board( const Board& orig );

    // No error checking for now
    void set( int x, int y, Color color ); 
    
    // No error checking for now
    void set( const Move& move, Color color ); 

    // No error checking for now
    const Point& get( int x, int y )const
    { return m_grid[ x*GRID_SIZE+y ]; } 

    Point& get( int x, int y )
    { return m_grid[ x*GRID_SIZE+y ]; } 
    
    const Point& get( int idx )const
    { return m_grid[ idx ]; } 

    Point& get( int idx )
    { return m_grid[ idx ]; } 

    void setColor( Color c )
    { m_color = c; }

    int numStones( Color color )const
    { return color == WHITE ? m_num_white_stones : m_num_black_stones; }

    int numWhiteStones()const
    { return m_num_white_stones; }

    int numBlackStones()const
    { return m_num_black_stones; }

    int score( Color c )const
    {
        return c == WHITE                                            ? 
               m_num_white_stones - GROUP_PENALTY*m_num_white_groups :
               m_num_black_stones - GROUP_PENALTY*m_num_black_groups ;
    }

    bool gameFinished()const
    {  return m_num_black_stones + m_num_white_stones == NUM_GRID_CELLS; }

    const Point* grid()const 
    { return m_grid; }
    
    Point* grid()
    { return m_grid; }

    Color winner()const;

    static int wrap( int x );
    static int clamp( int x );

private:
    void changeGroup( int old_group, int new_group );

    Color       m_color;
    int         m_next_group;
    int         m_num_white_groups;
    int         m_num_black_groups;
    int         m_num_white_stones;
    int         m_num_black_stones;
    Point       m_grid[NUM_GRID_CELLS];
};


Board::Board()
    : m_color( WHITE ),
      m_next_group( 0 ),
      m_num_white_groups( 0 ),
      m_num_black_groups( 0 ),
      m_num_white_stones( 0 ),
      m_num_black_stones( 0 )
{
    int idx = 0;
    for( int i = 0; i < GRID_SIZE; ++i )
    {
        for( int j = 0; j < GRID_SIZE; ++j )
        {
            Point& p = m_grid[ idx++ ];
            p.x       = static_cast<unsigned char>( i );
            p.y       = static_cast<unsigned char>( j );
            p.color   = static_cast<unsigned char>( NONE );
            p.group   = 0;
            p.neighbors[0] = ( i == 0           ? INVALID_IDX : to1D(i-1, j) );
            p.neighbors[1] = ( i == GRID_SIZE-1 ? INVALID_IDX : to1D(i+1, j) );
            p.neighbors[2] = ( j == 0           ? INVALID_IDX : to1D(i, j-1) );
            p.neighbors[3] = ( j == GRID_SIZE-1 ? INVALID_IDX : to1D(i, j+1) );
        }
    }
}


Board::Board( const Board& orig )
    : m_color( orig.m_color ),
      m_next_group( orig.m_next_group ),
      m_num_white_groups( orig.m_num_white_groups ),
      m_num_black_groups( orig.m_num_black_groups ),
      m_num_white_stones( orig.m_num_white_stones ),
      m_num_black_stones( orig.m_num_black_stones )
{
    memcpy( m_grid, orig.m_grid, sizeof( m_grid ) );
}


void Board::set( int x, int y, Color color ) 
{ 
    assert( x >= 0 && x < GRID_SIZE ); 
    assert( y >= 0 && y < GRID_SIZE ); 
    assert( m_grid[ x*GRID_SIZE+y ].color == NONE );

    if( color == WHITE )
        m_num_white_stones++;
    else
        m_num_black_stones++;

    const int idx = to1D( x, y );

    Point& p = m_grid[ idx ];
    p.color = static_cast<unsigned char>( color ); 

    // Assign group to first neighbor group we find
    for( int i = 0; i < 4; ++i )
    {
        int nidx = p.neighbors[ i ];
        if( nidx == INVALID_IDX )
            continue;

        if( m_grid[ nidx ].color == color && m_grid[ nidx ].group != 0  )
        {
            p.group = m_grid[ nidx ].group;
            break;
        }
    }
    
    // All neighbors are empty, create new group
    if( p.group == 0 )
    {
        p.group = ++m_next_group; 
        if( color == WHITE )
            m_num_white_groups++;
        else
            m_num_black_groups++;
        return;
    }

    // Collapse groups if necessary 
    for( int i = 0; i < 4; ++i )
    {
        int nidx = p.neighbors[ i ];
        if( nidx != INVALID_IDX             &&
            m_grid[ nidx ].color == color   &&
            m_grid[ nidx ].group != 0       &&
            m_grid[ nidx ].group != p.group )
        {
            changeGroup( m_grid[ nidx ].group, p.group);
            if( color == WHITE )
                m_num_white_groups--;
            else
                m_num_black_groups--;
        }
    }
}

    
void Board::set( const Move& move, Color color )
{
    for( Move::const_iterator it = move.begin(); it != move.end(); ++it )
        set( it->first, it->second, color );
}


Color Board::winner()const
{
    const int wscore = m_num_white_stones - GROUP_PENALTY*m_num_white_groups;
    const int bscore = m_num_black_stones - GROUP_PENALTY*m_num_black_groups;

    if( wscore > bscore )
        return WHITE;
    else
        return BLACK;
}


void Board::changeGroup( int old_group, int new_group )
{
    for( int i = 0; i < NUM_GRID_CELLS; ++i )
        if( m_grid[i].group == old_group )
            m_grid[i].group = new_group;
}


int Board::wrap( int x )
{ 
    const int s = GRID_SIZE;
    return x < 0   ? s-1 : 
           x > s-1 ? 0   :
           x;
}


int Board::clamp( int x )
{ 
    const int s = GRID_SIZE;
    return x < 0   ? 0   : 
           x > s-1 ? s-1 :
           x;
}


std::ostream& operator<<( std::ostream& out, const Board& board )
{
    out << " -";
    for( int i = 0; i < GRID_SIZE; ++i )
        out << "--";
    out << "\n";

    for( int i = GRID_SIZE - 1; i >= 0; --i ) // row
    {
        out << "| "; 
        for( int j = 0; j < GRID_SIZE; ++j ) // col
        {
            out << toChar( static_cast<Color>( board.get( j, i ).color ) ) 
                << ' ';
        }
        out << "| " << i + 1 << "\n";
    }

    out << " -";
    for( int i = 0; i < GRID_SIZE; ++i )
        out << "--";
    out << std::endl << " "; //flush
    for( int i = 0; i < GRID_SIZE; ++i )
        out << " " << static_cast<char>( 'A' + i );
    out << std::endl; //flush




    out << " -";
    for( int i = 0; i < GRID_SIZE; ++i )
        out << "----";
    out << "\n";

    for( int i = GRID_SIZE - 1; i >= 0; --i ) // row
    {
        out << "|"; 
        for( int j = 0; j < GRID_SIZE; ++j ) // col
        {
            int g = static_cast<int>( board.get( j, i ).group );
            if( g )
                out << std::setw( 4 ) << g ;
            else
                out << std::setw( 4 ) << '.';
        }
        out << " | " << i + 1 << "\n";
    }

    out << " -";
    for( int i = 0; i < GRID_SIZE; ++i )
        out << "----";
    out << std::endl << " "; //flush
    for( int i = 0; i < GRID_SIZE; ++i )
        out << "   " << static_cast<char>( 'A' + i );
    out << std::endl; //flush

    return out;
}



//------------------------------------------------------------------------------
//
// AI
//
//------------------------------------------------------------------------------

class AI
{
public:
    virtual ~AI() {}

    virtual void chooseMove( 
            Color color,
            const Board& board,
            Move& move
            )=0;
};


//------------------------------------------------------------------------------
//
// RandomAI 
//
//------------------------------------------------------------------------------

// TODO: at this point there is a loose coupling between the board passed in
//   and the exploration list.  This means the same board must be passed in each
//   time with only the moves recommended by the RandomAI being used on the
//   board.  AI should probably own board.  This should be fixed so I can sleep
//   at night.
class RandomAI : public AI
{
    
public:
    RandomAI();

    virtual ~RandomAI() {}


    virtual void chooseMove( 
            Color color,
            const Board& board,
            Move& move
            );
protected:

    std::vector<unsigned char> m_explorations;
    std::vector<unsigned char> m_expansions;

};


RandomAI::RandomAI()
    : m_explorations( NUM_GRID_CELLS ),
      m_expansions( NUM_GRID_CELLS )
{
    for( int i = 0; i < NUM_GRID_CELLS; ++i )
        m_explorations[i] = m_expansions[i] = i;
    
    std::random_shuffle( m_explorations.begin(), m_explorations.end() ); 
    std::random_shuffle( m_expansions.begin(), m_expansions.end() ); 
}


void RandomAI::chooseMove( 
        Color color,
        const Board& board,
        Move& move 
        )
{
    if( board.numStones( color ) == 0 || drand48() < 0.5f )
    {
        if( !chooseExploration( color, board, m_explorations, move ) )
            chooseExpansion( color, board, m_expansions, move );
    }
    else
    {
        if( !chooseExpansion( color, board, m_expansions, move ) )
            chooseExploration( color, board, m_explorations, move );
    }
}




//------------------------------------------------------------------------------
//
// MCTSAI 
//
//------------------------------------------------------------------------------

class MCTSAI : public AI
{
public:
    MCTSAI();

    ~MCTSAI();


    void chooseMove( 
            Color color,
            const Board& board,
            Move& move
            );
protected:

    class Node
    {
    public:
        Node( Node* parent )
            : m_parent( parent ),
              m_num_wins( 0 ),
              m_num_visits( 0 ),
              m_explorations( NUM_GRID_CELLS ),
              m_expansions( NUM_GRID_CELLS )
        {
            for( int i = 0; i < NUM_GRID_CELLS; ++i )
                m_explorations[i] = m_expansions[i] = i;

            std::random_shuffle( m_explorations.begin(), m_explorations.end() );
            std::random_shuffle( m_expansions.begin(), m_expansions.end() ); 
        }

        const Board& board()const       { return m_board; }
        Board& board()                  { return m_board; }

        int numWins()const              { return m_num_wins;   }
        int numVisits()const            { return m_num_visits; }

        void addResult( int score )     { ++m_num_visits; m_num_wins += score; }

        void addChild( Node* child )    { m_children.push_back( child ); }

        Node* parent()                  { return m_parent; }

        const Move& move()const         { return m_move; }

        const std::vector<Node*>& children()const { return m_children; }

        void  bestMove( Move& move );

        float score()const              
        { 
            assert( m_num_visits ); 
            return static_cast<float>( m_num_wins ) / 
                   static_cast<float>( m_num_visits );
        }

        bool select( Node** next, Color color );

    private:
        Node*              m_parent;
        Move               m_move;
        int                m_num_wins;
        int                m_num_visits;
        Board              m_board;

        std::vector<Node*>          m_children;
        std::vector<unsigned char>  m_explorations;
        std::vector<unsigned char>  m_expansions;
    };

    static void printGraph( const Node* node, const std::string& filename );
    static void printGraphNode( const Node* node, std::ostream& out );

    unsigned m_move_number;
    Node* m_root;

};


MCTSAI::MCTSAI()
    : AI(),
      m_move_number( 0 ),
      m_root( new Node( 0 ) )
{
}


MCTSAI::~MCTSAI()
{
}


void MCTSAI::chooseMove( 
            Color color,
            const Board& board,
            Move& move
            )
{
    m_move_number++;

    /*
    RandomAI random_ai;
    random_ai.chooseMove( color, board, move );
    return;
    */


    const double time_allowed = 5;
    Timer timer;
    timer.start();
    
    std::vector< Node* > visited;
    unsigned iter_count = 0u;
    while( timer.getTimeElapsed() < time_allowed )
    {
        iter_count++;
        LDEBUG << " Iteration: " << iter_count << std::endl;

        if( iter_count % 10 == 0 )
        {
            std::ostringstream oss;
            oss << "graph_" << m_move_number << "_" << iter_count << ".dot";
            printGraph( m_root, oss.str() );
        }

        //
        // SELECT: Walk tree, selecting moves until we hit never before seen pos
        //
        LDEBUG << "    SELECT: ";
        visited.clear();
        visited.push_back( m_root );

        Node* cur  = m_root;
        Node* next = 0;

        while( cur->select( &next, color ) )
        {
            cur = next;
            visited.push_back( cur );

            if( cur->board().gameFinished() )
            {
                LDEBUG << "Reached finished game state during select!";
                break;
            }
        }

        //
        // EXPAND: add new node to  the tree
        //
        LDEBUG << "    EXPAND";
        cur->addChild( next );

        //
        // SIMULATE: Run simulation
        //
        LDEBUG << "    SIMULATE";
        RandomAI  random_ai;
        Board     sim_board = next->board();
        Color     cur_color = color;
        Move      move;
        while( !sim_board.gameFinished() )
        {
            random_ai.chooseMove( cur_color, sim_board, move );
            sim_board.set( move, cur_color );

            cur_color = (cur_color == WHITE ? BLACK : WHITE );
        }

        assert( sim_board.winner() != NONE );
        const int score = sim_board.winner() == color ? 1 : 0;

        //
        // PROPAGATE: Back propagate the score
        //
        LDEBUG << "    PROPAGATE";
        next->addResult( score );
        Node* parent = next->parent();
        while( parent )
        {
            parent->addResult( score );
            parent = parent->parent();
        }
    }

    m_root->bestMove( move );
}


void MCTSAI::printGraph( const Node* node, const std::string& filename )
{
    std::ofstream out( filename.c_str() );
    out << "digraph g {\n"
        << "  graph [ size=\"8.0,10.5\""
        << " bgcolor=white"
        << " overlap=false"
        << " splines=true"
        << " labelloc=\"t\"];\n";

    out << "  node [fontname=\"Courier\","
        << "fontsize=10,"
        << "textalign=left,"
        << "shape=Mrecord];\n";

    printGraphNode( node, out );

    out <<"}";
}
    

void MCTSAI::printGraphNode( const Node* n, std::ostream& out )
{
    out << "  " << reinterpret_cast<unsigned long long>( n )
        << " [style=filled,fillcolor=\"#FFFFFF\",labeljust=l,label=\"Move: ";

    for( Move::const_iterator it = n->move().begin();
         it != n->move().end();
         ++it )
    {
        out << toString( it->first, it->second );
        if( it + 1 != n->move().end() )
            out << "-";
    }
    out << "\\n";
    out << "Wins  : " << n->numWins() << "\\n";
    out << "Visits: " << n->numVisits();
    out << "\"];\n";

    for( std::vector<Node*>::const_iterator it = n->children().begin();
         it != n->children().end();
         ++it )
    {
        out << "  " 
            << reinterpret_cast<unsigned long long>( n ) << " -> " 
            << reinterpret_cast<unsigned long long>( *it ) << ";\n";
    }
    
    for( std::vector<Node*>::const_iterator it = n->children().begin();
         it != n->children().end();
         ++it )
    {
        printGraphNode( *it, out );
    }
}


void  MCTSAI::Node::bestMove( Move& move )
{
    assert( !m_children.empty() );

    float max_score = std::numeric_limits<float>::min();  
    Node* best = 0;

    for( std::vector<Node*>::iterator it = m_children.begin();
         it != m_children.end();
         ++it )
    {
        Node* child = *it;
        if( child->score() > max_score )
        {
            best = child;
            max_score = child->score();
        }
    }

    move.assign( best->m_move.begin(), best->m_move.end() );
}


bool MCTSAI::Node::select( Node** node, Color c )
{
    LDEBUG << "        selecting from " << m_children.size() << " children";

    const float new_node_uct_score = uct( 0, 0, m_num_visits ); 
    float best_node_uct_score = -1e16f; //std::numeric_limits<float>::min();
    Node* best_node = 0;
        
    LDEBUG << "        initial UCT score " << new_node_uct_score;

    // If we have surpassed the threshold, find the best existing uct score
    const size_t T = 4;
    if( m_children.size() > T ) 
    {
        for( std::vector<Node*>::iterator it = m_children.begin();
             it != m_children.end();
             ++it )
        {
            Node* c = *it;
            float uct_score = uct( c->score(), c->numVisits(), m_num_visits );
            LDEBUG << "        new UCT score " << uct_score;
            if( uct_score > best_node_uct_score )
            {
                best_node = c;
                best_node_uct_score = uct_score;
            }
        }
    }

            std::cerr << "    here a " << best_node_uct_score << std::endl;
    if( new_node_uct_score > best_node_uct_score )
    {
            std::cerr << "    here b " << std::endl;
        //
        // Expand the tree here
        //

        Node* n = new Node( this ); 

        const float p_explore = 0.5f; // TODO:  make some function of mov
        if( m_board.numStones( c ) == 0 || drand48() < p_explore )
        {
            std::cerr << "    here 0 " << std::endl;
            if( chooseExploration( c, m_board, m_explorations, n->m_move ) )
            {
            std::cerr << "    here 0.1 " << std::endl;
                *node = n;
                n->m_board.set( n->m_move, c );
                return false;
            }
            std::cerr << "    here 1 " << std::endl;
        }
        else
        {
            if( chooseExpansion( c, m_board, m_expansions, n->m_move ) )
            {
                *node = n;
                n->m_board.set( n->m_move, c );
                return false;
            }
        }

        delete n;    
    }

    // If a preexisting node was selected
    assert( best_node );
    *node = best_node;;
    return true;
}



//------------------------------------------------------------------------------
//
//  Player
//
//------------------------------------------------------------------------------

class Player
{
public:
    Player();

    std::string doMove( const std::string& opponent_move );

    const Board& board()const { return m_board; }

private:
    void placeOpponentStones( const std::string& opponent_move );

    Board     m_board;            // Current board
    Color     m_color;
    Color     m_opp_color;
    int       m_move_number;
    AI*       m_ai;
};


Player::Player()
    : m_color( NONE ),
      m_opp_color( NONE ),
      m_move_number( 0 ),
      m_ai( new MCTSAI )
{
}


std::string Player::doMove( const std::string& opponent_move )
{

    m_move_number++;

    if( m_color == NONE ) // First move
    {
        if( opponent_move == "Start" )
        {
            m_color     = WHITE;
            m_opp_color = BLACK;
        }
        else
        {
            m_color     = BLACK;
            m_opp_color = WHITE;
        }

        m_board.setColor( m_color );
    }

    placeOpponentStones( opponent_move );

    Move move; // TODO: persistent
    m_ai->chooseMove( m_color, m_board, move );

    std::ostringstream oss;
    for( Move::iterator it = move.begin(); it != move.end(); ++it )
    {
        m_board.set( it->first, it->second, m_color );
        oss << toString( it->first, it->second );
        if( it + 1 != move.end() )
            oss << "-";
    }
    return oss.str(); 
}


void Player::placeOpponentStones( const std::string& opponent_move )
{
    if( opponent_move != "Start" )
    {
        std::istringstream iss( opponent_move );
        std::string position;
        while( std::getline( iss, position, '-' ) )
        {
            int x, y;
            toCoord( position, x, y );
            m_board.set( x, y, m_opp_color );
        }
    }
}


//------------------------------------------------------------------------------
//
//  Helper definitions 
//
//------------------------------------------------------------------------------

inline char toChar( Color c )
{
    return c == WHITE ? 'W' :
           c == BLACK ? 'B' :
                        '.';
}


// Currently no validation
inline int extractRow( const std::string& position )
{
    const int offset = 1u; // Index is 1 based (A1 maps to index 0,0)

    if( position.size() == 2 )
        return position[1] - '0' - offset;
    else
        return ( position[1] - '0' ) * 10u + position[2] - '0' - offset;
}


// Currently no validation
inline int extractCol( const std::string& position )
{
    return position[0] - 'A';
}


inline void toCoord( const std::string& position, int& x, int& y )
{
    x = extractCol( position );
    y = extractRow( position );
}


inline std::string toString( int x, int y )
{
    char buf[ 4 ];
    buf[0] = 'A' + x;

    y += 1; // One based index
    if( y < 10 )
    {
        buf[1] = '0' + y;
        buf[2] = '\0';
    }
    else
    {
        buf[1] = '1';
        buf[2] = '0' + ( y - 10 );
        buf[3] = '\0';
    }
    return buf;
}


inline bool legalExploration( const Board& b, Color c, int idx )
{
    assert( idx < NUM_GRID_CELLS );
    const Point* g = b.grid(); 
    const Point& p = g[idx];

    /*
    int x, y;
    to2D( idx, x, y );
    LDEBUG << "   testing " << x << "," << y; 
    for( int i = 0; i < 4; ++i )
    {
        if( p.neighbors[i] == INVALID_IDX )
            continue;
        LDEBUG << "      neighbor " << i;
        to2D( p.neighbors[i], x, y );
        LDEBUG << "      neighbor " << i << ": " << x << ", " << y;
        const Point& np = g[ p.neighbors[i] ];
    }
    */

    return( p.color == NONE                                                   &&
          ( p.neighbors[0] == INVALID_IDX || g[ p.neighbors[0] ].color != c ) &&
          ( p.neighbors[1] == INVALID_IDX || g[ p.neighbors[1] ].color != c ) &&
          ( p.neighbors[2] == INVALID_IDX || g[ p.neighbors[2] ].color != c ) &&
          ( p.neighbors[3] == INVALID_IDX || g[ p.neighbors[3] ].color != c ) );
}

inline bool legalExploration( const Board& b, Color c, int x, int y )
{
    return legalExploration( b, c, to1D( x, y ) );
}


inline int to1D( int x, int y )
{
    return x*GRID_SIZE+y;
}


void to2D( int i, int& x, int& y )
{
    x = i / GRID_SIZE;
    y = i % GRID_SIZE;
}

bool chooseExploration(
        Color color,
        const Board& board,
        std::vector<unsigned char>& potential_explorations,
        Move& move 
        )
{
    LDEBUG << "Choosing exploration *************************";
    move.clear();

    // Walk the vector till we find the first legal move
    while( !potential_explorations.empty() )
    {
        const int idx = potential_explorations.back();
        potential_explorations.pop_back();


        if( legalExploration( board, color, idx ) )
        {
            int x, y;
            to2D( idx, x, y );
            move.push_back( std::make_pair( x, y ) );
            return true;
        }
    }

    // No legal exploration found
    return false;

}

bool chooseExpansion(
        Color color,
        const Board& board,
        const std::vector<unsigned char>& random_points,
        Move& move 
        )
{
    LDEBUG << "Choosing expansion *************************";
    assert( board.numStones( color ) != 0 );
    
    move.clear();
    std::set< int > expansions;

    int adjacent_groups[4];

    // Iterate over points in random order
    for( std::vector<unsigned char>::const_iterator it = random_points.begin();
         it != random_points.end();
         ++it )
    {
        const int idx = *it;
        const Point& p = board.get( idx );

        int x, y;
        to2D( idx, x, y );
        if( p.color == NONE ) // empty point
        {
            LDEBUG1 << "considering " << x << "," << y;

            // Find this points adjacent groups
            int num_adjacent = 0;
            for( int i = 0; i < 4; ++i )
            {
                int nidx = p.neighbors[ i ];
                if( nidx == INVALID_IDX )
                    continue;

                const Point& neighbor_p = board.get( nidx );
                if( neighbor_p.color == color )
                {
                    int ngroup = neighbor_p.group;

                    // Avoid redundant group entries
                    if( ( num_adjacent < 1 || adjacent_groups[0] != ngroup ) &&
                        ( num_adjacent < 2 || adjacent_groups[1] != ngroup ) && 
                        ( num_adjacent < 3 || adjacent_groups[2] != ngroup ) ) 
                        adjacent_groups[ num_adjacent++ ] = neighbor_p.group;
                }
            }

            // Not on border of any group -- skip it
            if( num_adjacent == 0 )
            {
                LDEBUG1 << "       no adj groups, skipping";
                continue;
            }

            int x, y;
            to2D( idx, x, y );
            const std::pair<int, int> coord = std::make_pair( x, y );

            // If we have a single adjacent group
            if( num_adjacent == 1 )
            {
                int group = adjacent_groups[0];
                if( expansions.find( group ) != expansions.end() )
                {
                    LDEBUG1 << "         group already expanded, skipping";
                    continue;
                }

                LDEBUG1 << "        inserting ";
                expansions.insert( group ); 
                move.push_back( coord );
                continue;
            }

            // If we have multiple adjacent friendly groups, add this move if
            // none of the adjacent groups have been expanded yet
            if( num_adjacent > 1 )
            {
                bool ok_to_join = true;
                for( int i = 0; i < num_adjacent; ++i )
                {
                    if( expansions.count( adjacent_groups[ i ] ) > 0 )
                    {
                        ok_to_join = false;
                        break;
                    }
                }
                if( ok_to_join )
                {
                    for( int i = 0; i < num_adjacent; ++i )
                        expansions.insert( adjacent_groups[ i ] ); 

                    move.push_back( coord );
                    LDEBUG1 << "       >1 adj groups, works!!";
                }
                continue;
            }

            LDEBUG1 << "       >1 adj groups, skipping";
        }
    }

    return( !move.empty() );
}

float uct( float score, float num_visits, float num_visits_parent )
{
    const float C = 0.1f;
    return score + C * sqrtf( ( logf( num_visits_parent+1.0f ) ) / 
                              ( num_visits+1.0f ) );
}

//------------------------------------------------------------------------------
//
//  main
//
//------------------------------------------------------------------------------


int main( int argc, char** argv )
{

    Log::setReportingLevel( Log::DEBUG );
    
    LDEBUG << "Board size is " << sizeof( Board );
    LDEBUG << "Point size is " << sizeof( Point );

    Player player;

    LoopTimerInfo main_loop_time( "Main loop" );
    std::vector< std::string > opponent_moves;
    while( true )
    {
        std::string opponent_move;
        std::cin >> opponent_move;

        AutoTimerRef<LoopTimerInfo> schedule_timer( main_loop_time );

        LINFO << " opp move: " << opponent_move;
        opponent_moves.push_back( opponent_move );

        if( std::cin.eof() || opponent_move == "Quit" )
            break;

        std::string my_move = player.doMove( opponent_move );

        LINFO << " my move: " << my_move;
        LINFO << "\n" << player.board();
        std::cout << my_move << std::endl;
        std::cout.flush();
    }

    std::cerr << "WHITE: " << player.board().score( WHITE ) << "\n"
              << "BLACK: " << player.board().score( BLACK ) << "\n";
    main_loop_time.log();

    std::cerr << "REPLAY ------------------------------------\n";
    std::ostream_iterator< std::string > out( std::cerr, "\n" );
    std::copy( opponent_moves.begin(), opponent_moves.end(), out );
}



