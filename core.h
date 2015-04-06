/*
	The Castle - strategická hra
	Bedrich Pisl, I. rocnik
	Letni semestr 2013
	Programovani II.
*/
	
/* This is major header file, which includes all other header files.*/

#ifndef core_bedas_guard
#define core_bedas_guard

#define VERSION 0.2

#if defined(unix) || defined(__unix__) || defined(__unix)
	#define UNIX
#endif

#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>
#include <iostream>
#include <ctime>
#ifdef UNIX
	#include <sys/time.h>
#endif
#include <math.h>
#include <fstream>
#include <string>
#include <vector>
#include <queue>
#include <algorithm>
#include <limits>
#include <queue>
#include <cstdio>
#include <boost/archive/tmpdir.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
//#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

#define LOG(text) (log_file << __FILE__ << ", line " << __LINE__ << " " << __func__ << "(): " << text << std::endl)
		//instead of "text" there can be any variable, which has << operator, __FILE__, __LINE__ and __func__ are special things provided by compiler, example: LOG("ahoj"); 
#define BUTTON_SIZE 80

#include "enum.h"
#include "constant_database.h"
#include "parsing.h"
#include "window.h"
#include "loop.h"
#include "main.h"
#include "object.h"
#include "tile.h"
#include "others.h"
#include "building.h"
#include "button.h"
#include "people.h"
#include "mouse.h"
#include "ai.h"
#include "session.h"
#include "pathfinding.h"
#include "game_functions.h"

#define BLACK_COLOR al_map_rgb(0,0,0)
#define WHITE_COLOR al_map_rgb(255, 255, 255)
#define LIGHT_GREEN_COLOR al_map_rgb(50, 255, 50)
#define LIGHT_RED_COLOR al_map_rgb(255, 50, 50)
#define LIGHT_BLUE_COLOR al_map_rgb(50, 50, 255)
#define LIGHT_GREY_COLOR al_map_rgb(200, 200, 200)
#define BLUE_COLOR al_map_rgb(50, 50, 200)
#define RED_COLOR al_map_rgb(200, 50, 50)
#define GREEN_COLOR al_map_rgb(50, 200, 50)
#define YELLOW_COLOR al_map_rgb(255, 255, 50)
#define DARK_RED_COLOR al_map_rgb(150, 50, 50)
#define DARK_YELLOW_COLOR al_map_rgb(150, 150, 50)
#define DARK_GREEN_COLOR al_map_rgb(50, 150, 50)
#define DARK_GREY_COLOR al_map_rgb(100, 100, 100)
#define VERY_DARK_GREY_COLOR al_map_rgb(50, 50, 50)
#define WRITING_COLOR al_map_rgb(200, 200, 200)
#define GREY_COLOR al_map_rgb(150, 150, 150)
#define BACKGROUND_COLOR al_map_rgb(100, 100, 100)
#define MAP_WIDTH 70 //in tiles not pixels
#define MAP_HEIGHT 70 //big increasing of this can cause problems, because current A* needs about (20*n)^2 memory (A* can be overwritten to use less memory...)
#define FPS 60							// if FPS goes under 10 or something like that, mouse_button_down should have been invoked in main loop by allegro queue 
#define DISPLAY_MOVEMENT_SPEED (13.0/(game_info::fps/60.0))
//#define SOCHL 4							//Speed Of CHanging Legs when drawing moving people 
#define MAX_NUMBER_OF_CHOSEN_PEOPLE 128
#define MAX_PEOPLE 4096
#define MAX_MISSILES 2048
#define MARKED_LOCATION	8				//udava jak vzdaleny muze byt drawing_x jednotky od vybraneho bodu, aby byla jednotka jeste vybrana 
//#define LENGHT_OF_BUILDING_ACTION_QUEUE 10

#endif
