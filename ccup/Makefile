
all: kplayer



kplayer: main.cpp
	g++ -Wall -O2 -g -lm -o kplayer main.cpp
	#g++ -DLOCAL -Wall -g -lm -o kplayer main.cpp
	cp kplayer ~/caia/symple/bin/

.PHONY: clean
clean:
	rm -f kplayer


