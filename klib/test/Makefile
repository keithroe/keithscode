
CXX_FLAGS= -O3 -g -Werror -Wall -I..
CXX_FLAGS+= -DKLOG_MAX_LEVEL=Log::INFO

all: astar timer
	
astar: ../AStar.h ../Logger.h ../Timer.cc ../Timer.h astar.cc
	g++ $(CXX_FLAGS) astar.cc ../Timer.cc -o astar 

timer: ../Timer.h ../Timer.cc timer.cc
	g++ $(CXX_FLAGS) timer.cc ../Timer.cc -o timer 

clean:
	rm -rf *.dSYM
	rm astar
