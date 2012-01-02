
CXX_FLAGS= -O3 -g -Werror -Wall -I..
CXX_FLAGS+= -DKLOG_MAX_LEVEL=Log::INFO

astar: ../AStar.h ../Logger.h ../Timer.cc ../Timer.h astar.cc
	g++ $(CXX_FLAGS) astar.cc ../Timer.cc -o astar 


clean:
	rm -rf *.dSYM
	rm astar
