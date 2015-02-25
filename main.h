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

void create_display();

bool init_allegro();

bool load_pictures();

int delete_pictures();

bool load_fonts();

int delete_fonts();

int pow(int a, int b);

int resize_display(ALLEGRO_EVENT* ev);

int load_settings();

int lower_case(std::string& word);

std::vector<std::string> split(std::string line);

void remove_spaces_and_apostrophs(std::string& word);

#endif
