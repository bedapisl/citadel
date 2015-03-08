CC=g++
CFLAGS= -std=c++11 -ggdb -ftemplate-depth=1024 -pedantic -Wall -Wno-sign-compare	#-lprofiler
							#clang has low default template depth
LDFLAGS=
				#allegro headers are in /usr/include/allegro5

all: ai.o building.o button.o game_functions.o loop.o main.o mouse.o object.o others.o parsing.o pathfinding.o people.o session.o tile.o window.o
	$(CC) ai.o building.o button.o game_functions.o loop.o main.o mouse.o object.o others.o parsing.o pathfinding.o people.o session.o window.o tile.o $(LDFLAGS)  -lallegro_acodec -lallegro_audio -lallegro_color -lallegro_dialog -lallegro_font -lallegro_ttf -lallegro_image -lallegro_primitives -lallegro_main -lallegro -lboost_iostreams -lboost_serialization -lboost_system -lboost_filesystem

ARCHITECTURE=$(shell uname -m | sed 's@x86_64@64@' | sed 's@i686@32@')
BINARY_NAME=citadel_$(shell $(shell pwd)/citadel.out -v)_$(ARCHITECTURE)bit
				
				#create distributable binaries. Needs up-to-date 'a.out' file in this directory.
binaries: ai.o building.o button.o game_functions.o loop.o main.o mouse.o object.o others.o parsing.o pathfinding.o people.o session.o tile.o window.o 
	$(CC) ai.o building.o button.o game_functions.o loop.o main.o mouse.o object.o others.o parsing.o pathfinding.o people.o session.o window.o tile.o $(LDFLAGS) -Wl,-rpath=\$$ORIGIN/lib$(ARCHITECTURE)/ -lallegro_acodec -lallegro_audio -lallegro_color -lallegro_dialog -lallegro_font -lallegro_ttf -lallegro_image -lallegro_primitives -lallegro_main -lallegro -lboost_iostreams -lboost_serialization -lboost_system -lboost_filesystem -o citadel.out
	cp citadel.out $(BINARY_NAME).out
	zip -r $(BINARY_NAME).zip $(BINARY_NAME).out data lib$(ARCHITECTURE)
	echo -e "\n Don't forget VERSION"

ai.o: ai.cpp $(HEADERS)
	$(CC) $(CFLAGS) -c ai.cpp

building.o: building.cpp $(HEADERS)
	$(CC) $(CFLAGS) -c building.cpp
 
button.o: button.cpp $(HEADERS)
	$(CC) $(CFLAGS) -c button.cpp
 
game_functions.o: game_functions.cpp $(HEADERS)
	$(CC) $(CFLAGS) -c game_functions.cpp

loop.o: loop.cpp $(HEADERS)
	$(CC) $(CFLAGS) -c loop.cpp

main.o: main.cpp $(HEADERS)
	$(CC) $(CFLAGS) -c main.cpp

mouse.o: mouse.cpp $(HEADERS)
	$(CC) $(CFLAGS) -c mouse.cpp
 
object.o: object.cpp $(HEADERS)
	$(CC) $(CFLAGS) -c object.cpp
 
others.o: others.cpp $(HEADERS)
	$(CC) $(CFLAGS) -c others.cpp
 
parsing.o: parsing.cpp $(HEADERS)
	$(CC) $(CFLAGS) -c parsing.cpp

pathfinding.o: pathfinding.cpp $(HEADERS)
	$(CC) $(CFLAGS) -c pathfinding.cpp

people.o: people.cpp $(HEADERS)
	$(CC) $(CFLAGS) -c people.cpp
  
session.o: session.cpp $(HEADERS)
	$(CC) $(CFLAGS) -c session.cpp

tile.o: tile.cpp $(HEADERS)
	$(CC) $(CFLAGS) -c tile.cpp

window.o: window.cpp $(HEADERS)
	$(CC) $(CFLAGS) -c window.cpp

clean:
	rm *.o a.out

