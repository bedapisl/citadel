/*
	Citadel - strategic game
	Bedrich Pisl - part of bachelor thesis
	2015
*/

/**
 * \file core.h 
 * \brief This is major header file, which includes all other header files and contains some macros.
 *
 */

#ifndef core_bedas_guard
#define core_bedas_guard

#define VERSION 0.3

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
#include <chrono>
#include <boost/archive/tmpdir.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

//Instead of "text" there can be any variable, which has << operator, __FILE__, __LINE__ and __func__ are special things provided by compiler, example: LOG("ahoj"); 
//Writes info to file called log.
#define LOG(text) (log_file << __FILE__ << ", line " << __LINE__ << " " << __func__ << "(): " << text << std::endl)
#define BUTTON_SIZE 80		///Height of main panel in game. In pixels.

#include "enum.h"
#include "constant_database.h"
#include "parsing.h"
#include "window.h"
#include "loop.h"
#include "main.h"
#include "object.h"
#include "tile.h"
#include "map_generator.h"
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
			// Following two macros shouldnt be used. Use game_info::map_width and game_info::map_height instead.
#define MAP_WIDTH 100 	//in tiles not pixels
#define MAP_HEIGHT 100 	//big increasing of this can cause problems, because current A* needs about (20*n)^2 memory (A* can be overwritten to use less memory...)

#define DISPLAY_MOVEMENT_SPEED (13.0/(game_info::fps/60.0))
#define MARKED_LOCATION	8				//How far away from unit can the player click to select the unit. In pixels.

#endif
