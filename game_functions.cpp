#include "core.h"

extern std::ofstream log_file; 

extern int display_width;
extern int display_height;
extern ALLEGRO_BITMAP** image_list;
extern ALLEGRO_FONT* font15;
extern ALLEGRO_FONT* font20;
extern ALLEGRO_FONT* font25;
extern ALLEGRO_FONT* font30;
extern game_session* session;

/*Draws game map and all objects except missiles. Often slows game, but is quite optimized and very complicated so dont rewrite it if you dont have lots of time. */
int draw_map(int screen_position_x, int screen_position_y, game_mouse* mouse)
{
	LOG("start drawing map");
	
	al_clear_to_color(BLACK_COLOR);
	
	al_hold_bitmap_drawing(true);

	if(!mouse->chosen_button.expired())
	{
		boost::shared_ptr<button> chosen_button_ptr = mouse->chosen_button.lock();

		if(chosen_button_ptr)
			chosen_button_ptr->draw_action(screen_position_x, screen_position_y);//, chosen_button_ptr);
	}


	int x1 = compute_tile_x(0,0, screen_position_x, screen_position_y) - 3;
	int y1 = compute_tile_y(0,0, screen_position_x, screen_position_y);
	int x2 = compute_tile_x(display_width, display_height - BUTTON_SIZE, screen_position_x, screen_position_y) + 5 + game_object::highest_surface;
	int y2 = compute_tile_y(display_width, display_height - BUTTON_SIZE, screen_position_x, screen_position_y) + 3 + game_object::highest_surface;
	
	std::vector<tile*> tiles_to_check = tiles_in_rectangle(x1, y1, x2, y2);

	std::vector<game_object*> objects_to_draw;
	std::vector<game_object*> next_objects_to_draw;

	for(int i=0; i<tiles_to_check.size(); i++)
	{
		objects_to_draw.push_back(tiles_to_check[i]);
	}
	
	int pictures_drawn = 0;
	int surface = 0;
	
	while(objects_to_draw.size() > 0)
	{
		for(int i=0; i<objects_to_draw.size(); i++)
		{
			if(objects_to_draw[i]->show_visible() == false)
			{
				//cout << objects_to_draw[i]->show_tile_x() << " " << objects_to_draw[i]->show_tile_y() << endl;
			}
			if(objects_to_draw[i]->show_current_drawing_height() > surface)
			{
				if(objects_to_draw[i]->show_visible() & (surface > 0))
				{
					int x = objects_to_draw[i]->show_tile_x();
					int y = objects_to_draw[i]->show_tile_y();
					bool draw = false;
					if((objects_to_draw[i]->show_tile_x() == game_info::map_width - 1) || (objects_to_draw[i]->show_tile_y() == game_info::map_height - 1))
						draw = true;

					else if((session->tile_list[y + 1][x]->show_current_drawing_height() <= surface) || (session->tile_list[y][x + 1]->show_surface_height() <= surface))
					{
						draw = true;
					}
					
					if((draw) && (objects_to_draw[i]->type_of_object == TILE))
					{
						int drawing_x = compute_game_x(x, y) - screen_position_x - 32;
						int drawing_y = compute_game_y(x, y) - screen_position_y - 32*surface - 16;
						al_draw_bitmap_region(image_list[GRASS_IMAGE], 64, 0, 64, 64, drawing_x, drawing_y, 0);
						pictures_drawn++;
					}
				}
				
				next_objects_to_draw.push_back(objects_to_draw[i]);
			}
			else if(objects_to_draw[i]->show_current_drawing_height() == surface)
			{	
				pictures_drawn++;
				std::vector<game_object*> objects = objects_to_draw[i]->draw(screen_position_x, screen_position_y);
				for(int j=0; j<objects.size(); ++j)
				{
					next_objects_to_draw.push_back(objects[j]);
					if(objects[j] == NULL)
						throw std::exception();
				}
			}
		}
		swap(objects_to_draw, next_objects_to_draw);
		next_objects_to_draw.clear();
		surface++;
	}
	
	al_hold_bitmap_drawing(false);

	session->hints.draw_and_update(screen_position_x, screen_position_y);

	LOG("drawn " << pictures_drawn << " pictures.");

	LOG("drawing finished");	

	return 0;
}

/*Moves map if player press arrow key or if mouse is near end of display. Moves of map done by clicking in minimap are handled by draw_and_update_mouse() function in mouse.cpp*/
int screen_move(ALLEGRO_MOUSE_STATE *mouse_state, ALLEGRO_KEYBOARD_STATE* keyboard_state, int & screen_position_x, int & screen_position_y)
{
	al_get_mouse_state(mouse_state);		//movement by mouse
	if((mouse_state->x > display_width - 15) && (screen_position_x < compute_game_x(game_info::map_width, 0) + 200 - display_width ))
		(screen_position_x) += DISPLAY_MOVEMENT_SPEED;		//move right

	else if((mouse_state->x < 15) && (screen_position_x > -200))		//move left
		(screen_position_x) -= DISPLAY_MOVEMENT_SPEED;

	if((mouse_state->y > display_height - 15) && (screen_position_y < BUTTON_SIZE - display_height + compute_game_y(game_info::map_width, game_info::map_height) + 200))
		(screen_position_y) += DISPLAY_MOVEMENT_SPEED;		//down

	else if((mouse_state->y < 15) && (screen_position_y > -200)) 		//up
		(screen_position_y) -= DISPLAY_MOVEMENT_SPEED;

	
	al_get_keyboard_state(keyboard_state);		//movement by keys
	if(al_key_down(keyboard_state, ALLEGRO_KEY_RIGHT) || al_key_down(keyboard_state, ALLEGRO_KEY_D))
		(screen_position_x) += DISPLAY_MOVEMENT_SPEED;

	if(al_key_down(keyboard_state, ALLEGRO_KEY_LEFT) || al_key_down(keyboard_state, ALLEGRO_KEY_A))
		(screen_position_x) -= DISPLAY_MOVEMENT_SPEED;
	
	if(al_key_down(keyboard_state, ALLEGRO_KEY_DOWN) || al_key_down(keyboard_state, ALLEGRO_KEY_S))
		(screen_position_y) += DISPLAY_MOVEMENT_SPEED;

	if(al_key_down(keyboard_state, ALLEGRO_KEY_UP) || al_key_down(keyboard_state, ALLEGRO_KEY_W))
		(screen_position_y) -= DISPLAY_MOVEMENT_SPEED;

	return 0;
}
/*Returns x coordinate of tile (it's index in session->tile_list). Drawing_x and y are real values (0 - display_width or height) which describes position on display, screen_position_x and y describe position of map relative to upper left corner of display.*/ 
int compute_tile_x(int drawing_x, int drawing_y, int screen_position_x, int screen_position_y)
{
	int modulo = (2*(drawing_y + screen_position_y) + drawing_x + screen_position_x - game_info::map_height*32 + 64) % 64;
	int tile_x = (2*(drawing_y + screen_position_y) + drawing_x + screen_position_x - game_info::map_height*32 + 64)/64;
	if(modulo < 0)			//because of rouding negative values
		tile_x--;
	return tile_x;
}
		

int compute_tile_y(int drawing_x, int drawing_y, int screen_position_x, int screen_position_y)
{
	int modulo = (2*(drawing_y + screen_position_y) - (drawing_x + screen_position_x) + game_info::map_height*32) % 64;
	int tile_y = (2*(drawing_y + screen_position_y) - (drawing_x + screen_position_x) + game_info::map_height*32)/64;
	if(modulo < 0)
		tile_y--;
	return tile_y;
}
/*Returns x coordinate of tile (it's index in session->tile_list) given it's real coordinates relative to upper left corner of map, without counting surface height.*/  
int compute_tile_x(int game_x, int game_y)
{
	int modulo = (2*game_y + game_x - 32*game_info::map_height) % 64;
	int tile_x = (2*game_y + game_x - 32*game_info::map_height)/64;
	if(modulo < 0)
		tile_x--;
	return tile_x;
}

int compute_tile_y(int game_x, int game_y)
{
	int modulo = (2*game_y - game_x + 32*game_info::map_height) % 64;
	int tile_y = (2*game_y - game_x + 32*game_info::map_height)/64;
	if(modulo < 0)
		tile_y--;
	return tile_y;
}

/*Draws panel with buttons and minimap. Also draws happiness, time to next invasion and numbers of idles/inhabitants. */ 
int draw_main_panel(ALLEGRO_BITMAP* minimap, int screen_position_x, int screen_position_y, game_mouse* mouse, game_session* session)
{	
	LOG("drawing main panel");
	
	for(int i = 0; i*256 < display_width; i++)
	{
		al_draw_bitmap_region(image_list[TEXTURE_GREY_IMAGE], 0,0, 256, BUTTON_SIZE, i*256, display_height - BUTTON_SIZE, 0);
	}
	
	mouse->draw_mouse(screen_position_x, screen_position_y);

	mouse->draw_button_info();
	mouse->check_death();
	
	if(mouse->chosen_building.lock())
	{
		mouse->chosen_building.lock()->draw_interface();
		mouse->chosen_building.lock()->draw_function_info(mouse->mouse_x(), mouse->mouse_y());
	}
	else if(mouse->chosen_people.size() > 0)
	{
		if(mouse->chosen_people.size() == 1)
		{
			mouse->chosen_people[0].lock()->draw_interface();
		}
		else if(mouse->chosen_people.size() > 1)
		{
			for(int i=0; i<mouse->chosen_people.size(); ++i)
			{
				mouse->chosen_people[i].lock()->draw_partial_interface(i);
			}
		}
	}
	else
	{
		al_draw_bitmap(image_list[ROTATE_LEFT_IMAGE], display_width - 3*BUTTON_SIZE, display_height - BUTTON_SIZE/2 + 3, 0);
		al_draw_bitmap(image_list[ROTATE_RIGHT_IMAGE], display_width - 2.5*BUTTON_SIZE, display_height - BUTTON_SIZE/2 + 3, 0);
		al_draw_filled_rectangle(display_width - 3*BUTTON_SIZE, display_height - BUTTON_SIZE + 8, display_width - 2*BUTTON_SIZE - 8, display_height - BUTTON_SIZE/2 - 4, BLACK_COLOR);
		al_draw_text(font20, WRITING_COLOR, display_width - 2.5*BUTTON_SIZE, display_height - BUTTON_SIZE + 12, ALLEGRO_ALIGN_CENTRE, "Menu"); 
		for(int i=0; i<(*button::button_list).size(); i++)
		{
			(*button::button_list)[i]->draw_button(i);
		}
	}

	al_draw_bitmap(minimap, display_width - 2*BUTTON_SIZE, display_height - BUTTON_SIZE, 0);

	float size_increase = (float)BUTTON_SIZE / (float)((game_info::map_width > game_info::map_height) ? game_info::map_width : game_info::map_height); 

	int x1 = display_width - BUTTON_SIZE*2 + (screen_position_x*size_increase)/32;
	int y1 = display_height - BUTTON_SIZE + (screen_position_y*size_increase)/32;
	int x2 = x1 + (display_width*size_increase)/32;
	int y2 = y1 + ((display_height-BUTTON_SIZE)*size_increase)/32;

	if(x1 < display_width - BUTTON_SIZE*2)
		x1 = display_width - BUTTON_SIZE*2;
	if(y1 < display_height - BUTTON_SIZE)
		y1 = display_height - BUTTON_SIZE;

	al_draw_rectangle(x1, y1, x2, y2, WRITING_COLOR, 1);	//minimap

				//draw happiness
	al_draw_bitmap(image_list[SMILE_IMAGE], 0, 0, 0);
	al_draw_textf(font25, WRITING_COLOR, 40, 5, ALLEGRO_ALIGN_LEFT, "%i", session->show_happiness());
		
	al_draw_bitmap(image_list[HONOUR_IMAGE], 100, 10, 0);
	al_draw_textf(font25, WRITING_COLOR, 140, 5, ALLEGRO_ALIGN_LEFT, "%i", session->honour);
				
				//draw time to invasion
	int time_to_invasion = session->frames_to_invasion();
	if((time_to_invasion > 0) && (time_to_invasion < 300*game_info::fps))
		al_draw_textf(font25, WRITING_COLOR, display_width - 5, 35, ALLEGRO_ALIGN_RIGHT, "%i:%.2i", time_to_invasion / (60*game_info::fps), (time_to_invasion / game_info::fps) % 60 );
	LOG("main panel finished");
	return 0;
}

void draw_tile_to_minimap(tile* t, float size_increase)
{
	int tile_x = t->show_tile_x();
	int tile_y = t->show_tile_y();
	int x1 = (tile_x - tile_y + game_info::map_height)*size_increase;
	int y1 = (tile_x + tile_y)*size_increase/2;
	int x2 = (tile_x - tile_y + MAP_HEIGHT + 1)*size_increase;
	int y2 = (tile_x + tile_y + 2)*size_increase/2;

	ALLEGRO_COLOR color;

	t->check_death_people_on_tile();

	if(!t->building_on_tile.expired())
	{
		switch(t->building_on_tile.lock()->show_owner())
		{
		case(BLUE_PLAYER):
			color = BLUE_COLOR;
			break;
		case(RED_PLAYER):
			color = RED_COLOR;
			break;
		}
	}
	else if(!t->people_on_tile.empty())
	{
		switch(t->people_on_tile[0].lock()->show_owner())
		{
		case(BLUE_PLAYER):
			color = BLUE_COLOR;
			break;
		case(RED_PLAYER):
			color = RED_COLOR;
			break;
		}
	}
	else if(t->is_water_tile())
	{
		color = LIGHT_BLUE_COLOR;
	}
	else if(t->object != NOTHING)
	{
		switch(t->object)
		{
			case(IRON_TILE):
				color = DARK_RED_COLOR;
				break;

			case(MARBLE_TILE):
				color = LIGHT_GREY_COLOR;
				break;
			
			case(COAL_TILE):
				color = BLACK_COLOR;
				break;

			case(GOLD_TILE):
				color = YELLOW_COLOR;
				break;
			
			case(TREE_TILE):
				color = DARK_GREEN_COLOR;
				break;
			default:
				throw std::exception();
		}
	}
	else if(t->is_fertile())
	{
		color = DARK_YELLOW_COLOR;
	}
	else 
		color = GREEN_COLOR;


	al_draw_filled_rectangle(x1, y1, x2, y2, color);
}

ALLEGRO_BITMAP* create_minimap()
{
	LOG("");
	
	ALLEGRO_BITMAP* minimap = al_create_bitmap(160, 80);
	al_set_target_bitmap(minimap);
	al_clear_to_color(BLACK_COLOR);
	
	float size_increase = (float)BUTTON_SIZE / (float)((game_info::map_width > game_info::map_height) ? game_info::map_width : game_info::map_height); 
	for(int i=0; i<game_info::map_height; i++)
	{
		for(int j = 0; j< game_info::map_width; j++)
		{
			draw_tile_to_minimap(session->tile_list[i][j].get(), size_increase);		
		}
	}
	
	al_set_target_backbuffer(game_info::display);			//not drawing to bitmap, but normally to display 
	
	LOG("minimap finished");

	return minimap;
}

int update_minimap(ALLEGRO_BITMAP* minimap)
{
	LOG("");
	
	al_set_target_bitmap(minimap);
	float size_increase = (float)BUTTON_SIZE / (float)((game_info::map_width > game_info::map_height) ? game_info::map_width : game_info::map_height); 
	for(unsigned int i=0; i<tile::minimap_updates.size(); i++)
		draw_tile_to_minimap(tile::minimap_updates[i], size_increase);
	
	tile::minimap_updates.clear();
	al_set_target_backbuffer(game_info::display);			//not drawing to bitmap, but normally to display 
	
	LOG("minimap update finished");

	return 0;
}

/*Deletes all dead people, building and missiles from their lists and also deletes all pointers to them. */
int delete_death(game_mouse* mouse) // deletes all death people
{
	LOG("");
	
	mouse->check_death();			
	for(int i=0; i<session->people_list.size(); i++)
	{
		session->people_list[i]->check_death();
	}
							//remove all dead people from session->people_list
	std::vector<boost::shared_ptr<people>>::iterator people_it = remove_if(session->people_list.begin(), session->people_list.end(), [](boost::shared_ptr<people> p) {return p->is_death();});
	session->people_list.erase(people_it, session->people_list.end());
							//remove dead buildings
	std::vector<boost::shared_ptr<building>>::iterator building_it = remove_if(session->building_list.begin(), session->building_list.end(), [](boost::shared_ptr<building> p) {return p->is_death();});
	session->building_list.erase(building_it, session->building_list.end());
							//remove dead missiles
	std::vector<boost::shared_ptr<missile>>::iterator missile_it = remove_if(session->missile_list.begin(), session->missile_list.end(), [](boost::shared_ptr<missile> p) {return p->is_death();});
	session->missile_list.erase(missile_it, session->missile_list.end());
	
	LOG("check death finished");	

	return 0;
}

/*Returns array with tile pointers, which are in circle defined by start and radius parametr. Size of array is in number_of_tiles variable. Array must be deleted in calling function. */ 
std::vector<tile*> tiles_in_circle(float radius, tile* start)
{
	//this returns tiles in circle sorted from the nearest ones to the farest one
	
	
	radius = radius*1.5;		//radius one means that in circle will be 9 tiles

	std::vector<tile*> tiles_in_circle;
	tiles_in_circle.push_back(start);

	int x;
	int y;

	int line_to_check = 0;
	bool done = false;

	while(!done)
	{
		done = true;
		line_to_check++;

		x = start->show_tile_x() + line_to_check;
		y = start->show_tile_y() + line_to_check;
		
		for(int side=0; side < 4; side++)
		{
			for(int i=0; i<2*line_to_check; i++)
			{
				if(((y >= 0) & (y < game_info::map_height)) & ((x >= 0) & (x < game_info::map_width))) 
				{
					if(pow((session->tile_list[y][x]->show_tile_x() - start->show_tile_x()), 2) + 
						pow((session->tile_list[y][x]->show_tile_y() - start->show_tile_y()), 2) <= (int)(radius*radius))
					{
						tiles_in_circle.push_back(session->tile_list[y][x].get());
						done = false;
					}
				}
				switch(side)
				{
				case(0):
					y--;
					break;
				case(1):
					x--;
					break;
				case(2):
					y++;
					break;
				case(3):
					x++;
					break;
				}
			}
		}
	}
	return tiles_in_circle;
}

std::vector<tile*> tiles_in_rectangle(int x1, int y1, int x2, int y2)
{
	//returns array of tiles, first one is left up corner[y1][x1], then left [y1-1][x1+1], [y1-2][x1+2], [y1-3][x+3],..., 
	// then right up corner and then the same but one line lower, so first from left to right and then next line   
	//Tiles should be deleted to avoid memory leaks.

	int start_x = (x1 + x2)/2 + (y1 - y2)/2;
	int rectangle_width = start_x - x1;
	int start_y = y1 - rectangle_width;
	int last_x = x2 - rectangle_width;
	int last_y = y2 + rectangle_width;
	int rectangle_height = x2 - start_x;
	
	if((rectangle_width < 0) & (rectangle_height < 0))
	{
		std::swap(x1, x2);
		std::swap(y1, y2);
		std::swap(start_x, last_x);
		std::swap(start_y, last_y);
		rectangle_width = -rectangle_width;
		rectangle_height = -rectangle_height;
	}

	if(rectangle_width < 0)
	{
		std::swap(start_x, x1);
		std::swap(start_y, y1);
		std::swap(last_x, x2);
		std::swap(last_y, y2);
		rectangle_width = -rectangle_width;
	}
	if(rectangle_height < 0)
	{
		std::swap(start_x, x2);
		std::swap(start_y, y2);
		std::swap(last_x, x1);
		std::swap(last_y, y1);
		rectangle_height = -rectangle_height;
	}

	if((rectangle_height*rectangle_width*2 > 1000000) || (rectangle_height < 0) || (rectangle_width < 0))
	{
		throw std::exception();
		return std::vector<tile*>();
	}
	std::vector<tile*> tiles_in_rectangle;

	for(int i = 0; i < rectangle_height; i++)
	{
		for(int j = 0; j < rectangle_width; j++)
		{
			int x = x1 + i + j;
			int y = y1 + i - j;

			if(((y >= 0) & (y < game_info::map_height)) & ((x >= 0) & (x < game_info::map_width)))
				tiles_in_rectangle.push_back(session->tile_list[y][x].get());
			
		}
		for(int j = 0; j<rectangle_width; j++)
		{
			int x = x1 + i + j + 1;
			int y = y1 + i - j;
			if(((y >= 0) & (y < game_info::map_height)) & ((x >= 0) & (x < game_info::map_width)))
				tiles_in_rectangle.push_back(session->tile_list[y][x].get());
		}
	}
	
	return tiles_in_rectangle;
}

void reset_map(std::vector<std::vector<boost::shared_ptr<tile>>>& map)
{
	map.clear();
	for(int i=0; i<game_info::map_height; ++i)
	{
		map.push_back(std::vector<boost::shared_ptr<tile>>());
		for(int j=0; j<game_info::map_width; ++j)
			map[i].push_back(boost::shared_ptr<tile>(new tile(GRASS, j, i, 0)));
	}
}

/* Main generating function. In generated map are all tiles accessible from borders. Needs lots of time (normally about 0.5s but theoretically infinite amount).*/
std::vector<std::vector<boost::shared_ptr<tile>>> generate_map(std::vector<int> resources, int mountains)
{
	std::vector<std::vector<boost::shared_ptr<tile>>> map;

	bool done = false;
	while(!done)
	{
		LOG("generating map");
		
		reset_map(map);

		generate_surface(map, mountains);

		generate_water(map, resources[static_cast<int>(natural_resources::WATER)]);
		generate_resource(map, TREE_TILE, 3*resources[static_cast<int>(natural_resources::WOOD)]);	//we need more forests
		generate_resource(map, IRON_TILE, resources[static_cast<int>(natural_resources::IRON)]);
		generate_resource(map, COAL_TILE, resources[static_cast<int>(natural_resources::COAL)]);
		generate_resource(map, MARBLE_TILE, resources[static_cast<int>(natural_resources::MARBLE)]);
		generate_resource(map, GOLD_TILE, resources[static_cast<int>(natural_resources::GOLD)]);
		generate_fertile(map, resources[static_cast<int>(natural_resources::FERTILE_SOIL)]);

		for(int i = 0; i<game_info::map_height; i++)		
		{
			for(int j = 0; j<game_info::map_width; j++)
				map[i][j]->set_right_drawing(map);
		}

		for(int i = 0; i<game_info::map_height; i++)		
		{
			for(int j = 0; j<game_info::map_width; j++)
				map[i][j]->set_accessible_neighbours(map);
			
		}
		done = check_map(map);
	}
	return map;
}

/*Generated tiles height of tiles and ramps.*/
void generate_surface(std::vector<std::vector<boost::shared_ptr<tile>>> & map, int mountains)
{

	int clenitost = 50*mountains;
	int max_height = 5*mountains;
	int hornatost = 2;

	for(int i = 0; i<clenitost; i++)
	{
		int tile_x1 = 1 + (rand() % (game_info::map_width - 2));
		int tile_y1 = 1 + (rand() % (game_info::map_height - 2));
		int tile_x2 = 1 + (rand() % (game_info::map_width - 2));
		int tile_y2 = 1 + (rand() % (game_info::map_height - 2));

		if(tile_y2 - tile_y1 < 3)		//negative or very small size
			continue;
		if(tile_x2 - tile_x1 < 3)
			continue;
	
		if(tile_x2 - tile_x1 > game_info::map_width/2)
		{
			tile_x1 += game_info::map_width/4;
			tile_x2 -= game_info::map_width/4;
		}
		
		if(tile_y2 - tile_y1 > game_info::map_height/2)
		{
			tile_y1 += game_info::map_height/4;
			tile_y2 -= game_info::map_height/4;
		}
		
		int height = max_height;
		for(int j = 0; j < hornatost; j++)
		{
			int random_number = rand() % max_height;
			if(random_number < height)
				height = random_number;
		}
		
		int max_height_under = 0;
		for(int y = tile_y1; y <= tile_y2; y++)
		{
			for(int x = tile_x1; x <= tile_x2; x++)
			{
				if(max_height_under < map[y][x]->show_surface_height())
				{
					max_height_under = map[y][x]->show_surface_height();
				}
			}
		}
		if(height > max_height_under)
			height = max_height_under + 1;
				
		for(int y = tile_y1; y <= tile_y2; y++)
		{
			for(int x = tile_x1; x <= tile_x2; x++)
			{
				if((((x == tile_x1) || (x == tile_x2)) || ((y == tile_y1) || (y == tile_y2))) && (height > 0))
				{
					map[y][x]->set_tile_type(RAMP);
					map[y][x]->set_surface_height(height);
				}
				else
				{
					map[y][x]->set_tile_type(GRASS);
					map[y][x]->set_surface_height(height);
				} 
			}
		}
	}
}

/* Adds one 5*5 block of tiles with stone, one with iron, one with trees. There is a chnace that blocks will be at same location thus one will overwrite another. Dont add anything if map is very small.*/ 
void generate_resource(std::vector<std::vector<boost::shared_ptr<tile>>>& map, object_on_tile ob, int amount)
{
	if((game_info::map_width < 10) || (game_info::map_height < 10))
	{
		LOG("cannot generate resources - map is too small");
		return;
	}

	for(int k=0; k<amount; ++k)
	{
		int resource_x = rand() % (game_info::map_width - 5);
		int resource_y = rand() % (game_info::map_height - 5);

		if((MAP_WIDTH > 10) & (MAP_HEIGHT > 10))		//add some trees
		{
			for(int i=resource_x; i<resource_x + 4; i++)
			{
				for(int j = resource_y; j<resource_y + 4; j++)
				{
					if(map[i][j]->show_type() == GRASS)
						map[i][j]->add_object(ob);
				}
			}
		}
	}
}

void generate_fertile(std::vector<std::vector<boost::shared_ptr<tile>>>& map, int amount)
{	
	for(int i=0; i<4*amount; ++i)
	{
		int x = (rand() % (game_info::map_width - 6));
		int y = (rand() % (game_info::map_width - 6));
		for(int i=0; i<25; ++i)
		{
			int x2 = rand() % 7 + x;
			int y2 = rand() % 7 + y;
			if((x2 >= 0) && (x2 < game_info::map_width) && (y2 >= 0) && (y2 < game_info::map_height))
			{
				if(map[y2][x2]->show_type() == GRASS)
					map[y2][x2]->add_fertile();
			}
		}
	}
}

void generate_water(std::vector<std::vector<boost::shared_ptr<tile>>>& map, int amount)
{
	for(int i=0; i<3*amount; ++i)
	{
		int x = rand() % game_info::map_width;
		int y = rand() % game_info::map_height;
		int new_x = x, new_y = y;
	
		int probability = 120;

		while(rand() % 100 <= probability)
		{
			map[y][x]->add_water();
			if(rand() % 2 == 1)
				new_x = x + (rand() % 3) - 1;
			
			else
				new_y = y + (rand() % 3) - 1;
			
			if((new_x >= 0) && (new_x < game_info::map_width))
				x = new_x;

			if((new_y >= 0) && (new_y < game_info::map_height))
				y = new_y;

			probability--;
		}
	}
}

//	Check if every tile is accessible from borders. If not, then makes them accessible. If it is too hard to make them accessible return false.  
bool check_map(std::vector<std::vector<boost::shared_ptr<tile>>>& map)
{	
	for(int y=0; y<game_info::map_height; y++)
	{
		for(int x=0; x<game_info::map_width; x++)
		{
			if(((y >= 0) && (y < game_info::map_height)) && ((x >= 0) && (x < game_info::map_width)))
				map[y][x]->set_right_drawing(map);
		}
	}
	
	for(int y=0; y<game_info::map_height; y++)
	{
		for(int x=0; x<game_info::map_width; x++)
		{
			if(((y >= 0) && (y < game_info::map_height)) && ((x >= 0) && (x < game_info::map_width)))
				map[y][x]->set_accessible_neighbours(map);
		}
	}

	std::vector<std::vector<bool>> explored(game_info::map_height, std::vector<bool>(game_info::map_width, false));
	
	explored = find_accessible(explored, map[0][0].get());

	for(int y=0; y<game_info::map_height; y++)
	{
		for(int x=0; x<game_info::map_width; x++)
		{
			if((!explored[y][x]) && (!map[y][x]->is_water_tile()))
				//if tile is not explored I will change it's height such that it will be accessible from previous tile
			{			
				//int new_height;
				tile* previous_tile;
				direction direction_from_previous;

				if(y != 0)
				{
					previous_tile = map[y-1][x].get();
					direction_from_previous = SOUTHWEST;
				}
				else			//x > 0 because algorithm started with [0][0], which must be explored 
				{
					previous_tile = map[y][x-1].get();
					direction_from_previous = SOUTHEAST;
				}

				tile* t = map[y][x].get();
				
				possible_borders borders = tile_borders(previous_tile, direction_from_previous);

				if(borders == NORMAL_BORDER)
				{
					if(t->show_surface_height() > previous_tile->show_surface_height())
						t->set_surface_height(previous_tile->show_surface_height() + 1);
					else
						t->set_surface_height(previous_tile->show_surface_height());
				}
				else if(borders == DOWN_BORDER)
					t->set_surface_height(previous_tile->show_surface_height() - 1);

				else if(borders == RAMP_BORDER)
					t->set_surface_height(previous_tile->show_surface_height());
				
				t->set_tile_type(RAMP);

				for(int i=y-1; i<=y+1; i++)
				{
					for(int j=x-1; j<=x+1; j++)
					{
						if(((i >= 0) && (i < game_info::map_height)) && ((j >= 0) && (j < game_info::map_height)))
							map[i][j]->set_right_drawing(map);
					}
				}

				for(int i=y-1; i<=y+1; i++)
				{
					for(int j=x-1; j<=x+1; j++)
					{
						if(((i >= 0) && (i < game_info::map_height)) && ((j >= 0) && (j < game_info::map_height)))
							map[i][j]->set_accessible_neighbours(map);
					}
				}
				
				if(explored[previous_tile->show_tile_y()][previous_tile->show_tile_x()])
				{
					if(!previous_tile->is_water_tile())
						explored = find_accessible(explored, previous_tile);		//try if it is accessible from previous tile
				}
			}
		}
	}

	for(int i=0; i<game_info::map_height; ++i)
	{
		for(int j=0; j<game_info::map_width; ++j)
		{
			if((!explored[i][j]) && (!map[i][j]->is_water_tile()))	
				return false;
		}
	}
	return true;
}

/* Used only by check map function. Starts or continues breadth first search from start and remembers explored tiles. If cannot explore anything else stops and returns array with explored tiles.*/
std::vector<std::vector<bool>>& find_accessible(std::vector<std::vector<bool>> & explored, tile* start)
{
	std::vector<tile*> queue;

	int start_index = 0;

	queue.push_back(start);

	explored[start->show_tile_y()][start->show_tile_x()] = true;
	
	while(start_index < queue.size())		//breadth first search
	{
		for(int i=0; i<queue[start_index]->accessible_neighbours.size(); i++)
		{
			tile* next_tile = queue[start_index]->accessible_neighbours[i];
			
			if(!next_tile)
				LOG("WARNING - wrong accessible_neighbours");	

			if(!explored[next_tile->show_tile_y()][next_tile->show_tile_x()])
			{
				queue.push_back(next_tile);
				explored[next_tile->show_tile_y()][next_tile->show_tile_x()] = true;
			}
		}
		start_index++;
	}
	
	return explored;
}

	//I think that this function should be deleted and rotation of map should be only graphical effect which will not change data structures. 
	//Because I had to change every tile pointer, there can be bugs or will be after adding new functionality. But it will be very hard to rewrite it.
void rotate_map(bool clockwise, ALLEGRO_BITMAP** minimap, int *screen_position_x, int* screen_position_y)
{
	//rotates the map 90 degrees clockwise, rotates path of people, and let all missiles 
	//hit theirs target, because it would be very hard to rotate them
	
	LOG("rotating map");

	int middle_screen_x = compute_tile_x(*screen_position_x + display_width/2, *screen_position_y + display_height/2);
	int middle_screen_y = compute_tile_y(*screen_position_x + display_width/2, *screen_position_y + display_height/2);

	if(clockwise)
	{
		*screen_position_x = compute_game_x(MAP_HEIGHT - 1 - middle_screen_y, middle_screen_x) - display_width/2;
		*screen_position_y = compute_game_y(MAP_HEIGHT - 1 - middle_screen_y, middle_screen_x) - display_height/2;
	}
	else if(!clockwise)
	{
		*screen_position_x = compute_game_x(middle_screen_y, MAP_WIDTH - 1 - middle_screen_x) - display_width/2;
		*screen_position_y = compute_game_y(middle_screen_y, MAP_WIDTH - 1 - middle_screen_x) - display_height/2;
	}
		
	
	for(int i=0; i<session->missile_list.size(); i++)
	{
		while(!session->missile_list[i]->is_death())
			session->missile_list[i]->draw_and_update(0,0);	
	}

	std::vector<std::vector<boost::shared_ptr<tile>>> new_tile_list(game_info::map_width, std::vector<boost::shared_ptr<tile>>(game_info::map_height, boost::shared_ptr<tile>()));

	for(int old_y=0; old_y<session->tile_list.size(); ++old_y)
	{
		for(int old_x=0; old_x<session->tile_list[old_y].size(); ++old_x)
		{
			int new_x, new_y;
			if(clockwise)
			{
				new_y = old_x;
				new_x = game_info::map_height - 1 - old_y;
			}
			else
			{
				new_y = game_info::map_width - 1 - old_x;
				new_x = old_y;
			}
			new_tile_list[new_y][new_x] = session->tile_list[old_y][old_x];
			new_tile_list[new_y][new_x]->rotate(new_x, new_y, clockwise);
			
			boost::shared_ptr<tile> new_tile = new_tile_list[new_y][new_x];
			
			
			if(!new_tile->building_on_tile.expired())
			{
				boost::shared_ptr<building> building_to_rotate = new_tile->building_on_tile.lock();
				if((building_to_rotate->show_tile_x() == old_x) && (building_to_rotate->show_tile_y() == old_y))
					building_to_rotate->rotate(new_x, new_y, clockwise);
			}
		}
	}

	session->tile_list = new_tile_list;

	std::swap(game_info::map_height, game_info::map_width);

	for(int y=0; y<session->tile_list.size(); ++y)
	{
		for(int x=0; x<session->tile_list[y].size(); ++x)
		{	
			boost::shared_ptr<tile> t = session->tile_list[y][x];
	
			t->check_death_people_on_tile();
			for(int i=0; i<t->people_on_tile.size(); ++i)
				t->people_on_tile[i].lock()->rotate(x, y, clockwise);
			
			if(!t->building_on_tile.expired())
			{
				boost::shared_ptr<building> building_to_rotate = t->building_on_tile.lock();
				if((building_to_rotate->show_tile_x() == x) && (building_to_rotate->show_tile_y() == y))
					building_to_rotate->set_drawing_tile();
			}

			t->set_right_drawing(session->tile_list);
			
			if((t->unreal_path_on_tile()) || (t->real_path_on_tile()))
				t->set_path_drawing();
		}
	}

	for(int i=0; i<new_tile_list.size(); ++i)
	{
		for(int j=0; j<new_tile_list[i].size(); ++j)
			new_tile_list[i][j]->set_accessible_neighbours(session->tile_list);
	}
	
	al_destroy_bitmap(*minimap);
	
	*minimap = create_minimap();
	
	LOG("rotating finished");
}

//Returns direction from old tile to new_tile.
direction compute_direction(tile* old_tile, tile* new_tile)
{
	direction movement_direction = NO_DIRECTION;

	if(old_tile->show_tile_x() == new_tile->show_tile_x())
	{
		if(old_tile->show_tile_y() > new_tile->show_tile_y())
			movement_direction = NORTHEAST;
		else
			movement_direction = SOUTHWEST;
	}
	else if(old_tile->show_tile_x() > new_tile->show_tile_x())
	{
		if(old_tile->show_tile_y() == new_tile->show_tile_y())
			movement_direction = NORTHWEST;
		else if(old_tile->show_tile_y() > new_tile->show_tile_y())
			movement_direction = NORTH;
		else
			movement_direction = WEST;
	}
	else
	{
		if(old_tile->show_tile_y() == new_tile->show_tile_y())
			movement_direction = SOUTHEAST;
		else if(old_tile->show_tile_y() > new_tile->show_tile_y())
			movement_direction = EAST;
		else
			movement_direction = SOUTH;
	}
	return movement_direction;
}


