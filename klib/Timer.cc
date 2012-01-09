
#include "Timer.h"
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


double secondsToMilliseconds( double seconds )
{
    return seconds * 1.0e3;
}


double secondsToMicroseconds( double seconds )
{
    return seconds * 1.0e6;
}
