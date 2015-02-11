CFLAGS= -ggdb -std=c++11	#-pedantic -Wall#-lprofiler
LDFLAGS=
				#allegro headers are in /usr/include/allegro5
STATIC_LIBRARIES=-lboost_iostreams -lboost_serialization -lboost_system -lboost_filesystem `pkg-config --libs --static allegro-static-5 allegro_acodec-static-5 allegro_audio-static-5 allegro_color-static-5 allegro_dialog-static-5 allegro_font-static-5 allegro_ttf-static-5 allegro_image-static-5 allegro_primitives-static-5 allegro_main-static-5`

all: clear ai.o building.o button.o game_functions.o loop.o main.o mouse.o object.o others.o parsing.o pathfinding.o people.o session.o tile.o window.o
	echo -e '\n\n\n\n'
	g++ ai.o building.o button.o game_functions.o loop.o main.o mouse.o object.o others.o parsing.o pathfinding.o people.o session.o window.o tile.o $(LDFLAGS)  -lallegro_acodec -lallegro_audio -lallegro_color -lallegro_dialog -lallegro_font -lallegro_ttf -lallegro_image -lallegro_primitives -lallegro_main -lallegro -lboost_iostreams -lboost_serialization -lboost_system -lboost_filesystem 

static: ai.o building.o button.o game_functions.o loop.o main.o mouse.o object.o others.o parsing.o pathfinding.o people.o session.o tile.o window.o
	echo -e '\n\n\n\n'
	g++ ai.o building.o button.o game_functions.o loop.o main.o mouse.o object.o others.o parsing.o pathfinding.o people.o session.o window.o tile.o $(LDFLAGS) $(STATIC_LIBRARIES) 

clear:
	echo -e '\n\n\n\n'

ai.o: ai.cpp $(HEADERS)
	g++ $(CFLAGS) -c ai.cpp

building.o: building.cpp $(HEADERS)
	g++ $(CFLAGS) -c building.cpp
 
button.o: button.cpp $(HEADERS)
	g++ $(CFLAGS) -c button.cpp
 
game_functions.o: game_functions.cpp $(HEADERS)
	g++ $(CFLAGS) -c game_functions.cpp

loop.o: loop.cpp $(HEADERS)
	g++ $(CFLAGS) -c loop.cpp

main.o: main.cpp $(HEADERS)
	g++ $(CFLAGS) -c main.cpp

mouse.o: mouse.cpp $(HEADERS)
	g++ $(CFLAGS) -c mouse.cpp
 
object.o: object.cpp $(HEADERS)
	g++ $(CFLAGS) -c object.cpp
 
others.o: others.cpp $(HEADERS)
	g++ $(CFLAGS) -c others.cpp
 
parsing.o: parsing.cpp $(HEADERS)
	g++ $(CFLAGS) -c parsing.cpp

pathfinding.o: pathfinding.cpp $(HEADERS)
	g++ $(CFLAGS) -c pathfinding.cpp

people.o: people.cpp $(HEADERS)
	g++ $(CFLAGS) -c people.cpp
  
session.o: session.cpp $(HEADERS)
	g++ $(CFLAGS) -c session.cpp

tile.o: tile.cpp $(HEADERS)
	g++ $(CFLAGS) -c tile.cpp

window.o: window.cpp $(HEADERS)
	g++ $(CFLAGS) -c window.cpp

clean:
	rm *.o a.out

