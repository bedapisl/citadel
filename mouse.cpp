#include "core.h"

extern std::ofstream log_file;
extern int display_height;
extern int display_width;
extern ALLEGRO_FONT* font15;
extern game_session* session;

/*Handles press of mouse left button. To work well it needs actual information in mouse->state.*/
void game_mouse::left_button_go_down(int &screen_position_x, int &screen_position_y, rotation* rotate)
{
	LOG("left button down");
	is_left_button_down = true;
		
	std::pair<int, int> mouse_location = find_mouse_location(screen_position_x, screen_position_y);

	button_down_game_x = state->x + screen_position_x;
	button_down_game_y = state->y + screen_position_y;
	button_down_tile_x = mouse_location.first;
	button_down_tile_y = mouse_location.second;

	if(window::active_windows_function_click(state->x, state->y))
	{
		return;		//click handled some window
	}

	if(state->y < display_height - BUTTON_SIZE)			//click to main game screen
	{
	
		if(chosen_button.expired())
		{	
			chosen_people.clear();
			chosen_building.reset();
			chosen_button.reset();
		}
	}
	else if(state->y > display_height - BUTTON_SIZE)		//click to down menu
	{	
		if((chosen_building.expired()) && (chosen_people.size() == 0))
		{
			if((state->x > display_width - 2.5*BUTTON_SIZE) & (state->x < display_width - 2*BUTTON_SIZE))	//click to rotate buttons
			{
				if(state->y < display_height - BUTTON_SIZE/2)
					*rotate = LEFT_ROTATION; 
				else
					*rotate = RIGHT_ROTATION;
			}
			else if(state->x < display_width - 2.5*BUTTON_SIZE)		//click to buttons
			{
				for(int i=0; i<(*button::button_list).size(); i++)
				{
					if((state->x < (i+1)*BUTTON_SIZE) & (state->x > i*BUTTON_SIZE))
					{
						choose_button((*button::button_list)[i]);
					}
				}
			}
		}
		else if(!chosen_building.expired())
		{
			boost::shared_ptr<building> chosen_building_ptr = chosen_building.lock();
			if(state->x < display_width - 2*BUTTON_SIZE)
			{
				chosen_building_ptr->function_click(state->x, state->y);
			}
		}
		else if(chosen_people.size() > 0)
		{
			int people_index = state->x / BUTTON_SIZE;
			if(chosen_people.size() > people_index)
			{
				boost::weak_ptr<people> people_ptr = chosen_people[people_index];
				chosen_people.clear();
				chosen_people.push_back(people_ptr);
			}
		}
	}
	return;
}
/*Handles release of left mouse button. Choses objects. */
void game_mouse::left_button_go_up(int screen_position_x, int screen_position_y)
{
	LOG("left button up");
	is_left_button_down = false;
	al_get_mouse_state(state);
	if((!chosen_button.expired()) && (state->y < display_height - BUTTON_SIZE))	//click in main area
	{
		if(chosen_button.expired())
			LOG("error");

		else
		{
			boost::shared_ptr<button> chosen_button_ptr = chosen_button.lock();
			chosen_button_ptr->map_click();
		}
	}
	else if(!chosen_button.expired())
	{

	}
	else if(((button_down_game_x > -10000) && (button_down_game_y > -10000)) && (state->y < display_height - BUTTON_SIZE))
	{
		int max_marking_x;							//this is for chosing right people
		int min_marking_x;
		int max_marking_y;
		int min_marking_y;

		if(button_down_game_x > state->x + screen_position_x)
		{
			max_marking_x = button_down_game_x;
			min_marking_x = state->x + screen_position_x;
		}
		else
		{
			max_marking_x = state->x + screen_position_x;
			min_marking_x = button_down_game_x;
		}

		if(button_down_game_y > state->y + screen_position_y)
		{
			max_marking_y = button_down_game_y;
			min_marking_y = state->y + screen_position_y;
		}
		else
		{
			max_marking_y = state->y + screen_position_y;
			min_marking_y = button_down_game_y;
		}
		for(int i=0; i<session->people_list.size(); i++)
		{
			if((session->people_list[i]->show_game_x() < max_marking_x + MARKED_LOCATION) & (session->people_list[i]->show_game_x() > min_marking_x - MARKED_LOCATION))
			{
				int drawing_y = session->people_list[i]->show_game_y() - session->people_list[i]->show_surface_height()*32 + 32;
				if((drawing_y < max_marking_y + MARKED_LOCATION) & (drawing_y > min_marking_y - MARKED_LOCATION))
				{
					if((session->people_list[i]->show_owner() == BLUE_PLAYER) && (session->people_list[i]->show_type() != CARRIER))
					{
						chosen_people.push_back(session->people_list[i]);
						chosen_building.reset();
						chosen_button.reset();
					}
				}
			}
		}
	
		if(chosen_building.expired() && chosen_button.expired() && chosen_people.empty())
		{
			if(((tile_x >= 0) & (tile_x < game_info::map_width)) & ((tile_y >= 0) & (tile_y < game_info::map_height)))
			{
				boost::shared_ptr<tile> t = session->tile_list[tile_y][tile_x];
				t->check_death_people_on_tile();

				if(!t->building_on_tile.expired())
				{
					chosen_building = t->building_on_tile;
					LOG("choosing building");
				}
				else if(!t->people_on_tile.empty())
				{
					for(int i=0; i<t->people_on_tile.size(); ++i)
					{
						boost::shared_ptr<people> p = session->tile_list[tile_y][tile_x]->people_on_tile[i].lock();
						chosen_people.push_back(p);
					}
				}
			}
		}
		button_down_game_x = 0;
		button_down_game_y = 0;
	}
}

/*Handles press of right mouse button, which is used to control chosen people.*/
void game_mouse::right_button_go_down()
{
	LOG("right button down");
	is_right_button_down = true;
	if(!chosen_people.empty())
	{
		check_death();
		if(((tile_x > -1) & (tile_x < game_info::map_width)) & ((tile_y > -1) & (tile_y < game_info::map_height)))
		{
			tile* target_tile = session->tile_list[tile_y][tile_x].get();
			
			target_tile->check_death_people_on_tile();
			if((target_tile->people_on_tile.size() > 0) && (target_tile->people_on_tile[0].lock()->show_owner() != BLUE_PLAYER))
			{
				for(int i=0; i<chosen_people.size(); ++i)
				{	
					boost::shared_ptr<people> ptr = chosen_people[i].lock();
					if((ptr->show_owner() == BLUE_PLAYER) && (ptr->show_type() != CARRIER))
					{
						boost::shared_ptr<warrior> warrior_ptr = boost::dynamic_pointer_cast<warrior>(ptr);
						warrior_ptr->add_target(boost::shared_ptr<target>(new target(target_tile->people_on_tile[0], target_priority::PLAYER_ORDER)));
					}
				}
			}
			
			else if((!target_tile->building_on_tile.expired()) && (target_tile->building_on_tile.lock()->show_owner() != BLUE_PLAYER))
			{
				for(int i=0; i<chosen_people.size(); ++i)
				{	
					boost::shared_ptr<people> ptr = chosen_people[i].lock();
					if((ptr->show_owner() == BLUE_PLAYER) && (ptr->show_type() != CARRIER))
					{
						boost::shared_ptr<warrior> warrior_ptr = boost::dynamic_pointer_cast<warrior>(ptr);
						warrior_ptr->add_target(boost::shared_ptr<target>(new target(target_tile->building_on_tile, target_priority::PLAYER_ORDER)));
					}
				}
			}
			else
			{
				std::vector<tile*> target_tiles = pathfinding::near_accessible_tiles(target_tile, chosen_people.size());
				for(int i=0; i<chosen_people.size(); ++i)
				{	
					boost::shared_ptr<people> ptr = chosen_people[i].lock();
					if((ptr->show_owner() == BLUE_PLAYER) && (ptr->show_type() != CARRIER))
					{
						boost::shared_ptr<warrior> warrior_ptr = boost::dynamic_pointer_cast<warrior>(ptr);
						warrior_ptr->add_target(boost::shared_ptr<target>(new target(target_tiles[i], target_priority::PLAYER_ORDER)));
					}
				}
			}
		}
	}
	if((!chosen_button.expired()) || (!chosen_building.expired()))
	{
		chosen_people.clear();
		chosen_building.reset();
		chosen_button.reset();
	}
}

void game_mouse::right_button_go_up()
{
	LOG("right button go up");
	is_right_button_down = false;
	if(!chosen_button.expired())
	{
		unchoose_button();
	}
}	

game_mouse::game_mouse()
{
	state = new ALLEGRO_MOUSE_STATE;
	is_left_button_down = false;
	is_right_button_down = false;
	scrolling = false;
}

game_mouse::~game_mouse()
{
	delete state;
}

int game_mouse::draw_mouse(int screen_position_x, int screen_position_y)
{
	LOG("drawing mouse");
		//draws life bar of objects under cursor
	if(((tile_x >= 0) & (tile_x < game_info::map_width)) & ((tile_y >= 0) & (tile_y < game_info::map_height)))
	{
		tile* t = session->tile_list[tile_y][tile_x].get();
		t->check_death_people_on_tile();
		
		if(!t->building_on_tile.expired())
			t->building_on_tile.lock()->draw_life_bar(screen_position_x, screen_position_y);

		else if(!t->people_on_tile.empty())
		{
			for(int i=0; i<t->people_on_tile.size(); ++i)
			{
				t->people_on_tile[i].lock()->draw_life_bar(screen_position_x, screen_position_y);
			}
		}
	}
		//draws life bar of chosen objects
	if(!chosen_building.expired())
		chosen_building.lock()->draw_life_bar(screen_position_x, screen_position_y);

	for(int i=0; i<chosen_people.size(); i++)
	{
		if(!chosen_people[i].expired())
			chosen_people[i].lock()->draw_life_bar(screen_position_x, screen_position_y);
	}
	
	if((state->buttons & 1) && (state->y < display_height - BUTTON_SIZE)) 
	{
		if(chosen_button.expired())		//draw rectangle to chose people 
		{
			if((button_down_game_x > 0) || (button_down_game_y > 0))
				al_draw_rectangle(button_down_game_x - screen_position_x, button_down_game_y - screen_position_y, state->x , state->y, GREY_COLOR, 1);
		}
	}

	return 0;
}

int game_mouse::move(int &screen_position_x, int &screen_position_y)
{
	LOG("mouse move");
	al_get_mouse_state(state);
	if(mouse_wheel_position != state->z)
	{
		mouse_wheel_position = state->z;
		if(!chosen_button.expired())
			chosen_button.lock()->scroll();
	}

	if(state->y > display_height - BUTTON_SIZE)		//in panel
	{	
		if((state->x > display_width - 2*BUTTON_SIZE) && (state->buttons & 1)) 	//button hold in minimap
		{
			float size_increase = (float)BUTTON_SIZE / (float)((game_info::map_width > MAP_HEIGHT) ? game_info::map_width : game_info::map_height); 
			screen_position_x = - ((display_width - BUTTON_SIZE*2 - state->x)/size_increase)*32 - display_width/2;
			screen_position_y = - ((display_height - BUTTON_SIZE - state->y)/size_increase)*32 - display_height/2;
		}
	}
	else							//click in map - find tile where is mouse
	{
		std::pair<int, int> mouse_location = find_mouse_location(screen_position_x, screen_position_y);
		tile_x = mouse_location.first;
		tile_y = mouse_location.second;
	
		if((tile_x < 0) || (tile_y < 0))
			throw std::exception();
		
		if(!session->tile_list[tile_y][tile_x]->building_on_tile.expired())
		{
			session->tile_list[tile_y][tile_x]->building_on_tile.lock()->draw_selection = true;
		}
	}

	if(!chosen_button.expired())
		chosen_button.lock()->update_tiles_with_action(is_left_button_down, tile_x, tile_y, button_down_tile_x, button_down_tile_y);

	return 0;
}

//Returns tile_x and tile_y of current mouse location
std::pair<int, int> game_mouse::find_mouse_location(int screen_position_x, int screen_position_y)
{
	al_get_mouse_state(state);

	int mouse_game_x = state->x + screen_position_x;
	int mouse_game_y = state->y + screen_position_y;

	int lowest_tile_x = compute_tile_x(mouse_game_x, mouse_game_y);
	int lowest_tile_y = compute_tile_y(mouse_game_x, mouse_game_y);

	if((lowest_tile_x >= game_info::map_width) || (lowest_tile_y >= game_info::map_height))		//under the map
	{
		return std::pair<int, int>(std::min(lowest_tile_x, game_info::map_width - 1), std::min(lowest_tile_y, game_info::map_height - 1));
	}

	std::vector<std::pair<int, int>> possible_tiles;
	for(int i=0; i <= game_object::highest_surface; ++i)
	{
			
		possible_tiles.push_back(std::pair<int, int>(lowest_tile_x, lowest_tile_y));
		possible_tiles.push_back(std::pair<int, int>(lowest_tile_x + 1, lowest_tile_y));
		possible_tiles.push_back(std::pair<int, int>(lowest_tile_x, lowest_tile_y + 1));
		lowest_tile_x++;
		lowest_tile_y++;
	}
	
	std::pair<int, int> founded_tile;
	bool founded = false;
	bool secondary_founded = false;		//if mouse is not on top of tile but on side of tile
	for(auto coordinates : possible_tiles)
	{
		int x = coordinates.first;
		int y = coordinates.second;
	
		if(((x >= 0) && (x < game_info::map_width)) && ((y >= 0) && (y < game_info::map_height)))
		{
			tile* t = session->tile_list[y][x].get();
				
				//correct tile? (maybe it should also take in account buildings on this tile
			if(std::abs(mouse_game_x - t->show_game_x()) + 2*(std::abs(mouse_game_y - t->show_game_y() + t->show_surface_height() * 32)) <= 32)
			{
				founded_tile = coordinates;
				founded = true;
				break;
			}

			else if((std::abs(mouse_game_x - t->show_game_x()) <= 32) && (mouse_game_y >= t->show_game_y() - t->show_surface_height() * 32) && (!founded))
			{
				secondary_founded = true;
				founded_tile = coordinates;
				//no break because I want the last one (it should be nearest to mouse location)
			}
		}
	}

	if((!founded) && (!secondary_founded))
	{
		int x = possible_tiles[0].first;
		int y = possible_tiles[0].second;

		if((x < 0) && (y < 0))
			return std::pair<int, int>(0, 0);
		
		else if(x < 0)
			return std::pair<int, int>(0, y);

		else if(y < 0)
			return std::pair<int, int>(x, 0);
				
		else
			throw std::exception();
	}
	return founded_tile;
}

/* Removes pointers to dead object from game_mouse, so dead object can be deleted.*/
int game_mouse::check_death()
{
	if((!chosen_building.expired()) && (chosen_building.lock()->is_death()))
	{	
		LOG("unchoosing building");
		chosen_building.reset();
	}

	std::vector<boost::weak_ptr<people>>::iterator it = remove_if(chosen_people.begin(), chosen_people.end(), 
								[](boost::weak_ptr<people> p){return (p.expired() || p.lock()->is_death());});
	chosen_people.erase(it, chosen_people.end());
	return 0;
}
/*Calls button::draw_info() for right button.*/
int game_mouse::draw_button_info()
{
	int x = state->x;
	int y = state->y;
	int button_number = x / BUTTON_SIZE;

	if((y > display_height - BUTTON_SIZE) && (x < (*button::button_list).size()*BUTTON_SIZE))
	{
		if((chosen_people.empty()) && (chosen_building.expired()))
			(*button::button_list)[button_number]->draw_info(button_number);
		
		else if(!chosen_building.expired())
			chosen_building.lock()->draw_function_info(x, y);
	}
	return 0;
}

int game_mouse::choose_button(boost::shared_ptr<button> button_to_choose)
{
	bool choose = button_to_choose->panel_click();
	if(choose)
	{
		chosen_people.clear();
		chosen_building.reset();
		chosen_button = button_to_choose;
	}
	return 0;
}

int game_mouse::unchoose_button()
{
	if(!chosen_button.expired())
	{
		chosen_button.reset();
		chosen_building.reset();
		chosen_people.clear();
	}
	return 0;
}

