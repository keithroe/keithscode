
#include "Timer.h"


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
    : m_start_ticks( getTicks() )
{
}


Timer::~Timer()
{
}


void Timer::reset()
{
    m_start_ticks = getTicks();
}


double Timer::getTimeElapsed()const
{
    const Ticks current_ticks = getTicks();
    return ( current_ticks - m_start_ticks ) * secondsPerTick();
}


double Timer::getTimeElapsedMilliSecond()const
{
    return getTimeElapsed() * 1.0e3;
}


double Timer::getTimeElapsedMicroSecond()const
{
    return getTimeElapsed() * 1.0e6;
}
