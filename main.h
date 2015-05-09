/*
	The Citadel - strategic game
	Bedrich Pisl
*/

/**
 * \file main.h
 * \brief Entry point and functions mainly for loading resources and initilization.
 */

//dont use function stoi() from standard library - it doesnt work with MinGW

#ifndef main_h_bedas_guard
#define main_h_bedas_guard

#include <boost/serialization/base_object.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/assume_abstract.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/weak_ptr.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/export.hpp>
#include <boost/filesystem.hpp>

class game_info;

void create_display();

bool init_allegro();		///< Initializes all parts of Allegro library. Returns false when something fails.

bool load_pictures();		///< Loads all pictures in global variable image_list. If some pictures can't be loaded, creates image with text "No image" and puts it in image list. Returns false if some images were not loaded.

void transform_bitmaps_to_atlas();	///< Creates one big image, draws all other images to it and changes image_list accordingly. Is needed for performance reasons.

int delete_pictures();		///< Destroys image_list.

bool load_fonts();		///< Tries loading fonts. Returns false if loading fails.

int delete_fonts();		///< Destroys fonts.

int pow(int a, int b);		///< Mathematical power function.

int load_settings();		///< Loads contents of data/config.txt file in game_info class.


#endif
