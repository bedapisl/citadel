#include "core.h" 
#include "people.h"

int game_info::map_width = MAP_WIDTH;
int game_info::map_height = MAP_HEIGHT;
bool game_info::fullscreen = false;
//ALLEGRO_TIMER* game_info::timer = NULL;
int game_info::fps = 60;
bool game_info::close_display = false;
bool game_info::music = false;
ALLEGRO_DISPLAY* game_info::display = NULL;

extern ALLEGRO_BITMAP** image_list;
extern std::ofstream log_file;
extern game_session* session;

missile::missile(missile_type type, tile* attacker_position, int damage, tile* goal) 
			: type(type), attacker_position(attacker_position), damage(damage), goal(goal), bIs_death(false)
{
	game_x = compute_game_x(attacker_position->show_tile_x(), attacker_position->show_tile_y());
	game_y = compute_game_y(attacker_position->show_tile_x(), attacker_position->show_tile_y()) - 32*attacker_position->show_surface_height();

	if(type == ARROW)	//classic arrows are from TOWERs
	{
		image = ARROW_IMAGE;
	}
	else if(type == ARCHERS_ARROW)
	{
		image = ARCHERS_ARROW_IMAGE;
	}
	else if(type == CATAPULT_SHOT)
	{
		image = CATAPULT_SHOT_IMAGE;
	}

	int goal_game_x = compute_game_x(goal->show_tile_x(), goal->show_tile_y());
	int goal_game_y = compute_game_y(goal->show_tile_x(), goal->show_tile_y()) - 32*goal->show_surface_height();

	angle = ALLEGRO_PI + acos((float)(goal_game_x - game_x)/sqrt((float)(pow(goal_game_x - game_x, 2) + pow(goal_game_y - game_y, 2))));
	if(goal_game_y - game_y < 0)
		angle = 2*ALLEGRO_PI - angle;

	int speed;

	if(type ==  ARROW)
		speed = 20;
	
	if(type == ARCHERS_ARROW)
		speed = 20;

	if(type == CATAPULT_SHOT)
		speed = 6;

	int distance = pow((double)pow(goal_game_x - game_x, 2) + pow(2*(goal_game_y - game_y), 2), 0.5);

	frames_to_goal = distance/speed;
	if(frames_to_goal == 0)
	{
		game_x_change = 0;
		game_y_change = 0;
	}
	else
	{
		game_x_change = (goal_game_x - game_x)/frames_to_goal;
		game_y_change = (goal_game_y - game_y)/frames_to_goal;
	}
}

/*Draws and update missile. Should be called once every frame.*/
int missile::draw_missile(int screen_position_x, int screen_position_y)
{
	if(is_death())
		return 0;

	game_x += game_x_change;
	game_y += game_y_change;
	frames_to_goal--;
	if(frames_to_goal >= 1)
	{
		al_draw_rotated_bitmap(image_list[image], 32, 32, game_x - screen_position_x, game_y - screen_position_y, angle, 0);
	}
	else if(frames_to_goal < 1)
	{
		bIs_death = true;
		
		goal->check_death_people_on_tile();

		if(!goal->people_on_tile.empty())
			goal->people_on_tile[0].lock()->damage(damage, attacker_position, true);
		
		else if(!goal->building_on_tile.expired())
			goal->building_on_tile.lock()->damage(damage);
	}
	
	return 0;
}

possible_borders tile_borders(tile* cTile, direction way)
{
	if((!cTile->is_ramp()) && (cTile->building_on_tile.expired()))
	{
		return NORMAL_BORDER;
	}
	
	borders tile_borders = cTile->show_border();
	bool double_ramp = false;
	possible_borders result = NORMAL_BORDER;

	if(!cTile->building_on_tile.expired())
	{
		switch(cTile->building_on_tile.lock()->type)
		{
			case(NORTHWEST_STAIRS):
				tile_borders = SOUTHEAST_BORDER;
				break;

			case(NORTHEAST_STAIRS):
				tile_borders = SOUTHWEST_BORDER;
				break;

			case(SOUTHEAST_STAIRS):
				tile_borders = NORTHWEST_BORDER;
				break;

			case(SOUTHWEST_STAIRS):
				tile_borders = NORTHEAST_BORDER;
				break;
			
			case(SOUTHWEST_TOWER):
			{
				if(cTile == boost::dynamic_pointer_cast<tower>(cTile->building_on_tile.lock())->doors_tile)
				{
					tile_borders = SOUTHWEST_BORDER;
					double_ramp = true;
				}
				else 
					return NORMAL_BORDER;

			}
			break;
			case(NORTHWEST_TOWER):
			{
				if(cTile == boost::dynamic_pointer_cast<tower>(cTile->building_on_tile.lock())->doors_tile)
				{
					tile_borders = NORTHWEST_BORDER;
					double_ramp = true;
				}
				else
					return NORMAL_BORDER;
			}
			break;
			case(NORTHEAST_TOWER):
			{
				if(cTile == boost::dynamic_pointer_cast<tower>(cTile->building_on_tile.lock())->doors_tile)
				{
					tile_borders = NORTHEAST_BORDER;
					double_ramp = true;
				}
				else
					return NORMAL_BORDER;
			}
			break;
			case(SOUTHEAST_TOWER):
			{
				if(cTile == boost::dynamic_pointer_cast<tower>(cTile->building_on_tile.lock())->doors_tile)
				{
					tile_borders = SOUTHEAST_BORDER;
					double_ramp = true;
				}
				else
					return NORMAL_BORDER;
			}
			break;

			default:
				return NORMAL_BORDER;
		}
	}
		
	if((tile_borders == NO_BORDERS) || (tile_borders == GROUND_LEVEL))
		result = NORMAL_BORDER;

	switch(tile_borders)				
	{
	case(SOUTHWEST_BORDER):
		{
			if((way == NORTHWEST) || (way == SOUTHEAST))
				result = RAMP_BORDER;
			else if((way == NORTH) || (way == NORTHEAST) || (way == EAST))
				result = NORMAL_BORDER;
			else result = DOWN_BORDER;	
		}
		break;
	case(NORTHWEST_BORDER):
		{
			if((way == SOUTHWEST) || (way == NORTHEAST))
				result = RAMP_BORDER;
			else if((way == EAST) || (way == SOUTHEAST) || (way == SOUTH))
				result = NORMAL_BORDER;
			else result = DOWN_BORDER;	
		}
		break;
	case(NORTHEAST_BORDER):
		{
			if((way == NORTHWEST) || (way == SOUTHEAST))
				result = RAMP_BORDER;
			else if((way == SOUTH) || (way == SOUTHWEST) || (way == WEST))
				result = NORMAL_BORDER;
			else result = DOWN_BORDER;	
		}
		break;
	case(SOUTHEAST_BORDER):
		{
			if((way == SOUTHWEST) || (way == NORTHEAST))
				result = RAMP_BORDER;
			else if((way == WEST) || (way == NORTHWEST) || (way == NORTH))
				result = NORMAL_BORDER;
			else result = DOWN_BORDER;	
		}
		break;
	case(SOUTH_BORDER):
		{
			if(way == NORTH)
				result = NORMAL_BORDER;
			else if((way == NORTHEAST) || (way == NORTHWEST))
				result = RAMP_BORDER;
			else result = DOWN_BORDER;
		}
		break;
	case(WEST_BORDER):
		{
			if(way == EAST)
				result = NORMAL_BORDER;
			else if((way == NORTHEAST) || (way == SOUTHEAST))
				result = RAMP_BORDER;
			else result = DOWN_BORDER;
		}
		break;
	case(NORTH_BORDER):
		{
			if(way == SOUTH)
				result = NORMAL_BORDER;
			else if((way == SOUTHEAST) || (way == SOUTHWEST))
				result = RAMP_BORDER;
			else result = DOWN_BORDER;
		}
		break;
	case(EAST_BORDER):
		{
			if(way == WEST)
				result = NORMAL_BORDER;
			else if((way == NORTHWEST) || (way == SOUTHWEST))
				result = RAMP_BORDER;
			else result = DOWN_BORDER;
		}
		break;
	case(SOUTH_INSIDE_BORDER):
		{
			if(way == SOUTH)
				result = DOWN_BORDER;
			else if((way == SOUTHEAST) || (way == SOUTHWEST))
				result = RAMP_BORDER;
			else result = NORMAL_BORDER;
		}
	case(WEST_INSIDE_BORDER):
		{
			if(way == WEST)
				result = DOWN_BORDER;
			else if((way == SOUTHWEST) || (way == NORTHWEST))
				result = RAMP_BORDER;
			else result = NORMAL_BORDER;
		}
	case(NORTH_INSIDE_BORDER):
		{
			if(way == NORTH)
				result = DOWN_BORDER;
			else if((way == NORTHEAST) || (way == NORTHWEST))
				result = RAMP_BORDER;
			else result = NORMAL_BORDER;
		}
	case(EAST_INSIDE_BORDER):
		{
			if(way == EAST)
				result = DOWN_BORDER;
			else if((way == SOUTHEAST) || (way == NORTHEAST))
				result = RAMP_BORDER;
			else result = NORMAL_BORDER;
		}
	}
	if((result == DOWN_BORDER) && (double_ramp == true))
		result = DOUBLE_DOWN_BORDER;
	
	return result;
}

int stock::save(resources type, int amount)
{
	stored[type] += amount;

	if(stored[type] > capacity)
	{
		int difference = stored[type] - capacity;
		stored[type] = capacity;
		return difference;
	}
	return 0;
}
 
void stock::save_list(std::vector<int> prices)
{
	for(int i=0; i<prices.size(); ++i)
	{
		save(static_cast<resources>(i), prices[i]);
	}
}

bool stock::try_subtract(resources type, int amount)
{
	if(stored[type] < amount)
		return false;
	
	stored[type] -= amount;
	return true;
}
 
bool stock::try_subtract_list(std::vector<int> price)
{
	for(int i=0; i<NUMBER_OF_RESOURCES; ++i)
	{
		if(stored[i] < price[i])
		{
			return false;
		}
	}
	for(int i=0; i<NUMBER_OF_RESOURCES; ++i)
	{
		stored[i] -= price[i];
	}
	return true;
}

void stock::subtract(resources type, int amount)
{
	stored[type] -= amount;
	if(stored[type] < 0)
	{
		LOG("error");
		throw new std::exception;
	}
}


void stock::draw_nonzero_resources(int real_x, bool with_capacity)
{
	int drawed_resources = 0;
	for(int i=0; i<stored.size(); ++i)
	{
		if(stored[i] != 0)
		{
			al_draw_bitmap_region(image_list[RESOURCES_IMAGE], i*30, 0, 30, 30, drawed_resources*90 + real_x, display_height - BUTTON_SIZE + 5, 0);
			if(with_capacity)
				al_draw_textf(font25, WRITING_COLOR, drawed_resources*90 + 30 + real_x, display_height - BUTTON_SIZE + 5, 0, "%i/%i", stored[i], capacity);
			else
				al_draw_textf(font25, WRITING_COLOR, drawed_resources*90 + 30 + real_x, display_height - BUTTON_SIZE + 5, 0, "%i", stored[i]);

			drawed_resources++;
		}
	}
}

void stock::increase_capacity(int new_capacity)
{
	if(new_capacity < capacity)
		throw new std::exception;

	capacity = new_capacity;
}


carrier_output::carrier_output(int capacity, std::vector<resources> in, std::vector<resources> out, int number_of_carriers) 
	: stock(capacity), in(in), out(out), reserved_in(NUMBER_OF_RESOURCES, 0), reserved_out(NUMBER_OF_RESOURCES, 0),	
		max_number_of_carriers(number_of_carriers), time_to_produce_carrier(TIME_TO_PRODUCE_CARRIER)
{

}

void carrier_output::update()
{
	for(int i=0; i<carriers.size(); ++i)
	{
		if(carriers[i].expired())
		{
			carriers.erase(carriers.begin());
		}
	}
	
	if(carriers.size() < max_number_of_carriers)
	{
		if(time_to_produce_carrier > 0)
		{
			time_to_produce_carrier--;
		}
		else
		{
			boost::shared_ptr<people> new_carrier(new carrier(CARRIER, this_building.lock()->show_tile_x(), this_building.lock()->show_tile_y(), 
								this_building.lock()->show_surface_height(), BLUE_PLAYER, this_building.lock()));
			idle_carriers.push_back(boost::dynamic_pointer_cast<carrier>(new_carrier));
			carriers.push_back(boost::dynamic_pointer_cast<carrier>(new_carrier));
			session->people_list.push_back(new_carrier);

			time_to_produce_carrier = TIME_TO_PRODUCE_CARRIER;
		}
	}

	for(int i=0; i<idle_carriers.size(); ++i)
	{
		if(idle_carriers[i].expired())
		{
			idle_carriers.erase(idle_carriers.begin());
		}
	}
	
	if(idle_carriers.size() > 0)
	{
		assign_tasks();
	}
}
	

bool carrier_output::reserve_transaction(resources resource_type, int amount, transaction_type type)
{
	if((type == IN) && (contains(in, resource_type)))
	{
		if(stored[resource_type] + reserved_in[resource_type] + amount <= capacity)
		{
			reserved_in[resource_type] += amount;
			return true;
		}
	}
	if((type == OUT) && (contains(out, resource_type)))
	{
		if(stored[resource_type] - reserved_out[resource_type] - amount >= 0)
		{
			reserved_out[resource_type] += amount;
			return true;
		}
	}
	return false;
}

void carrier_output::delete_transaction(resources resource_type, int amount, transaction_type type)
{
	if(type == IN)
	{
		reserved_in[resource_type] -= amount;

		if(reserved_in[resource_type] < 0)
			throw new std::exception;
	}
	else if(type == OUT)
	{
		reserved_out[resource_type] -= amount;

		if(reserved_out[resource_type] < 0)
			throw new std::exception;
	}
}

bool carrier_output::accomplish_transaction(resources resource_type, int amount, transaction_type type)
{
	if(type == IN)
	{
		reserved_in[resource_type] -= amount;
		save(resource_type, amount);			//if output doesn't have enough capacity, resources disapear
		if(reserved_in[resource_type] < 0)
			throw new std::exception;
	}
	else if(type == OUT)
	{
		reserved_out[resource_type] -= amount;
		bool return_value = try_subtract(resource_type, amount);
		if((return_value == false) || (reserved_out[resource_type] < 0))
			throw new std::exception;
	}
	return true;
}

int carrier_output::show_max_possible_transaction(resources resource_type, transaction_type type)
{
	if((type == IN) && (contains(in, resource_type)))
	{
		return capacity - stored[resource_type] - reserved_in[resource_type];
	}
	else if((type == OUT) && (contains(out, resource_type)))
	{
		return stored[resource_type] - reserved_out[resource_type];
	}
	return 0;
}

void carrier_output::change_accepted_resources(resources r, bool in, bool add)
{
	std::vector<resources>* vector_to_change;
	if(in)
		vector_to_change = &this->in;
	
	else
		vector_to_change = &this->out;

	if(add)
	{
		for(int i=0; i<vector_to_change->size(); ++i)
		{
			if((*vector_to_change)[i] == r)
				return;
		}
		vector_to_change->push_back(r);
	}
	else
	{
		for(int i=0; i<vector_to_change->size(); ++i)
		{
			if((*vector_to_change)[i] == r)
			{
				vector_to_change->erase(vector_to_change->begin() + i);
			}
		}
	}
}

int carrier_output::show_idle_carrier_capacity()
{
	for(int i=0; i<idle_carriers.size(); ++i)
	{
		if(idle_carriers[i].expired())
			idle_carriers.erase(idle_carriers.begin() + i);
	}
	return idle_carriers.size() * CARRIER_CAPACITY;
}

void carrier_output::assign_tasks()
{
	
	std::vector<tile*> building_tiles = tiles_under_building(this_building.lock()->show_tile_x(), this_building.lock()->show_tile_y(), this_building.lock()->show_size());
	std::vector<tile*> starting_tiles;
	for(int i=0; i<building_tiles.size(); ++i)
	{
		std::vector<tile*> adjacent_tiles = pathfinding::adjacent_tiles(building_tiles[i], false);
		for(int j=0; j<adjacent_tiles.size(); ++j)
		{
			if(carrier::static_can_move(building_tiles[i], adjacent_tiles[j]) && (!contains(starting_tiles, adjacent_tiles[j])))
			{
				starting_tiles.push_back(adjacent_tiles[j]);
			}
		}
	}

	std::vector<std::vector<tile*>> paths_to_tasks = pathfinding::breadth_first_search(starting_tiles, carrier::static_can_move, pathfinding::any_building_goal_functor(), false, true);

	for(int i=0; i<paths_to_tasks.size(); ++i)
	{
		if(paths_to_tasks[i].back()->building_on_tile.lock()->type == WAREHOUSE)
		{
			for(int j=i+1; j<paths_to_tasks.size(); ++j)
			{
				if(paths_to_tasks[j].back()->building_on_tile.lock()->type != WAREHOUSE)
				{
					swap(paths_to_tasks[i], paths_to_tasks[j]);
					break;
				}
			}
		}
		boost::shared_ptr<building> founded_building = paths_to_tasks[i].back()->building_on_tile.lock();
		if((!founded_building->has_carrier_output()) || (founded_building == this_building.lock()))
		{
			paths_to_tasks.erase(paths_to_tasks.begin() + i);
			--i;
		}
	}

	for(int i=0; i<paths_to_tasks.size(); ++i)
	{
		boost::shared_ptr<carrier_output> founded_output = paths_to_tasks[i].back()->building_on_tile.lock()->show_carrier_output();

		for(int j = 0; j < in.size(); ++j)
		{
			int max_amount = std::min(show_max_possible_transaction(in[j], IN), founded_output->show_max_possible_transaction(in[j], OUT));
			
			while(max_amount > 0)
			{
				int real_amount = std::min(max_amount, CARRIER_CAPACITY);
				max_amount -= real_amount;

				idle_carriers.front().lock()->give_task(in[j], real_amount, IN, paths_to_tasks[i].back()->building_on_tile.lock(), paths_to_tasks[i]);
				idle_carriers.erase(idle_carriers.begin());
				if(idle_carriers.size() == 0)
				{
					return;
				}
			}
		}
		for(int j = 0; j < out.size(); ++j)
		{
			int max_amount = std::min(show_max_possible_transaction(out[j], OUT), founded_output->show_max_possible_transaction(out[j], IN));
			
			while(max_amount > 0)
			{
				int real_amount = std::min(max_amount, CARRIER_CAPACITY);
				max_amount -= real_amount;

				idle_carriers.front().lock()->give_task(out[j], real_amount, OUT, paths_to_tasks[i].back()->building_on_tile.lock(),
										paths_to_tasks[i]);
				idle_carriers.erase(idle_carriers.begin());
				if(idle_carriers.size() == 0)
				{
					return;
				}
			}
		}
	}
}






