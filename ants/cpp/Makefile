CC=g++
CFLAGS=-O3 -funroll-loops -c -DDEBUG 
LDFLAGS=-O2 -lm

HEADERS= AStar.h \
		 Bot.h \
         Bug.h \
		 Direction.h \
         Location.h \
		 Map.h \
         Path.h \
         PathFinder.h \
         Square.h \
         State.h \
         Timer.h

SOURCES= AStar.cc \
		 Bot.cc \
		 Map.cc \
         MyBot.cc \
         Path.cc \
         PathFinder.cc \
         State.cc

OBJECTS=$(SOURCES:.cc=.o)
EXECUTABLE=MyBot

#Uncomment the following to enable debugging
#CFLAGS+=-g -DDEBUG

all: $(OBJECTS) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS) $(HEADERS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

%.o : %.cc $(HEADERS) 
	$(CC) $(CFLAGS) $< -o $@

clean: 
	-rm -f ${EXECUTABLE} ${OBJECTS} *.d
	-rm -f debug.txt

zip:
	-make clean
	-rm -rf ~/Desktop/cpp/
	-rm -f ~/Desktop/cpp.zip
	-cp -r ../cpp ~/Desktop

.PHONY: all clean zip

