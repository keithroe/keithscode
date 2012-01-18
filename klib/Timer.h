
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

#endif // KLIB_TIMER_H_
