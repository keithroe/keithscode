
#ifndef KLIB_TIMER_H_
#define KLIB_TIMER_H_


///
/// Simple timer class which allows client to keep track of elapsed time
///
class Timer
{
public:
    typedef unsigned long long Ticks;

    /// Initializes timer to current time
    Timer();

    ~Timer();

    /// Resets start time to current time
    void reset();

    /// Get time elapsed in seconds
    double getTimeElapsed()const;

    /// Get time elapsed in milliseconds 
    double getTimeElapsedMilliSecond()const;

    /// Get time elapsed in microseconds 
    double getTimeElapsedMicroSecond()const;
    
private:


    // TODO: use mach_absolute_time on mac
    Ticks m_start_ticks;
};


///
/// Calls client provided callback upon destruction, passing in time elapsed
/// since AutoTimer creation
///
template <typename Action>
class AutoTimer
{
public:
    AutoTimer( Action action )
        : m_action( action ) {}

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
    AutoTimerRef( Action& action )
        : m_action( action ) {}

    ~AutoTimerRef()
    { m_action( m_timer.getTimeElapsed() ); }

private:
    Action& m_action;
    Timer   m_timer;
};

#endif // KLIB_TIMER_H_
