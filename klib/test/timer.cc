
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
    Timer global_timer;
    global_timer.start();

    unsigned long long total_sleep_ms = 0;
    double loop_time_elapsed = 0.0;

    Timer outer_timer;
    AccumulateElapsed ae;
    for( int i = 0; i < 100; ++i )
    {
        AutoTimerRef<AccumulateElapsed> auto_timer( ae );
        outer_timer.start();

        Timer inner_timer;
        inner_timer.start();

        usleep( 10000 );
        total_sleep_ms += 10000;

        loop_time_elapsed += inner_timer.getTimeElapsed();

        outer_timer.stop();
    }

    global_timer.stop();


    std::cerr << "global_timer: " << global_timer.getTimeElapsed() << std::endl;
    std::cerr << "auto_timer  : " << ae.accumulated_time_elapsed << std::endl;
    std::cerr << "outer_timer : " << outer_timer.getTimeElapsed() << std::endl;
    std::cerr << "inner_timer : " << loop_time_elapsed  << std::endl;
}
