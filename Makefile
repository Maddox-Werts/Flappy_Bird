# Variables
LIBS:= -lSDL2main -lSDL2 -lSDL2_mixer -lSDL2_image

# Instructions
all: compile link

compile:
	g++ -c -I include source/*.cpp

link:
	g++ *.o -L lib $(LIBS) -o debug/flappybird