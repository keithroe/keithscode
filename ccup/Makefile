
CXX=g++
#CXXFLAGS=-Wall -O2 -g 
CXXFLAGS=-Wall  -g  -DLOCAL
#CXXFLAGS=-Wall -O2 -g -DLOCAL
LDFLAGS=-lm

HEADERS=src/AI.h \
		src/Board.h \
		src/Logger.h \
		src/MCTSAI.h \
		src/Player.h \
		src/Point.h \
		src/RandomAI.h \
		src/Timer.h \
		src/Util.h

SRCS=src/Board.cpp \
	 src/Logger.cpp \
	 src/MCTSAI.cpp \
	 src/Player.cpp \
	 src/RandomAI.cpp \
	 src/Util.cpp \
	 src/main.cpp

OBJS=$(patsubst src/%.cpp,obj/%.o,$(SRCS))



#all: kplayer kplayer_profile
all: kplayer 

obj:
	@mkdir -p $@

obj/%.o: src/%.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

kplayer: obj $(OBJS) 
	$(CXX) $(LDFLAGS) -g -o kplayer $(OBJS)
	#g++ -DLOCAL -Wall -g -lm -o kplayer_debug  main.cpp
	cp kplayer ~/caia/symple/bin/

kplayer_profile: $(HEADERS) $(SRCS)
	$(CXX) $(LDFLAGS) $(CXXFLAGS) $(SRCS) -o kplayer_profile

.PHONY: clean
clean:
	rm -rf kplayer kplayer_profile kplayer_profile.dSYM $(OBJS)


