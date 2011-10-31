CC=g++
#CFLAGS=-O3 -funroll-loops -c  -DDEBUG
CFLAGS=-g -funroll-loops -c  -DDEBUG
LDFLAGS=-O2 -lm

HEADERS= Ant.h \
         AStar.h \
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

#Uncomment the following to enable debugging
#CFLAGS+=-g -DDEBUG

all: $(OBJECTS) $(MYBOT) $(ASTARTEST)

$(MYBOT): MyBot.o $(OBJECTS)  $(HEADERS) 
	$(CC) $(LDFLAGS) MyBot.o $(OBJECTS) -o $@

$(ASTARTEST): AStarTest.o $(OBJECTS) $(HEADERS) 
	$(CC) $(LDFLAGS) AStarTest.o $(OBJECTS) -o $@

%.o : %.cc $(HEADERS) 
	$(CC) $(CFLAGS) $< -o $@

clean: 
	-rm -f ${EXECUTABLE} MyBot astartest AStarTest.o MyBot.o ${OBJECTS} *.d
	-rm -f debug.txt

zip:
	-make clean
	-rm -rf ~/Desktop/cpp/
	-rm -f ~/Desktop/cpp.zip
	-cp -r ../cpp ~/Desktop

.PHONY: all clean zip

