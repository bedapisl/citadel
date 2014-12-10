/*
	The Citadel - strategická hra
	Bedrich Pisl
*/

//dont use function stoi() from standard library - it doesnt work with MinGW

#ifndef main_h
#define main_h

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

//int game_loop();	//is in loop.cpp

//int main_menu();

//int draw_main_menu(main_menu_option chosen_option);

//int settings(ALLEGRO_EVENT_QUEUE* queue, ALLEGRO_EVENT* ev);

//int draw_settings(int option_focus, std::string text, int fullscreen_option, int display_width_option, int display_height_option, int fps_option);

//void save_settings(bool fullscreen_option, int display_width_option, int display_height_option, int fps_option);

void create_display();

//void start_playing_sound();

bool init_allegro();

bool load_pictures();

int delete_pictures();

bool load_fonts();

int delete_fonts();

int pow(int a, int b);

//int message(const char* text);

int resize_display(ALLEGRO_EVENT* ev);

int load_settings();

int string_to_int(const std::string& word);

int lower_case(std::string& word);

std::vector<std::string> split(std::string line);

void remove_spaces_and_apostrophs(std::string& word);

//void save_window(bool& display_closed);

#endif
