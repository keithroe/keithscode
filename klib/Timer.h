
#ifndef KLIB_TIMER_H_
#define KLIB_TIMER_H_


///
/// Simple timer class which allows client to keep track of elapsed time within
/// a program.  Mimics stopwatch functionality
///
class Timer
{
public:
    typedef unsigned long long Ticks;

    /// Initialize timer
    explicit Timer();

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

///
/// Convert seconds to milliseconds
///
double secondsToMilliseconds( double seconds );

///
/// Convert seconds to microseconds
///
double secondsToMicroseconds( double seconds );


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
