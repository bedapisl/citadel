#include "core.h"

extern std::ofstream log_file;
extern ALLEGRO_BITMAP** image_list;
extern game_session* session;

std::vector<tile*> tile::minimap_updates;

tile::tile(tile_type type, int tile_x, int tile_y, int surface_height) : game_object(tile_x, tile_y, surface_height, true, NO_IMAGE, 0, TILE)
{
	game_object::update(tile_x, tile_y, surface_height);
	this->type = type;
	visible = true;
	number_of_tile_with_action = 0;
	path_on_tile = false;
	bIs_path_real = false;
	draw_building = false;
	can_go_inside_building = false;
	can_go_on_building = false;
	stairs_on_tile = false;
	tile_object_image = NO_IMAGE;
	if(type != WATER)
		image = GRASS_IMAGE;

	else
		image = WATER_IMAGE;

	object = NOTHING;
	drawing_floor = 0;
	fertile = false;
	
	if(show_surface_height() == 0)
		border = GROUND_LEVEL;
	if(show_surface_height() > 0)
		border = NO_BORDERS;
	
	path_border = UNKNOWN_BORDERS;
}

std::vector<game_object*> tile::draw(int screen_position_x, int screen_position_y)
{
	int drawing_x = game_x - screen_position_x - 32;
	int drawing_y = game_y - screen_position_y - show_surface_height()*32 - 16;
	
	std::vector<game_object*> objects;

	if((type == GRASS) && (drawing_floor == 0))
	{
		al_draw_bitmap_region(image_list[image], 64*border, 0, 64, 64, drawing_x, drawing_y, 0);
	}
	else if(type == RAMP)
	{
		al_draw_bitmap_region(image_list[image], 64*border, 64, 64, 64, drawing_x, drawing_y, 0);
	}
	else if(type == WATER)
	{
		std::vector<std::pair<int, int>> adjacent_tiles{std::pair<int, int>(-1, 0), std::pair<int, int>(-1, -1), std::pair<int, int>(0, -1), std::pair<int, int>(1, -1), 
							std::pair<int, int>(1, 0), std::pair<int, int>(1, 1), std::pair<int, int>(0, 1), std::pair<int, int>(-1, 1), };
		
		int image_number = 0;
		for(int i=0; i<adjacent_tiles.size(); ++i)
		{
			int x = adjacent_tiles[i].first + tile_x;
			int y = adjacent_tiles[i].second + tile_y;

			if((x >= 0) && (x < game_info::map_width) && (y >= 0) && (y < game_info::map_height))
			{
				if((!session->tile_list[y][x]->is_water_tile()) 
							|| (session->tile_list[y][x]->show_surface_height() != show_surface_height()))
				{
					image_number += pow(2, i);
				}
			}
		}				
		al_draw_bitmap_region(image_list[image], 64*border, 0, 64, 64, drawing_x, drawing_y, 0);
		al_draw_bitmap_region(image_list[WATER_IMAGE], 64* (image_number % 16), 64* (image_number / 16), 64, 64, drawing_x, drawing_y, 0);
	}

	if((fertile) && drawing_floor == 0)
		al_draw_bitmap(image_list[FERTILE_IMAGE], drawing_x, drawing_y - 32, 0);
	
	if(real_path_on_tile())
	{
		if(type == RAMP)
			al_draw_bitmap_region(image_list[PATHS_IMAGE], 64*path_border, 64, 64, 64, drawing_x, drawing_y, 0);

		else 
			al_draw_bitmap_region(image_list[PATHS_IMAGE], 64*path_border, 0, 64, 64, drawing_x, drawing_y, 0);
	}
	
	else if(unreal_path_on_tile())
	{
		if(action_on_tile.expired())
		{
			path_on_tile = false;
		}

		if(type == RAMP)
			al_draw_tinted_bitmap_region(image_list[PATHS_IMAGE], LIGHT_GREEN_COLOR, 64*path_border, 64, 64, 64, drawing_x, drawing_y, 0);

		else 
			al_draw_tinted_bitmap_region(image_list[PATHS_IMAGE], LIGHT_GREEN_COLOR, 64*path_border, 0, 64, 64, drawing_x, drawing_y, 0);
	}

	if(!action_on_tile.expired())
	{
		game_object* object_to_draw = action_on_tile.lock()->draw(tile_x, tile_y, show_surface_height() + 1, number_of_tile_with_action);
		action_on_tile.reset();
		if(object_to_draw != nullptr)
		{
			objects.push_back(object_to_draw);
		}
	}
	
	check_death_people_on_tile();
	for(int i=0; i<people_on_tile.size(); ++i)
	{
		bool drawn_by_building = false;
		if(!building_on_tile.expired())
		{
			switch(building_on_tile.lock()->type)
			{
				case(PALISADE):
				case(WALL):
				{
					boost::shared_ptr<wall> wall_ptr = boost::dynamic_pointer_cast<wall>(building_on_tile.lock());
					wall_ptr->add_people_to_draw(people_on_tile[i]);
					drawn_by_building = true;
				}
				break;
				case(SOUTHWEST_TOWER):
				case(NORTHWEST_TOWER):
				case(NORTHEAST_TOWER):
				case(SOUTHEAST_TOWER):
				{
					boost::shared_ptr<tower> tower_ptr = boost::dynamic_pointer_cast<tower>(building_on_tile.lock());
					tower_ptr->add_people_to_draw(people_on_tile[i]);
					drawn_by_building = true;
				}
				break;
				default:
					{ }
			}
		}
		
		if(!drawn_by_building)
			objects.push_back(people_on_tile[i].lock().get());
	}
	
	if(draw_building)
	{
		if(building_on_tile.expired())
		{
			LOG("error");
			throw new std::exception;
		}
		objects.push_back(building_on_tile.lock().get());
	}
	
	if(object == TREE_TILE)//bTree_on_tile)				//tree has 4 floors, so at first time, this function draws only grass and returns tile. 
	{									//Then, when drawing_floor is 1-3, it draws tree. Same system as drawing buildings.
		if(drawing_floor == 0)
		{
			if(objects.size() == 0)
			{
				objects.push_back(this);
				drawing_floor++;
			}
		}
		else if(drawing_floor <= number_of_floors)
		{
			drawing_y -= 96;

			al_draw_bitmap_region(image_list[tile_object_image], drawing_floor*64 - 64, 0, 64, 128, drawing_x, drawing_y, 0); 

			if(drawing_floor < number_of_floors)
			{
				if(objects.size() == 0)
				{
					objects.push_back(this);
					drawing_floor++;
				}
				else
					throw new std::exception;
			}
			else
			{
				drawing_floor = 0;
			}
		}
	}
	
	if((object == MARBLE_TILE) || (object == IRON_TILE) || (object == COAL_TILE) || (object == GOLD_TILE))
	{
		if(drawing_floor == 0)
		{
			if(objects.size() == 0)
			{
				objects.push_back(this);
				drawing_floor++;
			}
		}
		else if(drawing_floor == 1)
		{
			int start_x = (static_cast<int>(object) - static_cast<int>(IRON_TILE))*64;
			al_draw_bitmap_region(image_list[tile_object_image], start_x, 0, 64, 64, drawing_x, drawing_y - 32, 0);

			drawing_floor = 0;
		}
	}

	for(int i=0; i<objects.size(); ++i)
	{
		if(objects[i] == NULL)
			throw new std::exception;
	}

	return objects;
}

void tile::build(building_type type, player owner)
{
	/*
	for(int i=0; i<NUMBER_OF_RESOURCES; ++i)		//check if player has enough resources
	{
		resources resource_type = static_cast<resources>(i);
		if(show_building_price(type, resource_type, NO_UPGRADE) > session->global_stock->show_amount(resource_type))
		{
			LOG("Not enough resources");
			return 0;
		}
	}
	*/
	for(int i=0; i<NUMBER_OF_RESOURCES; ++i)		//subtract resources
	{
		resources resource_type = static_cast<resources>(i);
		session->global_stock->subtract(resource_type, show_building_price(type, resource_type, NO_UPGRADE));
	}
	
	building_size size = building_info::show_building_info(type).size;
	if(type == RIGHT_GATE)
		size = RIGHT_GATE_BUILDING;		//left and right gate has same building info with "size = LEFT_GATE_BUILDING", which is wrong for right gate 
	
	std::vector<tile*> build_here = tiles_under_building(tile_x, tile_y, size);
	
	boost::shared_ptr<building> new_building = building::create_building(type, tile_x, tile_y, show_surface_height() + 1, owner, true);

	for(unsigned int i=0; i<build_here.size(); i++)
	{
		build_here[i]->building_on_tile = new_building;
		
		if(!build_here[i]->can_go_inside_building)
			build_here[i]->remove_path(true);

		tile::minimap_updates.push_back(build_here[i]);
	}

	new_building->set_drawing_tile();
	session->building_list.push_back(new_building);
}

boost::weak_ptr<warrior> tile::warrior_born(people_type type, player owner)
{
	LOG("people born - x " << tile_x << " y " << tile_y); 
	if(are_people_on_tile())
	{
		LOG("error - tile is not empty");
		std::cout << "impossible to make new unit - tile is not empty" << std::endl;
		return boost::weak_ptr<warrior>();
	}
	
	boost::shared_ptr<warrior> created_warrior(new warrior(type, tile_x, tile_y, show_surface_height() + 1, owner));
	session->people_list.push_back(boost::shared_ptr<people>(created_warrior));
	
	people_on_tile.push_back(session->people_list.back());

	LOG("number of people: " << session->people_list.size());

	tile::minimap_updates.push_back(this);

	return created_warrior;
}

int tile::people_die()		//this should be called only by people::die function
{
	tile::minimap_updates.push_back(this);

	return 0;
}

int tile::people_come(boost::shared_ptr<people> people_which_comes)
{
	people_on_tile.push_back(people_which_comes);
	tile::minimap_updates.push_back(this);
	return 0;
}

int tile::people_leave(boost::shared_ptr<people> people_which_leaves)
{
	check_death_people_on_tile();
	for(int i=0; i<people_on_tile.size(); ++i)
	{
		if(people_on_tile[i].lock().get() == people_which_leaves.get())
		{
			people_on_tile.erase(people_on_tile.begin() + i);
		}
	}
	
	tile::minimap_updates.push_back(this);
	return 0;
}
 
int tile::show_effective_height()
{
	if(building_on_tile.expired())
		return show_surface_height();
	
	switch(building_on_tile.lock()->type)
	{
		case(WALL):
		case(PALISADE):
		case(NORTHWEST_STAIRS):
		case(NORTHEAST_STAIRS):
		case(SOUTHEAST_STAIRS):
		case(SOUTHWEST_STAIRS):
			return show_surface_height() + 1;
			break;
		case(SOUTHWEST_TOWER):
		case(NORTHWEST_TOWER):
		case(NORTHEAST_TOWER):
		case(SOUTHEAST_TOWER):
			return show_surface_height() + 2;
			break;
		default:
			{ }
	}
	return show_surface_height();
}

void tile::set_type(tile_type type)
{
	if((type == WATER) || (type == RAMP))
	{
		fertile = false;
		object = NOTHING;
	}
	this->type = type;
}

int tile::set_right_drawing(std::vector<std::vector<boost::shared_ptr<tile>>> & map)
{
	visible = true;
	if((show_surface_height() == 0) && ((show_type() == RAMP) || (show_type() == GRASS)))
	{
		set_type(GRASS);
		border = GROUND_LEVEL;
		return 0;
	}

	std::vector<std::pair<int, int>> neighbour_coordinates{std::pair<int, int>(tile_x, tile_y - 1), std::pair<int,int>(tile_x + 1, tile_y), 
							std::pair<int, int>(tile_x, tile_y + 1), std::pair<int, int>(tile_x - 1, tile_y)};
	
	std::vector<int> neighbour_heights(4, show_surface_height());
	for(int i=0; i<4; ++i)
	{
		if((neighbour_coordinates[i].first >= 0) && (neighbour_coordinates[i].first < game_info::map_width) &&
			(neighbour_coordinates[i].second >= 0) && (neighbour_coordinates[i].second < game_info::map_height))
		{
			neighbour_heights[i] = map[neighbour_coordinates[i].second][neighbour_coordinates[i].first]->show_surface_height();
		}
	}
	
	int number_of_down_tiles = 0;

	if(type == WATER)
	{
		bool cant_be_water = false;
		for(int i=0; i<4; ++i)
		{
			if(show_surface_height() > neighbour_heights[i])
				cant_be_water = true;
		}
		
		if(cant_be_water)
			set_type(GRASS);			

		else
		{
			if(show_surface_height() == 0)
				border = GROUND_LEVEL;
			else
				border = NO_BORDERS;

			return 0;
		}
	}	
	if(type == RAMP)
	{
		for(int i=0; i<4; i++)
		{
			if(show_surface_height() - 1 == neighbour_heights[i])
				number_of_down_tiles++;
		}
	}
	else if(type == GRASS)
	{
		for(int i=0; i<4; i++)
		{
			if(show_surface_height() > neighbour_heights[i])
				number_of_down_tiles++;
		}
	}
	

	border = UNKNOWN_BORDERS;
	if(number_of_down_tiles == 0)
	{
		if(type == GRASS)
		{
			border = NO_BORDERS;
		}
		else if(type == RAMP)
		{
			bool any_down_tiles = false;
			for(int i=0; i<neighbour_heights.size(); ++i)
			{
				if(neighbour_heights[i] < show_surface_height())
					any_down_tiles = true;
			}
			if(!any_down_tiles)
			{

				if((tile_y > 0) & (tile_x < game_info::map_width - 1))
				{
					if(map[tile_y - 1][tile_x + 1]->show_surface_height() + 1 == show_surface_height())
					{
						if((neighbour_heights[2] >= show_surface_height()) && (neighbour_heights[3] >= show_surface_height()))
						{
							border = EAST_INSIDE_BORDER;
						}
					}
				}
				if((tile_y < game_info::map_height - 1) & (tile_x < game_info::map_width - 1))
				{
					if(map[tile_y + 1][tile_x + 1]->show_surface_height() + 1 == show_surface_height())
					{
						if((neighbour_heights[3] >= show_surface_height()) && (neighbour_heights[0] >= show_surface_height()))
						{
							border = SOUTH_INSIDE_BORDER;
						}
					}
				}
				if((tile_y < game_info::map_height - 1) & (tile_x > 0))
				{
					if(map[tile_y + 1][tile_x - 1]->show_surface_height() + 1 == show_surface_height())
					{
						if((neighbour_heights[0] >= show_surface_height()) && (neighbour_heights[1] >= show_surface_height()))
						{
							border = WEST_INSIDE_BORDER;
						}
					}
				}
				if((tile_y > 0) & (tile_x > 0))
				{
					if(map[tile_y - 1][tile_x - 1]->show_surface_height() + 1 == show_surface_height())
					{
						if((neighbour_heights[1] >= show_surface_height()) && (neighbour_heights[2] >= show_surface_height()))
						{
							border = NORTH_INSIDE_BORDER;
						}
					}
				}
			}
		}
	}
	else if(number_of_down_tiles == 1)
	{
		if(type == GRASS)
		{
			if(neighbour_heights[0] < show_surface_height())
				border = NORTHEAST_BORDER;

			else if(neighbour_heights[1] < show_surface_height())
				border = SOUTHEAST_BORDER;

			else if(neighbour_heights[2] < show_surface_height())
				border = SOUTHWEST_BORDER;

			else if(neighbour_heights[3] < show_surface_height())
				border = NORTHWEST_BORDER;
		}
		else if(type == RAMP)
		{
			if((neighbour_heights[0] + 1 == show_surface_height()) && (neighbour_heights[2] >= show_surface_height()))
				border = NORTHEAST_BORDER;

			else if((neighbour_heights[1] + 1 == show_surface_height()) && (neighbour_heights[3] >= show_surface_height()))
				border = SOUTHEAST_BORDER;

			else if((neighbour_heights[2] + 1 == show_surface_height()) && (neighbour_heights[0] >= show_surface_height()))
				border = SOUTHWEST_BORDER;

			else if((neighbour_heights[3] + 1 == show_surface_height()) && (neighbour_heights[1] >= show_surface_height()))
				border = NORTHWEST_BORDER;
		}
	}
	else if(number_of_down_tiles == 2)
	{
		if(type == RAMP)
		{
			if((neighbour_heights[0] + 1 == show_surface_height()) & (neighbour_heights[1] +1  == show_surface_height()))
				border = EAST_BORDER;

			else if((neighbour_heights[1] + 1 == show_surface_height()) & (neighbour_heights[2] + 1 == show_surface_height()))
				border = SOUTH_BORDER;

			else if((neighbour_heights[2] + 1 == show_surface_height()) & (neighbour_heights[3] + 1 == show_surface_height()))
				border = WEST_BORDER;

			else if((neighbour_heights[3] + 1 == show_surface_height()) & (neighbour_heights[0] + 1 == show_surface_height()))
				border = NORTH_BORDER;
		}

		else if(type == GRASS)
		{
			if((neighbour_heights[0] < show_surface_height()) & (neighbour_heights[1] < show_surface_height()))
				border = EAST_BORDER;

			else if((neighbour_heights[1] < show_surface_height()) & (neighbour_heights[2] < show_surface_height()))
				border = SOUTH_BORDER;

			else if((neighbour_heights[2] < show_surface_height()) & (neighbour_heights[3] < show_surface_height()))
				border = WEST_BORDER;

			else if((neighbour_heights[3] < show_surface_height()) & (neighbour_heights[0] < show_surface_height()))
				border = NORTH_BORDER;

			else if((neighbour_heights[0] < show_surface_height()) & (neighbour_heights[2] < show_surface_height()))
				border = NORTHEAST_SOUTHWEST_BORDER;

			else if((neighbour_heights[1] < show_surface_height()) & (neighbour_heights[3] < show_surface_height()))
				border = NORTHWEST_SOUTHEAST_BORDER;
		}
	}
	else if((number_of_down_tiles == 3) && (type != RAMP))
	{
		if(neighbour_heights[0] >= show_surface_height())
			border = SOUTH_INSIDE_BORDER;

		else if(neighbour_heights[1] >= show_surface_height())
			border = WEST_INSIDE_BORDER;

		else if(neighbour_heights[2] >= show_surface_height())
			border = NORTH_INSIDE_BORDER;

		else if(neighbour_heights[3] >= show_surface_height())
			border = EAST_INSIDE_BORDER;
	}
	else if((number_of_down_tiles == 4) && (type != RAMP))
	{
		border = ALL_BORDERS;
	}	

	if(tile_x == game_info::map_width - 1)
		visible = true;
	
	if(tile_y == game_info::map_height - 1)
		visible = true;

	if(border == UNKNOWN_BORDERS)
	{
		set_type(GRASS);
		set_right_drawing(map);
	}

	return 0;
}

bool tile::are_people_on_tile()
{
	check_death_people_on_tile();
	return people_on_tile.size() > 0;
}

void tile::check_death_people_on_tile()
{
	auto it = remove_if(people_on_tile.begin(), people_on_tile.end(), [](boost::weak_ptr<people> p) {return p.expired();});
	people_on_tile.erase(it, people_on_tile.end());
}

int tile::add_object(object_on_tile object)
{
	if(type == GRASS)
	{
		if(object == TREE_TILE)
		{
			tile_object_image = TREE_IMAGE;
			number_of_floors = 4;
		}
		
		if((object == MARBLE_TILE) || (object == IRON_TILE) || (object == COAL_TILE) || (object == GOLD_TILE))
			tile_object_image = ROCK_RESOURCES_IMAGE;

		this->object = object;
	}
	return 0;
}

void tile::set_accessible_neighbours(std::vector<std::vector<boost::shared_ptr<tile>>>& map)
{
	accessible_neighbours.clear();

	for(int y=tile_y-1; y<= tile_y+1; y++)
	{
		for(int x=tile_x-1; x<=tile_x+1; x++)	//pro okolni policka
		{
			if((x == tile_x) && (y == tile_y))	//krome jeho samotneho
				continue;

			if(((x >= 0) && (x < game_info::map_width)) && ((y >= 0) && (y < game_info::map_height)))	//pokud policko existuje
			{
				if(people::general_can_move(this, map[y][x].get()))		//a da se tam dojit
					accessible_neighbours.push_back(map[y][x].get());
			}
		}
	}
}

int tile::add_path(bool real)
{
	if((!building_on_tile.expired()) || (object != NOTHING))
		return -1;
	
	if((path_on_tile) && (bIs_path_real))
		return -1;

	if(type == RAMP)
	{
		if((border != NORTHEAST_BORDER) && (border != SOUTHEAST_BORDER) && (border != SOUTHWEST_BORDER) && (border != NORTHWEST_BORDER))
		{
			return -1;
		}
	}
	if(type == WATER)
		return -1;

	path_on_tile = true;
	bIs_path_real = real;

	set_path_drawing();
	set_neighbours_with_path();
	
	if(tile_x - 1 >= 0)
		session->tile_list[tile_y][tile_x - 1]->set_path_drawing();

	if(tile_y - 1 >= 0)
		session->tile_list[tile_y - 1][tile_x]->set_path_drawing();
	
	if(tile_x + 1 < game_info::map_width)
		session->tile_list[tile_y][tile_x + 1]->set_path_drawing();

	if(tile_y + 1 < game_info::map_height)
		session->tile_list[tile_y + 1][tile_x]->set_path_drawing();

	return 0;
}
/*Removes path from tile and chnages path drawing on nearby tiles. If real is true, removes only real path, if false, removes only unreal path.*/ 
int tile::remove_path(bool real)
{
	if(!path_on_tile)
		return 0;

	if(!(bIs_path_real == real))
		return 0;
	
	path_on_tile = false;

	set_path_drawing();
	
	if(tile_x - 1 >= 0)
		session->tile_list[tile_y][tile_x - 1]->set_path_drawing();

	if(tile_y - 1 >= 0)
		session->tile_list[tile_y - 1][tile_x]->set_path_drawing();
	
	if(tile_x + 1 < game_info::map_width)
		session->tile_list[tile_y][tile_x + 1]->set_path_drawing();

	if(tile_y + 1 < game_info::map_height)
		session->tile_list[tile_y + 1][tile_x]->set_path_drawing();

	return 0;
}

void tile::add_fertile()
{
	if((type == GRASS) && (object == NOTHING))
		fertile = true;
}

int tile::set_path_drawing()
{
	if(type == RAMP)
	{
		if((border == NORTHEAST_BORDER) || (border == SOUTHEAST_BORDER) || (border == SOUTHWEST_BORDER) || (border == NORTHWEST_BORDER))
		{
			path_border = border;
		}
		else
		{
			LOG("error - path on wrong tile");
			path_on_tile = false;
		}
	}
	else
	{
		bool* is_path = new bool [4];
		for(int i=0; i<4; i++)
			is_path[i] = false;
			
		if(tile_y - 1 >= 0)
		{
			if(session->tile_list[tile_y - 1][tile_x]->path_on_tile)
				is_path[0] = true;
		}
		else
			is_path[0] = true;

		if(tile_x + 1 < game_info::map_width)
		{
			if(session->tile_list[tile_y][tile_x + 1]->path_on_tile)
				is_path[1] = true;
		}
		else
			is_path[1] = true;
		
		if(tile_y + 1 < game_info::map_height)
		{
			if(session->tile_list[tile_y + 1][tile_x]->path_on_tile)
				is_path[2] = true;
		}
		else
			is_path[2] = true;

		if(tile_x - 1 >= 0)
		{
			if(session->tile_list[tile_y][tile_x - 1]->path_on_tile)
				is_path[3] = true;
		}
		else
			is_path[3] = true;


		int number_of_paths = 0;
		for(int i=0; i<4; i++)
		{
			if(is_path[i])
				number_of_paths++;
		}

		if(number_of_paths == 0)
			path_border = NO_BORDERS;

		else if(number_of_paths == 1)
		{
			if(is_path[0])
				path_border = NORTHEAST_BORDER;
			
			else if(is_path[1])
				path_border = SOUTHEAST_BORDER;

			else if(is_path[2])
				path_border = SOUTHWEST_BORDER;

			else if(is_path[3])
				path_border = NORTHWEST_BORDER;
		}

		else if(number_of_paths == 2)
		{

			if(is_path[0] && is_path[1])
				path_border = EAST_BORDER;

			else if(is_path[1] && is_path[2])
				path_border = SOUTH_BORDER;

			else if(is_path[2] && is_path[3])
				path_border = WEST_BORDER;

			else if(is_path[3] && is_path[0])
				path_border = NORTH_BORDER;
		
			else if(is_path[0] && is_path[2])
				path_border = NORTHEAST_SOUTHWEST_BORDER;

			else if(is_path[1] && is_path[3])
				path_border = NORTHWEST_SOUTHEAST_BORDER;
		}
		else if(number_of_paths == 3)
		{
			if(!is_path[0])
				path_border = SOUTH_INSIDE_BORDER;
				
			if(!is_path[1])
				path_border = WEST_INSIDE_BORDER;
			
			if(!is_path[2])
				path_border = NORTH_INSIDE_BORDER;
			
			if(!is_path[3])
				path_border = EAST_INSIDE_BORDER;
		}
		else if(number_of_paths == 4)
			path_border = ALL_BORDERS;

		delete[] is_path;
	}

	return 0;
}

void tile::set_neighbours_with_path()
{
	std::vector<std::pair<int, int>> neighbours{std::pair<int,int>(-1,0), std::pair<int, int>(1,0), std::pair<int, int>(0, 1), std::pair<int, int>(0, -1)};
	
	for(int i=0; i<session->tile_list.size(); ++i)
	{
		for(int j=0; j<session->tile_list[i].size(); ++j)
		{
			for(int k=0; k<4; ++k)
			{
				int x = j + neighbours[k].first;
				int y = i + neighbours[k].second;

				session->tile_list[j][i]->neighbours_with_path.clear();
				if((x >= 0) && (x < game_info::map_width) && (y >= 0) && (y < game_info::map_height))
				{
					if(session->tile_list[y][x]->real_path_on_tile())
					{
						session->tile_list[j][j]->neighbours_with_path.push_back(session->tile_list[y][x].get());
					}
				}
			}
		}
	}
}

void tile::prepare_serialization()
{
	check_consistency();

	serializable_accessible_neighbours.clear();
	for(int i=0; i<accessible_neighbours.size(); ++i)
	{
		std::pair<int, int> coordinates(accessible_neighbours[i]->show_tile_x(), accessible_neighbours[i]->show_tile_y());
		serializable_accessible_neighbours.push_back(coordinates);
	}
	serializable_neighbours_with_path.clear();
	for(int i=0; i<neighbours_with_path.size(); ++i)
	{
		std::pair<int, int> coordinates(neighbours_with_path[i]->show_tile_x(), neighbours_with_path[i]->show_tile_y());
		serializable_neighbours_with_path.push_back(coordinates);
	}
}

void tile::finish_serialization()
{
	accessible_neighbours.clear();
	neighbours_with_path.clear();
	for(int i=0; i<serializable_accessible_neighbours.size(); ++i)
	{
		tile* t = session->tile_list[serializable_accessible_neighbours[i].second][serializable_accessible_neighbours[i].first].get();
		accessible_neighbours.push_back(t);
	}
	for(int i=0; i<serializable_neighbours_with_path.size(); ++i)
	{
		tile* t = session->tile_list[serializable_neighbours_with_path[i].second][serializable_neighbours_with_path[i].first].get();
		neighbours_with_path.push_back(t);
	}

	check_consistency();
}

void tile::check_consistency()
{
	assert(people_on_tile.size() >= 0);
	assert((object == NOTHING) || (object == IRON_TILE) || (object == MARBLE_TILE) || (object == GOLD_TILE) || (object == COAL_TILE) || (object == TREE_TILE));
	assert(((unsigned int)can_go_inside_building == 1) || ((unsigned int)can_go_inside_building == 0));
	assert(((unsigned int)can_go_on_building == 1) || ((unsigned int)can_go_on_building == 0));
	assert(((unsigned int)stairs_on_tile == 1) || ((unsigned int)stairs_on_tile == 0));
	assert((((unsigned int)tile_object_image) >= 0) && (((unsigned int)tile_object_image) < LAST_IMAGE));
	assert(((unsigned int)path_on_tile == 1) || ((unsigned int)path_on_tile == 0));
	assert(((unsigned int)bIs_path_real == 1) || ((unsigned int)bIs_path_real == 0));
	assert(((unsigned int)visible == 1) || ((unsigned int)visible == 0));
	assert((((unsigned int)border) >= 0) && (((unsigned int)border) <= UNKNOWN_BORDERS));
	assert(((unsigned int)path_border >= 0) && (((unsigned int)path_border) <= UNKNOWN_BORDERS));
	assert((((unsigned int)type) >= 0) && (((unsigned int)type) <= WATER));
	assert(((unsigned int)draw_building == 1) || ((unsigned int)draw_building == 0));
	assert(((unsigned int)fertile == 1) || ((unsigned int)fertile == 0));
}
	




