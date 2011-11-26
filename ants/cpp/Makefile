CC=g++
CFLAGS= -O3 -funroll-loops -Wall -Werror
LDFLAGS= -lm

HEADERS= Ant.h \
         AStar.h \
		 Battle.h \
         BF.h \
         BFS.h \
         Bot.h \
         Debug.h \
         Direction.h \
         Location.h \
         Map.h \
         Path.h \
         PathFinder.h \
         Square.h \
         State.h \
         Timer.h

SOURCES= AStar.cc \
		 Battle.cc \
         BFS.cc \
         Bot.cc \
         Location.cc \
         Map.cc \
         Path.cc \
         PathFinder.cc \
         State.cc

OBJECTS=$(SOURCES:.cc=.o)
MYBOT=MyBot
ASTARTEST=astartest
BFSTEST=bfstest
DIFFTEST=difftest

#Uncomment the following to enable debugging
#CFLAGS += -g -DDEBUG
#CFLAGS = -g -DDEBUG

all: $(OBJECTS) $(MYBOT) $(ASTARTEST) $(BFSTEST) $(DIFFTEST)

$(MYBOT): MyBot.o $(OBJECTS)  $(HEADERS) 
	$(CC)  $(CFLAGS) $(LDFLAGS) MyBot.o $(OBJECTS) -o $@
	cp MyBot ..

$(ASTARTEST): AStarTest.o $(OBJECTS) $(HEADERS) 
	$(CC) $(LDFLAGS) AStarTest.o $(OBJECTS) -o $@

$(BFSTEST): BFSTest.o $(OBJECTS) $(HEADERS) 
	$(CC) $(LDFLAGS) BFSTest.o $(OBJECTS) -o $@

$(DIFFTEST): DiffusionTest.o $(OBJECTS) $(HEADERS) 
	$(CC) $(LDFLAGS) DiffusionTest.o $(OBJECTS) -o $@

%.o : %.cc $(HEADERS) 
	$(CC) -c $(CFLAGS) $< -o $@

clean: 
	-rm -f ${EXECUTABLE} MyBot astartest bfstest difftest AStarTest.o MyBot.o ${OBJECTS} *.d
	-rm -f debug.txt

zip:
	-make clean
	-rm -rf ~/Desktop/cpp/
	-rm -f ~/Desktop/cpp.zip
	-cp -r ../cpp ~/Desktop
	-rm ~/Desktop/cpp/*Test*

.PHONY: all clean zip

