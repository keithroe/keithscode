CC=g++
CFLAGS=-O3 -funroll-loops -c -DDEBUG -I.
LDFLAGS=-O2 -lm
HEADERS= Bot.h Location.h State.h Bug.h Square.h Timer.h PathFinder.h

SOURCES=Bot.cc MyBot.cc PathFinder.cc State.cc
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

.PHONY: all clean

