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
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>


// TODO:
//   * Fixed sized allocator for Board
//   * Profiling pass
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

#define LINFO  KLOG( Log::INFO )
#define LDEBUG KLOG( Log::DEBUG )

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


const int GRID_SIZE      = 15u;
const int NUM_GRID_CELLS = GRID_SIZE*GRID_SIZE;
const int INVALID_IDX    = 255;


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

//------------------------------------------------------------------------------
//
// Point
//
//------------------------------------------------------------------------------

struct Point
{
    unsigned char x;
    unsigned char y;

    unsigned char color;
    unsigned char group; 

    unsigned char neighbors[4]; // { L, R, B, T }
};

//------------------------------------------------------------------------------
//
// Game board
//
//------------------------------------------------------------------------------

class Board
{
public:
    Board();

    // No error checking for now
    void set( int x, int y, Color color ); 

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

    int numWhiteStones()const
    { return m_num_white_stones; }

    int numBlackStones()const
    { return m_num_black_stones; }

    bool gameFinished()const
    {  return m_num_black_stones + m_num_white_stones == NUM_GRID_CELLS; }

    const Point* grid()const 
    { return m_grid; }
    
    Point* grid()
    { return m_grid; }

    static int wrap( int x );
    static int clamp( int x );

private:
    void changeGroup( int old_group, int new_group );

    Color       m_color;
    int         m_num_groups;
    int         m_num_white_stones;
    int         m_num_black_stones;
    Point       m_grid[NUM_GRID_CELLS];
};


Board::Board()
    : m_color( WHITE ),
      m_num_groups( 0 ),
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

    if( color != m_color )
        return;

    // Assign group to first neighbor group we find
    for( int i = 0; i < 4; ++i )
    {
        int nidx = p.neighbors[ i ];
        if( nidx == INVALID_IDX )
            continue;

        if( m_grid[ nidx ].group != 0 )
        {
            p.group = m_grid[ nidx ].group;
            break;
        }
    }
    
    // All neighbors are empty, create new group
    if( p.group == 0 )
    {
        p.group = ++m_num_groups; 
        return;
    }

    // Collapse groups if necessary 
    for( int i = 0; i < 4; ++i )
    {
        int nidx = p.neighbors[ i ];
        if( nidx != INVALID_IDX             &&
            m_grid[ nidx ].group != 0       &&
            m_grid[ nidx ].group != p.group )
        {
            changeGroup( m_grid[ nidx ].group, p.group);
        }
    }
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
            Move >& move
            )=0;
};


//------------------------------------------------------------------------------
//
// RandomAI 
//
//------------------------------------------------------------------------------

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
    // TODO:Specialize this for MCTS since it cant use persistant m_explorations
    virtual void chooseExploration(
            Color color,
            const Board& board,
            Move& move
            );

    virtual void chooseExpansion(
            Color color,
            const Board& board,
            Move& move
            );

    std::vector<int> m_explorations;
    std::vector<int> m_expansions;

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
    if( board.numWhiteStones() == 0 || drand48() < 0.5f )
        chooseExploration( color, board, move );
    else
        chooseExpansion( color, board, move );
    //chooseExploration( color, board, move );
}


void RandomAI::chooseExploration(
        Color color,
        const Board& board,
        Move& move 
        )
{
    move.clear();

    // Walk the vector till we find the first legal move
    while( !m_explorations.empty() )
    {
        const int idx = m_explorations.back();
        m_explorations.pop_back();

        if( legalExploration( board, color, idx ) )
        {
            int x, y;
            to2D( idx, x, y );
            move.push_back( std::make_pair( x, y ) );
            return;
        }
    }

    // No legal exploration, fall back to expansion
    chooseExpansion( color, board, move );
}


void RandomAI::chooseExpansion(
        Color color,
        const Board& board,
        Move >& move 
        )
{
    LDEBUG << "Choosing exansion *************************";
    assert( board.numWhiteStones() != 0 );
    
    move.clear();
    std::set< int > expansions;

    int adjacent_groups[4];

    // Iterate over points in random order
    for( std::vector<int>::iterator it = m_expansions.begin();
         it != m_expansions.end();
         ++it )
    {
        const int idx = *it;
        const Point& p = board.get( idx );

        int x, y;
        to2D( idx, x, y );
        LDEBUG << "considering " << x << "," << y;
        if( p.color == NONE ) // empty point
        {
            LDEBUG << "    it is empty";         

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
                LDEBUG << "       no adj groups, skipping";
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
                    LDEBUG << "         group already expanded, skipping";
                    continue;
                }

                LDEBUG << "        inserting ";
                expansions.insert( group ); 
                move.push_back( coord );
                continue;
            }

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
                    {
                        expansions.insert( adjacent_groups[ i ] ); 
                    }
                    move.push_back( coord );
                    LDEBUG << "       >1 adj groups, works!!";
                }
                continue;
            }

            LDEBUG << "       >1 adj groups, skipping";
        }
    }

    if( move.empty() )
        chooseExploration( color, board, move );
}

//------------------------------------------------------------------------------
//
// MCTSAI 
//
//------------------------------------------------------------------------------

class MCTSAI : public RandomAI
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
              m_num_visits( 0 )
        {}

        const Board& board()const       { return m_board; }
        Board& board()                  { return m_board; }

        int numWins()const              { return m_num_wins;   }
        int numVisits()const            { return m_num_visits; }

        void addWin()                   { ++m_num_visits; ++m_num_wins; }
        void addLoss()                  { ++m_num_visits; }


        void addChild( Node* child )    { m_children.push_back( child ); }

        bool select( Node** next );

    private:
        Node*              m_parent;
        int                m_num_wins;
        int                m_num_visits;
        Board              m_board;
        Move               m_move;

        std::vector<Node*> m_children;
    };


    Node* m_root;
};

MCTSAI::MCTSAI()
    : RandomAI(),
      m_root( 0 )
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
    RandomAI::chooseMove( color, board, move );
    return;

    const double time_allowed = 0.5;
    Timer timer;
    timer.start();
    
    std::vector< Node* > visited;
    while( timer.getTimeElapsed() < time_allowed )
    {
        //
        // SELECT Walk tree, selecting moves until we hit never before seen pos
        //
        visited.clear();
        visited.push_back( m_root );

        Node* cur  = m_root;
        Node* next = 0;

        while( !cur->select( &next ) )
        {
            cur = next;
            visited.push_back( cur );

            if( cur->board().gameFinished() )
            {
                LDEBUG << "Reached finished game state during select!";
                continue;
            }
        }

        //
        // Expand the tree
        //
        cur->addChild( next );

        //
        // Run simulation
        //
        

        





        /*

        // The tree is traversed 
        while (current node elem_of T )
            last node <- current node
            current node <- Select(current node)
        end
        // A node is added 
        last node <- Expand(last node)

        // A simulated game is played
        R <- P lay simulated game(last node)

        // The result is backpropagated 
        current node <- last node 
        while (current node elem_of T ) 
            Backpropagation(current node, R) current node <- current node.parent
        end
        */


    }
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

    const Board& getBoard()const { return m_board; }

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
    while( true )
    {
        std::string opponent_move;
        std::cin >> opponent_move;

        AutoTimerRef<LoopTimerInfo> schedule_timer( main_loop_time );

        if( opponent_move == "Quit" )
            break;

        std::string my_move = player.doMove( opponent_move );

        LINFO << " my move: " << my_move;
        LINFO << "\n" << player.getBoard();
        std::cout << my_move << std::endl;
       
    }

    main_loop_time.log();
}



