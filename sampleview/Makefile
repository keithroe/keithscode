
#CXX_FLAGS = -lGL -lglut
CXX_FLAGS = -framework OpenGL -framework glut

all: sampleview samplegenerate
sampleview: sampleview.cpp
	g++ -g -o sampleview sampleview.cpp $(CXX_FLAGS)

samplegenerate: samplegenerate.cpp
	g++ -g -o samplegenerate samplegenerate.cpp
