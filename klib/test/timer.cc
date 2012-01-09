
#include <iostream>
#include <unistd.h>
#include "../Timer.h"

struct AccumulateElapsed
{
    AccumulateElapsed() : accumulated_time_elapsed( 0.0 ) {}

    void operator()( double time_elapsed )
    { accumulated_time_elapsed += time_elapsed; }

    double accumulated_time_elapsed;
};


int main( int argc, char** argv )
{
    // global_timer  will stay on for the entire test
    Timer global_timer;
    global_timer.start();

    // outer_timer will be turned off and on within each loop iteration
    Timer outer_timer;
    
    // This var stores accumulated results from loop scope timer, inner_timer 
    double inner_timer_accumulated_time = 0.0;

    // This struct stores accumulated timer from AutoTimerRef within loop
    AccumulateElapsed ae;

    for( int i = 0; i < 100; ++i )
    {
        // Create our auto timer
        AutoTimerRef<AccumulateElapsed> auto_timer( ae );

        // Start the outer timer
        outer_timer.start();

        // Create and start the inner timer
        Timer inner_timer;
        inner_timer.start();

        usleep( 10000 );

        // accumulate from inner timer
        inner_timer_accumulated_time += inner_timer.getTimeElapsed();

        // pause our outer_timer
        outer_timer.stop();

        // lastly, auto timer goes out of scope and invokes callback
    }

    // These should all be VERY similar :)
    std::cout << "global_timer: " << global_timer.getTimeElapsed() << std::endl
              << "auto_timer  : " << ae.accumulated_time_elapsed   << std::endl
              << "outer_timer : " << outer_timer.getTimeElapsed()  << std::endl
              << "inner_timer : " << inner_timer_accumulated_time  << std::endl;
}
