#include "core.h"

int game_object::highest_surface = 0;

extern std::ofstream log_file;

game_object::game_object(int tile_x, int tile_y, int surface_height, bool is_real, picture image, int number_of_floors, game_object_type type_of_object)
				: draw_green(false), type_of_object(type_of_object), number_of_floors(number_of_floors), is_real(is_real), image(image) 
{	
	update(tile_x, tile_y, surface_height);
	drawing_floor = 0;
}

void game_object::update(int tile_x, int tile_y, int surface_height)
{
	if(((tile_x >= game_info::map_width) || (tile_y >= game_info::map_height)) || ((tile_x < 0) || (tile_y < 0)))
	{	
		LOG("error - game_object update: tile_x cannot be " << tile_x << " or tile_y cannot be " << tile_y);
		throw std::exception();
	}

	this->tile_x = tile_x;
	this->tile_y = tile_y;
	game_x = compute_game_x(tile_x, tile_y);
	game_y = compute_game_y(tile_x, tile_y);

	
	set_surface_height(surface_height);
}

void game_object::set_surface_height(int height)
{
	this->surface_height = height;
	
	if(surface_height > highest_surface)
		highest_surface = surface_height;
	
}


