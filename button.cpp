#include "core.h"

extern int display_width;
extern int display_height;
extern std::ofstream log_file;
extern ALLEGRO_BITMAP** image_list;
extern ALLEGRO_FONT* font15;
extern game_session* session;

boost::shared_ptr<std::vector<boost::shared_ptr<button>>> button::button_list;

void button::draw_button(int button_number)
{
	al_draw_filled_rectangle(button_number*BUTTON_SIZE + 5, display_height - BUTTON_SIZE + 5, (button_number + 1)*BUTTON_SIZE -5 , display_height - 5, BLACK_COLOR);		//draws black rectangle
	al_draw_bitmap_region(action_image, 0, 0, 64, 64, button_number*BUTTON_SIZE + 8, display_height - BUTTON_SIZE + 8, 0);	//draws button image

	if(button_number == 9)		//draws number of action 
		al_draw_textf(font15, WRITING_COLOR, button_number*BUTTON_SIZE + 72, display_height - 23, ALLEGRO_ALIGN_RIGHT, "%i", 0);
	else if(button_number < 9)
		al_draw_textf(font15, WRITING_COLOR, button_number*BUTTON_SIZE + 72, display_height - 23, ALLEGRO_ALIGN_RIGHT, "%i", button_number + 1);
}

button::button(ALLEGRO_BITMAP* image, button_type type, bool multiple_selection)
 		: action_image(image), type(type), multiple_selection(multiple_selection)
{ } 

	

int button::general_update_tiles_with_action(bool mouse_button_down, int tile_x, int tile_y, int button_down_tile_x, int button_down_tile_y)
{
	LOG("updating tiles tile_x: " << tile_x << " tile_y: " << tile_y << " button_x: " << button_down_tile_x << " button_y " << button_down_tile_y);
	reset_tiles_with_action();
	
	tile_x = std::min(game_info::map_width - 1, std::max(0, tile_x));			//tile_x should be valid tile coordinate
	tile_y = std::min(game_info::map_height - 1, std::max(0, tile_y));
	button_down_tile_x = std::min(game_info::map_width - 1, std::max(0, button_down_tile_x));
	button_down_tile_y = std::min(game_info::map_height - 1, std::max(0, button_down_tile_y));

	if((multiple_selection) && (mouse_button_down))		
	{
		
		int start_x = std::min(button_down_tile_x, tile_x);		//choose right tiles and give them action
		int start_y = std::min(button_down_tile_y, tile_y);
		int end_x = std::max(button_down_tile_x, tile_x);
		int end_y = std::max(button_down_tile_y, tile_y);

		for(int i=start_x; i<end_x; i++)
			tiles_with_action.push_back(session->tile_list[button_down_tile_y][i].get());
		
		for(int i=start_y; i<=end_y; i++)
			tiles_with_action.push_back(session->tile_list[i][tile_x].get());
	}
	else 		//only one tile with action
		tiles_with_action.push_back(session->tile_list[tile_y][tile_x].get());

	return 0;
}

void button::reset_tiles_with_action()
{
	for(size_t i=0; i<tiles_with_action.size(); ++i)
	{
		tiles_with_action[i]->action_on_tile.reset();
	}
	tiles_with_action.clear();
}

void button::set_basic_button_list(bool restart_unlocks)
{
	static boost::shared_ptr<std::vector<boost::shared_ptr<button>>> basic_button_list = init_buttons();
	static boost::shared_ptr<std::vector<boost::shared_ptr<button>>> starting_button_list(new std::vector<boost::shared_ptr<button>>(1, boost::shared_ptr<button>(new button_build(WAREHOUSE))));

	if(restart_unlocks)
	{
		basic_button_list = init_buttons();
		starting_button_list = boost::shared_ptr<std::vector<boost::shared_ptr<button>>>(new std::vector<boost::shared_ptr<button>>(1, boost::shared_ptr<button>(new button_build(WAREHOUSE))));
	}
	
	if((session != NULL) && (session->game_started))
	{
		button::button_list = basic_button_list;
	}
	else
		button::button_list = starting_button_list;
}

boost::shared_ptr<std::vector<boost::shared_ptr<button>>> button::init_buttons()
{
	int max_number_of_buttons = (display_width - 3*BUTTON_SIZE) / BUTTON_SIZE;

	if(max_number_of_buttons < 3)
	{
		LOG("display is too small");
		throw std::exception();
	}
	
	std::vector<building_type> food{HUNTER, FISHERMAN, APPLE_FARM, DAIRY_FARM, WHEAT_FARM, WINDMILL};
	std::vector<building_type> economy{HOUSE, MARKET, GREAT_HALL, CHURCH, STORE, WAREHOUSE};
	std::vector<building_type> industry{QUARRY, WOODCUTTER, CLAY_PIT, POTTERY_WORKSHOP, BRICKMAKER, MARBLE_QUARRY, GOLD_MINE, COAL_MINE, IRON_MINE};
	std::vector<building_type> military{BARRACKS, PALISADE, WALL, LEFT_GATE, NORTHWEST_TOWER, NORTHWEST_STAIRS, SMITH, ARMOURER, FLETCHER};
	
	std::vector<std::vector<building_type>> buttons_init_list{food, economy, industry, military};

	boost::shared_ptr<std::vector<boost::shared_ptr<button>>> basic(new std::vector<boost::shared_ptr<button>>());
	basic->push_back(boost::shared_ptr<button>(new navigation_button(FOOD, "Food production")));
	basic->push_back(boost::shared_ptr<button>(new navigation_button(ECONOMY, "Economy buildings")));
	basic->push_back(boost::shared_ptr<button>(new navigation_button(INDUSTRY, "Industry buildings")));
	basic->push_back(boost::shared_ptr<button>(new navigation_button(MILITARY, "Military buildings")));
	basic->push_back(boost::shared_ptr<button>(new button_path(true)));
	basic->push_back(boost::shared_ptr<button>(new button_path(false)));

	for(size_t i=0; i<buttons_init_list.size(); ++i)
	{
		size_t button_index = 0;
		boost::shared_ptr<std::vector<boost::shared_ptr<button>>> back_button_list = basic;
		boost::shared_ptr<std::vector<boost::shared_ptr<button>>> button_list(new std::vector<boost::shared_ptr<button>>);
		
		(boost::dynamic_pointer_cast<navigation_button>((*basic)[i]))->set_buttons_after_use(button_list);
		
		while(button_index < buttons_init_list[i].size())
		{	
			bool some_buttons_left = true;	

			button_list->push_back(boost::shared_ptr<button>(new navigation_button(BACK, "Previous selection")));
			(boost::dynamic_pointer_cast<navigation_button>((*button_list)[0]))->set_buttons_after_use(back_button_list);

			for(int j=1; j < max_number_of_buttons; ++j)
			{
				if(button_index < buttons_init_list[i].size())		//can add button to the current list
				{
					button_list->push_back(boost::shared_ptr<button>(new button_build(buttons_init_list[i][button_index])));
					++button_index;
				}
				else
				{
					some_buttons_left = false;
					break;
				}
			}
			if(some_buttons_left)
			{
				button_list->pop_back();
				--button_index;
				button_list->push_back(boost::shared_ptr<button>(new navigation_button(FORWARD, "Next selection")));
				boost::shared_ptr<std::vector<boost::shared_ptr<button>>> new_button_list(new std::vector<boost::shared_ptr<button>>);
				(boost::dynamic_pointer_cast<navigation_button>(button_list->back()))->set_buttons_after_use(new_button_list);
				back_button_list = button_list;
				button_list = new_button_list;
			}
		}
	}
	
	return basic;
}

void button::draw_button_info(const std::string& name, const std::string& text, const std::vector<int>& prices, int honour_price, int needed_workers, int real_x)
{
	int window_height = 200;			//these variables should be used to change window proportions
	int window_width = 280;
	int x = real_x;
	int y = display_height - window_height - 2*BUTTON_SIZE; 

	al_draw_filled_rectangle(x, y, x + window_width, display_height - BUTTON_SIZE, BACKGROUND_COLOR);

	al_draw_text(font15, WRITING_COLOR, x + 10, y + 10, ALLEGRO_ALIGN_LEFT, name.c_str());

	int pictures_drawn = 0;
	for(int i=0; i<NUMBER_OF_RESOURCES; i++)
	{
		if(prices[i] > 0)
		{
			al_draw_bitmap_region(image_list[RESOURCES_IMAGE], 30*i, 0, 30, 30, x + 5 + 60*pictures_drawn, y + 45, 0);
			al_draw_textf(font15, WRITING_COLOR, x + 5 + 40 + 60*pictures_drawn, y + 50, 0, "%i", prices[i]);
			pictures_drawn++;
		}
	}
	if(honour_price > 0)
	{
		al_draw_bitmap(image_list[HONOUR_IMAGE], x + 5 + 60*pictures_drawn, y + 45, 0);
		al_draw_textf(font15, WRITING_COLOR, x + 5 + 40 + 60*pictures_drawn, y + 50, 0, "%i", honour_price);
		pictures_drawn++;
	}
	if(needed_workers > 0)
	{
		al_draw_textf(font15, WRITING_COLOR, x + 10, y + 80, 0, "Workers: %i", needed_workers);
	}


	int length_of_line = 30; //how many characters per line
	int heigth_of_line = 25;
	int start_y = 80;

	if(needed_workers > 0)
		start_y += 30;

	int current_line_length = 0;
	int line_start = 0;
	int line_number = 0;

	for(size_t i=0; i<text.size(); ++i)
	{
		if((current_line_length > length_of_line) && (text[i] == ' '))
		{
			al_draw_text(font15, WRITING_COLOR, x + 10, y + start_y + line_number*heigth_of_line, ALLEGRO_ALIGN_LEFT, 
						text.substr(line_start, current_line_length).c_str());

			current_line_length = 0;
			line_start = i + 1;
			line_number++;

		}

		if(i == text.size() - 1)
		{
			al_draw_text(font15, WRITING_COLOR, x + 10, y + start_y + line_number*heigth_of_line, ALLEGRO_ALIGN_LEFT, 
					text.substr(line_start).c_str());
		}
	
		current_line_length++;
	}
}

void button::draw_progress_bar(int start_x, int start_y, int percentage, int bar_lenght, int bar_height)
{
	al_draw_filled_rectangle(start_x, start_y, start_x + (percentage*bar_lenght)/100, start_y + bar_height, LIGHT_GREEN_COLOR);
	al_draw_rectangle(start_x, start_y, start_x + bar_lenght, start_y + bar_height, GREY_COLOR, 1);
}

/*Initialize button. Creates imaginary building which will be drawn if player will be chosing locations of his new building.*/
button_build::button_build(building_type type_of_building) : button(image_list[building_info::show_building_info(type_of_building).image], BUTTON_BUILD, false), 
		type_of_building(type_of_building), number_of_floors(building_info::show_building_info(type_of_building).number_of_floors)
{
	buildings_to_draw.push_back(building::create_building(type_of_building, 0, 0, 0, BLUE_PLAYER, false));
	
	if((type_of_building == WALL) || (type_of_building == PALISADE))
		multiple_selection = true;
}

/* Is called if there is action on tile. (Player is chosing location of his new building.)*/
game_object* button_build::draw(int tile_x, int tile_y, int surface_height, int number_of_tile)
{
	buildings_to_draw[number_of_tile]->game_object::update(tile_x, tile_y, surface_height);
	building* return_value = buildings_to_draw[number_of_tile].get();
	if(return_value == nullptr)
		throw std::exception();
	
	return return_value;
}

bool button_build::panel_click()
{
	if(session->unlocked_buildings[type_of_building])
		return true;
	
	int price = building_info::show_building_info(type_of_building).honour_price;
	if(session->honour >= price)
	{
		session->unlocked_buildings[type_of_building] = true;
		if((type_of_building == LEFT_GATE) || (type_of_building == RIGHT_GATE))
		{
			session->unlocked_buildings[RIGHT_GATE] = true;
			session->unlocked_buildings[LEFT_GATE] = true;
		}
		else if((type_of_building == NORTHWEST_STAIRS) || (type_of_building == NORTHEAST_STAIRS) 
					|| (type_of_building == SOUTHEAST_STAIRS) || (type_of_building == SOUTHWEST_STAIRS))
		{
			session->unlocked_buildings[NORTHWEST_STAIRS] = true;
			session->unlocked_buildings[NORTHEAST_STAIRS] = true;
			session->unlocked_buildings[SOUTHEAST_STAIRS] = true;
			session->unlocked_buildings[SOUTHWEST_STAIRS] = true;
		}
		else if((type_of_building == NORTHWEST_TOWER) || (type_of_building == NORTHEAST_TOWER) 
					|| (type_of_building == SOUTHEAST_TOWER) || (type_of_building == SOUTHWEST_TOWER))
		{
			session->unlocked_buildings[NORTHWEST_TOWER] = true;
			session->unlocked_buildings[NORTHEAST_TOWER] = true;
			session->unlocked_buildings[SOUTHEAST_TOWER] = true;
			session->unlocked_buildings[SOUTHWEST_TOWER] = true;
		}
		session->honour -= price;
	}

	return false;
}

/*Builds building on tile, if building can be  build there.*/
void button_build::map_click()
{
	for(size_t i=0; i<tiles_with_action.size(); ++i)
	{
		int tile_x = tiles_with_action[i]->show_tile_x();
		int tile_y = tiles_with_action[i]->show_tile_y();

		can_build_output output = building::can_build_here(session->tile_list[tile_y][tile_x].get(), buildings_to_draw[0]->show_type());

		if(output == can_build_output::CAN_BUILD)
			session->tile_list[tile_y][tile_x]->build(type_of_building, BLUE_PLAYER);

		else
		{
			std::tuple<can_build_output, std::string> t = find<hint_database, 0>(output);
			std::string message = std::get<1>(t);
			session->hints.add_hint(compute_game_x(tile_x, tile_y), compute_game_y(tile_x, tile_y), message, LIGHT_RED_COLOR);
		}
	}
}

/*Draws imaginary building or sets action_on_tile so that imaginary building will be drawn by draw_map() function.*/ 
void button_build::draw_action(int screen_position_x, int screen_position_y)
{
	for(size_t i=0; i<tiles_with_action.size(); ++i)		//if !multiple_selection, then tiles_with_action.size() should be 1 or 0.
	{
		int tile_x = tiles_with_action[i]->show_tile_x();
		int tile_y = tiles_with_action[i]->show_tile_y();

		if(building::can_build_here(session->tile_list[tile_y][tile_x].get(), buildings_to_draw[i]->show_type()) == can_build_output::CAN_BUILD)
			buildings_to_draw[i]->draw_green = true;
		else
			buildings_to_draw[i]->draw_green = false;
		
		session->tile_list[tile_y][tile_x]->action_on_tile = shared_from_this();
		session->tile_list[tile_y][tile_x]->number_of_tile_with_action = i;
	}
}

/*Called if mouse wheel moves. Rotates gate.*/
int button_build::scroll()
{
	switch(type_of_building)
	{
		case(LEFT_GATE):
		{
			type_of_building = RIGHT_GATE;
			buildings_to_draw[0]->type = RIGHT_GATE;
			buildings_to_draw[0]->size = RIGHT_GATE_BUILDING;
		}
		break;
		case(RIGHT_GATE):
		{
			type_of_building = LEFT_GATE;
			buildings_to_draw[0]->type = LEFT_GATE;
			buildings_to_draw[0]->size = LEFT_GATE_BUILDING;
		}
		break;
		case(NORTHWEST_STAIRS):
		{
			type_of_building = NORTHEAST_STAIRS;
			buildings_to_draw[0]->type = NORTHEAST_STAIRS;
		}
		break;
		case(NORTHEAST_STAIRS):
		{
			type_of_building = SOUTHEAST_STAIRS;
			buildings_to_draw[0]->type = SOUTHEAST_STAIRS;
		}
		break;
		case(SOUTHEAST_STAIRS):
		{
			type_of_building = SOUTHWEST_STAIRS;
			buildings_to_draw[0]->type = SOUTHWEST_STAIRS;
		}
		break;
		case(SOUTHWEST_STAIRS):
		{
			type_of_building = NORTHWEST_STAIRS;
			buildings_to_draw[0]->type = NORTHWEST_STAIRS;
		}
		break;	
		case(NORTHWEST_TOWER):
		{
			type_of_building = NORTHEAST_TOWER;
			buildings_to_draw[0]->type = NORTHEAST_TOWER;
		}
		break;
		case(NORTHEAST_TOWER):
		{
			type_of_building = SOUTHEAST_TOWER;
			buildings_to_draw[0]->type = SOUTHEAST_TOWER;
		}
		break;
		case(SOUTHEAST_TOWER):
		{
			type_of_building = SOUTHWEST_TOWER;
			buildings_to_draw[0]->type = SOUTHWEST_TOWER;
		}
		break;
		case(SOUTHWEST_TOWER):
		{
			type_of_building = NORTHWEST_TOWER;
			buildings_to_draw[0]->type = NORTHWEST_TOWER;
		}
		break;
		default: {}
	}

	return 0;
}

/*Draws information about building which is build by this button.*/
void button_build::draw_info(int button_number)
{
	bool unlocked = session->unlocked_buildings[type_of_building];
	building_info info = building_info::show_building_info(type_of_building);
	
	if(unlocked)
	{
		draw_button_info(info.name, info.text, info.building_price, 0, info.number_of_workers, button_number*BUTTON_SIZE);
	}
	else
	{
		draw_button_info("Unlock " + info.name, info.text, std::vector<int>(NUMBER_OF_RESOURCES, 0), info.honour_price, 0, button_number*BUTTON_SIZE);
	}
}

void button_build::draw_button(int button_number)
{
	bool unlocked = session->unlocked_buildings[type_of_building];
	
	al_draw_filled_rectangle(button_number*BUTTON_SIZE + 5, display_height - BUTTON_SIZE + 5, (button_number + 1)*BUTTON_SIZE -5 , display_height - 5, BLACK_COLOR);		//draws black rectangle
	if(unlocked)
	{
		al_draw_bitmap_region(action_image, 0, 0, 64, 64, button_number*BUTTON_SIZE + 8, display_height - BUTTON_SIZE + 8, 0);	//draws button image
	}
	else
	{
		al_draw_bitmap_region(image_list[LOCK_IMAGE], 0, 0, 64, 64, button_number*BUTTON_SIZE + 8, display_height - BUTTON_SIZE + 8, 0);	
	}

	if(button_number == 9)		//draws number of action 
		al_draw_textf(font15, WRITING_COLOR, button_number*BUTTON_SIZE + 72, display_height - 23, ALLEGRO_ALIGN_RIGHT, "%i", 0);
	else if(button_number < 9)
		al_draw_textf(font15, WRITING_COLOR, button_number*BUTTON_SIZE + 72, display_height - 23, ALLEGRO_ALIGN_RIGHT, "%i", button_number + 1);
}
	


int button_build::update_tiles_with_action(bool mouse_button_down, int tile_x, int tile_y, int button_down_tile_x, int button_down_tile_y)
{
	general_update_tiles_with_action(mouse_button_down, tile_x, tile_y, button_down_tile_x, button_down_tile_y);
	
	if((buildings_to_draw.size() < tiles_with_action.size()) && (multiple_selection))
	{
		for(size_t i=buildings_to_draw.size(); i<tiles_with_action.size(); ++i)
		{
			if((type_of_building == WALL) || (type_of_building == PALISADE))
			{
				buildings_to_draw.push_back(building::create_building(type_of_building, 0, 0, 0, BLUE_PLAYER, false));
			}
		}
	}
	return 0;
}

void button_build::reset_tiles_with_action()
{
	button::reset_tiles_with_action();
}

button_path::button_path(bool add) : button(image_list[PATHS_IMAGE], BUTTON_PATH, true)
{
	this->add = add;
}

game_object* button_path::draw(int tile_x, int tile_y, int surface_height, int number_of_tile)
{
	return NULL;
}

void button_path::map_click()
{
	LOG("button path::map_click(), first x: " << tiles_with_action[0]->show_tile_x() << " first y: " << tiles_with_action[0]->show_tile_y());

	for(size_t i=0; i<tiles_with_action.size(); ++i)
	{
		if(add)
			tiles_with_action[i]->add_path(true);
		
		else if(!add)
		{
			if(tiles_with_action[i]->building_on_tile.expired())	//neccessary to prevent removing path under gate
				tiles_with_action[i]->remove_path(true);
		}
	}
}

void button_path::draw_action(int screen_position_x, int screen_position_y)
{
	for(size_t i=0; i<tiles_with_action.size(); ++i)
	{
		tiles_with_action[i]->action_on_tile = shared_from_this();
		tiles_with_action[i]->number_of_tile_with_action = i;
		tiles_with_action[i]->add_path(false);
	}
}

void button_path::draw_info(int button_number)
{
	std::string name;
	std::string text;
	std::vector<int> prices(NUMBER_OF_RESOURCES, 0);

	prices[STONE] = 0;
	prices[WOOD] = 0;

	if(add)
	{
		name.append("BUILD PATH");
		text.append("Your buildings should be connected by paths to allow transport of resources.");
	}
	else
	{
		name.append("DELETE PATH");
		text.append("Remove path to allow buildings on tile.");
	}

	draw_button_info(name, text, prices, 0, 0, button_number*BUTTON_SIZE);
}

void button_path::draw_button(int button_number)
{
	al_draw_filled_rectangle(button_number*BUTTON_SIZE + 5, display_height - BUTTON_SIZE + 5, (button_number + 1)*BUTTON_SIZE -5 , display_height - 5, BLACK_COLOR);		//draws black rectangle
	
	if(add)
		al_draw_bitmap_region(action_image, 0, 0, 64, 64, button_number*BUTTON_SIZE + 8, display_height - BUTTON_SIZE + 8, 0);	//draws button image
	else
		al_draw_bitmap_region(action_image, 0, 64, 64, 64, button_number*BUTTON_SIZE + 8, display_height - BUTTON_SIZE + 8, 0);	//draws button image
			

	if(button_number == 9)		//draws number of action 
		al_draw_textf(font15, WRITING_COLOR, button_number*BUTTON_SIZE + 72, display_height - 23, ALLEGRO_ALIGN_RIGHT, "%i", 0);
	else if(button_number < 9)
		al_draw_textf(font15, WRITING_COLOR, button_number*BUTTON_SIZE + 72, display_height - 23, ALLEGRO_ALIGN_RIGHT, "%i", button_number + 1);
}

void button_path::reset_tiles_with_action()
{
	for(size_t i=0; i<tiles_with_action.size(); ++i)
	{
		if(tiles_with_action[i]->unreal_path_on_tile())
		{
			tiles_with_action[i]->remove_path(false);
		}
	}
	
	button::reset_tiles_with_action();
}


navigation_button::navigation_button(navigation_button_type navigation_type, const std::string& button_name) 
	: button(image_list[NAVIGATION_BUTTONS_IMAGE], NAVIGATION_BUTTON, false), navigation_type(navigation_type), name(button_name)
{	
}

void navigation_button::draw_button(int button_number)
{
	al_draw_bitmap_region(image_list[NAVIGATION_BUTTONS_IMAGE], ((int)navigation_type)*70, 0, 70, 70, button_number*BUTTON_SIZE + 5, display_height - BUTTON_SIZE + 5, 0);
}

void navigation_button::draw_info(int button_number)
{
	button::draw_button_info(name, "", std::vector<int>(NUMBER_OF_RESOURCES, 0), 0, 0, BUTTON_SIZE * button_number);
}
	




