#include "core.h"

extern std::ofstream log_file;
extern game_session* session;

game_session::game_session(std::vector<int> starting_resources, int starting_honour, int enemies, std::vector<int> natural_resources_amount, int mountains_amount) 
	: tile_list(generate_map(natural_resources_amount, mountains_amount)), unlocked_buildings(set_unlocked_warehouse()), game_started(false)
{
	std::vector<resources> all_resources;
	for(int i=0; i<NUMBER_OF_RESOURCES; ++i)
		all_resources.push_back(static_cast<resources>(i));
	
	global_stock = boost::shared_ptr<carrier_output>(new carrier_output(std::numeric_limits<int>::max(), all_resources, all_resources, 0));
	
	global_stock->save_list(starting_resources);
	honour = starting_honour;
	switch(enemies)
	{
		case(0):
		{
			time_to_invasion = -1;
			invasion_interval = -1;
		}
		break;
		case(1):
		{
			time_to_invasion = 600*game_info::fps;		//first_invasion
			invasion_interval = 400*game_info::fps;
		}
		break;
		case(2):
		{
			time_to_invasion = 300*game_info::fps;
			invasion_interval = 200*game_info::fps;
		}
		break;
		default:
			throw new std::exception;
	}

	frames_from_start = 0;
	happiness = 100;
	honour = starting_honour;
	starving = false;
	invasion_number = 0;
	game_started = false;
	
	button::set_basic_button_list(true);

	//init_static_variables();
}

int game_session::update(game_mouse* mouse, bool& done)
{
	if(!game_started)
	{
		if(building_list.size() > 0)
		{
			game_started = true;
			button::set_basic_button_list(false);
			mouse->unchoose_button();
			unlocked_buildings = game_session::set_unlocked_buildings();
		}
		return 0;
	}

	bool warehouse_exist = false;
	for(int i=0; i<building_list.size(); ++i)
	{
		if(building_list[i]->show_type() == WAREHOUSE)
		{
			warehouse_exist = true;
			break;
		}
	}
	
	if(!warehouse_exist)
	{
	//	message("Your warehouse was destroyed! You lost.");
		done = true;
		return 0;
	}


	ai.update();

	LOG("happiness");
	update_happiness();
	
	LOG("invasion");
	{
		if(time_to_invasion > 0)
			time_to_invasion--;

		else if(time_to_invasion == 0)
		{
			time_to_invasion = invasion_interval;
			invasion();
		}
	}
			
	frames_from_start++;

	LOG("end");
	return 0;
}

int game_session::update_happiness()
{
	if((frames_from_start % TIME_TO_UPDATE_HAPPINESS) != 0)
		return 0;

	int happiness_equilibrium = 100;
	for(int i=0; i<building_list.size(); ++i)
	{
		if(building_list[i]->type == HOUSE)
		{
			house* house_ptr = dynamic_cast<house*>(building_list[i].get());
			happiness_equilibrium += house_ptr->show_happiness_modifier();
		}
	}

	if(happiness < happiness_equilibrium)
		happiness++;
	
	else if(happiness > happiness_equilibrium)
		happiness--;

	if(happiness < 0)
		happiness = 0;

	return 0;
}
		
int game_session::invasion()
{
	LOG("invasion");
	//message("Defend your buildings. Enemies coming.");

	int x = rand() % MAP_WIDTH;
	int y = rand() % MAP_HEIGHT;
	int side = rand() % 4;

	switch(side){		//move to map borders
	case(0):
		x = 0;
		break;
	case(1):
		x = MAP_WIDTH - 1;
		break;
	case(2):
		y = 0;
		break;
	case(3):
		y = MAP_HEIGHT - 1;
		break;
	}
	
	int number_of_enemies = 5*pow(2, invasion_number) + rand() % 5*pow(2, invasion_number);
	
	invasion_number++;

	for(int i=0; i<number_of_enemies; i++)
	{
		people_type t;
		switch(rand() % 5)
		{
			case(0):
				t = AXEMAN;
				break;
			case(1):
				t = SPEARMAN;
				break;
			case(2):
				t = SWORDSMAN;
				break;
			case(3):
				t = BOWMAN;
				break;
			case(4):
				t = CATAPULT;
				break;
		}
		warrior_born_near(t, x, y, RED_PLAYER);
		//warrior_born_near(CATAPULT, x, y, RED_PLAYER);
	}

	ai.register_units();

	LOG("number_of_people: " << people_list.size() << " number_of_enemies " << number_of_enemies);
	
	return 0;
}

/*
void game_session::reset_tile_list()
{
	tile_list.clear();
	for(int i=0; i<game_info::map_height; ++i)
	{
		tile_list.push_back(std::vector<boost::shared_ptr<tile>>());
		for(int j=0; j<game_info::map_width; ++j)
			tile_list[i].push_back(boost::shared_ptr<tile>(new tile(RAMP, j, i, 0)));
	
	}
}
*/

std::vector<bool> game_session::set_unlocked_warehouse()
{
	std::vector<bool> unlocked(NUMBER_OF_BUILDINGS, false);
	unlocked[WAREHOUSE] = true;
	
	
	return unlocked;
}

std::vector<bool> game_session::set_unlocked_buildings()
{
	std::vector<bool> unlocked(NUMBER_OF_BUILDINGS, false);

	unlocked[HOUSE] = true;
	unlocked[MARKET] = true;
	unlocked[QUARRY] = true;
	unlocked[WOODCUTTER] = true;
	unlocked[FISHERMAN] = true;
	unlocked[HUNTER] = true;
	unlocked[APPLE_FARM] = true;
	unlocked[GREAT_HALL] = true;

	return unlocked;
}

void game_session::finish_serialization()
{
	button::set_basic_button_list(false);
	
	for(int i=0; i<session->tile_list.size(); ++i)
	{
		for(int j=0; j<session->tile_list[i].size(); ++j)
			session->tile_list[i][j]->finish_serialization();
	}
}


