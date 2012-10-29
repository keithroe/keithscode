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

#ifndef CCUP_TIMER_H__
#define CCUP_TIMER_H__

#include <cassert>

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


inline
Timer::Timer()
    : m_is_running( false ),
      m_time_elapsed( 0.0 ),
      m_start_ticks( 0 )
{
}


inline
void Timer::start()
{
    assert( !m_is_running );
    m_start_ticks = getTicks();
    m_is_running  = true;
}


inline
void Timer::stop()
{
    assert( m_is_running );
    m_time_elapsed = getTimeElapsed();
    m_is_running   = false;
}


inline
void Timer::reset()
{
    m_time_elapsed = 0.0;
    m_is_running   = false;
}


inline
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


#endif // CCUP_TIMER_H__
