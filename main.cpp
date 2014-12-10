#include "core.h"

std::ofstream log_file("log", std::ios::out); 
 
int display_width;
int display_height;

ALLEGRO_FONT* font15;
ALLEGRO_FONT* font20;
ALLEGRO_FONT* font25;
ALLEGRO_FONT* font30;

ALLEGRO_BITMAP** image_list;
//ALLEGRO_DISPLAY* display = NULL;

game_session* session = NULL;

int main()
{
	if(!init_allegro())
	{
		log_file.close();
		return 0;
	}
	if(!load_fonts())
	{
		std::cout << "Failed to load font 'data/arial.ttf'." << std::endl; 	
		log_file.close();
		return 0;
	}
	load_pictures();
	load_settings();

	create_display();
	
	if(game_info::music)
		music::get_instance().play_background_music();
	

	event_handler::get_instance().change_state(game_state::MAIN_MENU);	//game is beginning in main menu
	event_handler::get_instance().run();					//runs event loop

	log_file.close();
	
	delete_pictures();
	delete_fonts();

	return 0;
}

void create_display()
{
	ALLEGRO_DISPLAY_MODE *display_mode = new ALLEGRO_DISPLAY_MODE;

	if(game_info::fullscreen)
	{
		al_set_new_display_flags(ALLEGRO_FULLSCREEN_WINDOW);
		al_get_display_mode(0, display_mode);
		display_width = display_mode->width;
		display_height = display_mode->height;
	}
	else
	{
		al_set_new_display_flags(ALLEGRO_WINDOWED);
		al_set_new_display_flags(ALLEGRO_RESIZABLE);
	}

	al_set_new_window_position(10, 10);

	game_info::display = al_create_display(display_width, display_height);
	delete display_mode;
}

/* Handles player actions in main menu and launches main game loop*/
/*
int main_menu()
{
	bool menu = true;

	ALLEGRO_DISPLAY_MODE *display_mode = new ALLEGRO_DISPLAY_MODE;

	if(game_info::fullscreen)
	{
		al_set_new_display_flags(ALLEGRO_FULLSCREEN_WINDOW);
		al_get_display_mode(0, display_mode);
		display_width = display_mode->width;
		display_height = display_mode->height;
	}
	else
	{
		al_set_new_display_flags(ALLEGRO_WINDOWED);
		al_set_new_display_flags(ALLEGRO_RESIZABLE);
	}

	al_set_new_window_position(10, 10);

	game_info::display = al_create_display(display_width, display_height);
	
	ALLEGRO_EVENT_QUEUE* queue = al_create_event_queue();
	ALLEGRO_EVENT* ev = new ALLEGRO_EVENT;
	ALLEGRO_MOUSE_STATE* mouse = new ALLEGRO_MOUSE_STATE; 

	al_register_event_source(queue, al_get_display_event_source(game_info::display));			//wihout this ALLEGRO_EVENT_DISPLAY_CLOSE doesnt run
	al_register_event_source(queue, al_get_mouse_event_source());
	al_register_event_source(queue, al_get_keyboard_event_source());
	
	main_menu_option chosen_option = static_cast<main_menu_option>(0);
	
	bool done = false;
	bool proved_volbu = false;

	if(!menu)		
	{
		chosen_option = static_cast<main_menu_option>(0);
		proved_volbu = true;
	}

	while(!done)
	{
		draw_main_menu(chosen_option);
		if(game_info::close_display)
		{
			LOG("display was closed - terminating");
			done = true;
		}

		al_wait_for_event(queue, ev);							//Game waits for player's action (e.g. key pressed). Type of action is saved in ev.  
		if(ev->type == ALLEGRO_EVENT_DISPLAY_CLOSE)
		{
			done = true;
		}
		if((ev->type == ALLEGRO_EVENT_DISPLAY_RESIZE) && (!game_info::fullscreen))
		{
			resize_display(ev);
		}
		else if(ev->type == ALLEGRO_EVENT_MOUSE_AXES)			//pohyb mysi
		{
			al_get_mouse_state(mouse);
			if((mouse->x > display_width/2 - 60) & (mouse->x < display_width/2 + 60))
			{
				if(((mouse->y - 100) / 80) < static_cast<int>(NO_OPTION))			
				{
					chosen_option = (main_menu_option)((mouse->y - 100) / 80);
				}
			}		
		}
		else if(ev->type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN)
		{
			al_get_mouse_state(mouse);
			if(((mouse->y - 100) / 80) < static_cast<int>(NO_OPTION))			
			{
				proved_volbu = true;
			}
		}
		else if(ev->type == ALLEGRO_EVENT_KEY_DOWN)									//some key was pressed
		{
			if(ev->keyboard.keycode == ALLEGRO_KEY_UP)							//zmacknuta klavesa "sipka nahoru"
			{
				chosen_option = (main_menu_option)((int)chosen_option - 1);			//the same as chosen_option-- 
				if(chosen_option < 0)
					chosen_option = QUIT;
			}
			else if(ev->keyboard.keycode == ALLEGRO_KEY_DOWN)
			{
				chosen_option = (main_menu_option)((int)chosen_option + 1);			//the same as chosen_option++ 
				if(chosen_option > 4)
					chosen_option = static_cast<main_menu_option>(0);
			}
			else if(ev->keyboard.keycode == ALLEGRO_KEY_ENTER)
			{
				proved_volbu = true;
			}
			else if(ev->keyboard.keycode == ALLEGRO_KEY_ESCAPE)
			{
				chosen_option = QUIT;
				proved_volbu = true;
			}
		}
		if(proved_volbu)
		{
			switch(chosen_option)
			{
			case(RANDOM_GAME):
			{
				game_loop();
				al_flush_event_queue(queue);
			}
			break;
			case(CAMPAIGN):
			{
			}
			break;
			case(LOAD_GAME):
			{
			}
			break;
			case(SETTINGS):
			{
				settings(queue, ev);
				al_flush_event_queue(queue);
			}
			break;
			case(QUIT):
			{
				done = true;
			}
			}

			proved_volbu = false;
		}
	}

	al_destroy_display(game_info::display);
	al_destroy_event_queue(queue);

	delete display_mode;
	delete ev;
	delete mouse;

	return 0;
}
*/
/* Draws menu. Parametr chosen option sets which option should be highlighted*/
/*
int draw_main_menu(main_menu_option chosen_option)
{
	al_clear_to_color(al_map_rgb(0,0,0));

	std::vector<std::string> options_names;
	options_names.push_back("Random game");
	options_names.push_back("Campaign");
	options_names.push_back("Load game");
	options_names.push_back("Settings");
	options_names.push_back("Quit");

	for(int i=0; i<options_names.size(); i++)
	{
		if(chosen_option != i)
			al_draw_textf(font25, al_map_rgb(150, 150, 150), display_width/2, 100 + 80*i, ALLEGRO_ALIGN_CENTRE, options_names[i].c_str());  
		else
			al_draw_textf(font30, al_map_rgb(200, 200, 200), display_width/2, 100 + 80*i, ALLEGRO_ALIGN_CENTRE, options_names[i].c_str()); 
	}
	
	al_flip_display();

	return 0;
}
*/

/*Handles player actions in settings. Saves changes when player leaves settings.*/
/*
int settings(ALLEGRO_EVENT_QUEUE* queue, ALLEGRO_EVENT* ev)
{
	al_flush_event_queue(queue);

	bool fullscreen_option = game_info::fullscreen;
	int display_width_option = display_width;
	int display_height_option = display_height;
	int fps_option = game_info::fps;

	int number_of_options = 4;
	bool done = false;
	int option_focus = 0;
	std::string text = "";

	while(!done)
	{
		draw_settings(option_focus, text, fullscreen_option, display_width_option, display_height_option, fps_option);
		al_wait_for_event(queue, ev);
		
		if(ev->type == ALLEGRO_EVENT_DISPLAY_CLOSE)
		{
			done = true;
			game_info::close_display = true;
		}
		else if(ev->type == ALLEGRO_EVENT_DISPLAY_RESIZE)
			resize_display(ev);		

		else if(ev->type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN)
		{	
			//cout << "x = " << ev->mouse.x << " y = " << ev->mouse.y << endl;

			if(text.length() > 0)
			{
				int value = string_to_int(text);
				
				if((value > 0) && (value < 10000))
				{
					if(option_focus == 1)
						display_width_option = value;

					else if(option_focus == 2)
						display_height_option = value;

					else if(option_focus == 3)
						fps_option = value;
				}
			}

			option_focus = 0;
			
			if((ev->mouse.x > display_width - 100) && (ev->mouse.y > display_height - 50))
				done = true;

			else if((ev->mouse.x > 300) && (ev->mouse.x < 500))
			{
				if((ev->mouse.y > 200) && (ev->mouse.y < 250))
				{
					option_focus = 0;
					fullscreen_option = !fullscreen_option;
				}
				else if((ev->mouse.y > 250) && (ev->mouse.y < 300))
				{
					option_focus = 1;
					text = "";
				}
				else if((ev->mouse.y > 300) && (ev->mouse.y < 350))
				{
					option_focus = 2;
					text = "";
				}
				else if((ev->mouse.y > 350) && (ev->mouse.y < 400))
				{
					option_focus = 3;
					text = "";
				}
			}
					
		}
		else if(ev->type == ALLEGRO_EVENT_KEY_DOWN)
		{
			if(ev->keyboard.keycode == ALLEGRO_KEY_ESCAPE)
				done = true;
			
			else if((ev->keyboard.keycode > 26) && (ev->keyboard.keycode <= 46))		//key with number was pressed
			{
				if(text.size() < 28)		//text[29] will be null
				{	
					int keycode = ev->keyboard.keycode;
					if(keycode > 36)
						keycode -= 10;

					text.push_back((char)(keycode + 21));
				}
			}
			else if(ev->keyboard.keycode == ALLEGRO_KEY_ENTER)
			{
				int value = string_to_int(text);
				if((value > 0) && (value < 10000))
				{
					if(option_focus == 1)
						display_width_option = value;

					else if(option_focus == 2)
						display_height_option = value;
					
					else if(option_focus == 3)
						fps_option = value;
				}
				option_focus = 0;
			}
			else if(ev->keyboard.keycode == ALLEGRO_KEY_BACKSPACE)
				text.pop_back();
			
			else if(ev->keyboard.keycode == ALLEGRO_KEY_UP)
			{
				text = "";
				option_focus--;
				if(option_focus < 0)
					option_focus = number_of_options - 1;
			}
			else if(ev->keyboard.keycode == ALLEGRO_KEY_DOWN)
			{
				text = "";
				option_focus++;
				if(option_focus > number_of_options - 1)
					option_focus = 0;
			}
		}
	}

	save_settings(fullscreen_option, display_width_option, display_height_option, fps_option);

	return 0;
}
*/
/*Draws settings. Parametr option focus sets which variable is changed by player, text is input from player, other parametres are actual values  of variables.*/
/*
int draw_settings(int option_focus, std::string text, int fullscreen_option, int display_width_option, int display_height_option, int fps_option)
{	
	
	al_clear_to_color(al_map_rgb(0,0,0));
	
	al_draw_text(font30, al_map_rgb(150, 150, 150), display_width/2, 100, ALLEGRO_ALIGN_CENTRE, "Settings");

	int number_of_options = 4;
	char** options_names = new char* [number_of_options];
	for(int i=0; i<number_of_options; i++)
		options_names[i] = new char [20];
	
	strcpy(options_names[0], "Fullscreen:");
	strcpy(options_names[1], "Display width:");
	strcpy(options_names[2], "Display height:");
	strcpy(options_names[3], "FPS:");

	int start_x = 150;
	int start_y = 200;	
	int line_y = 50;	//height of one line
	int option_x = 300;	//where will be values

	for(int i=0; i<number_of_options; i++)
		al_draw_text(font20, al_map_rgb(150, 150, 150), start_x, start_y + line_y*i, ALLEGRO_ALIGN_LEFT, options_names[i]);
	
	if(fullscreen_option)
		al_draw_text(font20, al_map_rgb(150, 150, 150), option_x, start_y, ALLEGRO_ALIGN_LEFT, "Enabled");

	else
		al_draw_text(font20, al_map_rgb(150, 150, 150), option_x, start_y, ALLEGRO_ALIGN_LEFT, "Disabled");

	for(int i=1; i<number_of_options; i++)
	{
		int option_value;
		switch(i)
		{
			case(1):
				option_value = display_width_option;
				break;
			case(2):
				option_value = display_height_option;
				break;
			case(3):
				option_value = fps_option;
				break;
			default:
				LOG("not defined option");
		}

		if(option_focus != i)
			al_draw_textf(font20, al_map_rgb(150, 150, 150), option_x, start_y + i*line_y, ALLEGRO_ALIGN_LEFT, "%i", option_value);
		
		else
		{
			al_draw_rectangle(option_x - 10, start_y + i*line_y - 10, option_x + 200, start_y + (i + 1)*line_y - 10, al_map_rgb(100, 100, 100), 3);
			al_draw_text(font20, al_map_rgb(200, 200, 200), option_x, start_y + i*line_y, ALLEGRO_ALIGN_LEFT, text.c_str());
		}
	}

	al_draw_text(font20, al_map_rgb(150, 150, 150), display_width/2, display_height - 150, ALLEGRO_ALIGN_CENTRE, "You must restart game to take any changes in effect.");
	
	al_draw_rectangle(display_width - 100, display_height - 50, display_width - 5, display_height - 5, al_map_rgb(100, 100, 100), 3);
	al_draw_text(font20, al_map_rgb(150, 150, 150), display_width - 90, display_height - 40, ALLEGRO_ALIGN_LEFT, "Done");

	al_flip_display();

	for(int i=0; i<number_of_options; i++)
		delete[] options_names[i]; 

	delete[] options_names;

	return 0;
}
*/
/*Saves settings to file. Doesnt change ingame variables.*/

/*void save_settings(bool fullscreen_option, int display_width_option, int display_height_option, int fps_option)
{
	std::ofstream file;
	file.open("data/config.txt", std::ios::out);

	file << "fullscreen = ";
	if(fullscreen_option)
		file << "true" << std::endl;

	else
		file << "false" << std::endl;


	file << "display_width = " << display_width_option << std::endl << "display_height = " << display_height_option << std::endl << "fps = " << fps_option;

	file.close();
	return 0;
}
*/
//Initialize allegro. If not succesfull, returns -1.
bool init_allegro()
{
	bool ok = true;
	if(!al_init())
		ok = false;
	if(!al_init_primitives_addon())
		ok = false;
	if(!al_init_image_addon())
		ok = false;
	if(!al_install_mouse())
		ok = false;
	if(!al_install_keyboard())
		ok = false;
	if(!al_install_audio())
		ok = false;
	if(!al_init_acodec_addon())
		ok = false;

	al_init_font_addon();										//font_addon must be initialized before ttf_addon

	if(!al_init_ttf_addon())
		ok = false;

	if(!ok)
	{	
		LOG("failed to initialize allegro");
		std::cout << "failed to initialize allegro" << std::endl;
		//return -1;
	}
	LOG("allegro succesfully initialized");
	return ok;
}

/*Loads all images. If image is not found, then instead it loads special image 'no_image.png' so game will not crash and function returns false.*/
bool load_pictures()
{
	image_list = new ALLEGRO_BITMAP* [LAST_IMAGE];
	
	//building images
	
	image_list[HOUSE_IMAGE] = al_load_bitmap("data/images/house.png");
	image_list[MARKET_IMAGE] = al_load_bitmap("data/images/market.png");
	image_list[WAREHOUSE_IMAGE] = al_load_bitmap("data/images/warehouse.png");
	image_list[GREAT_HALL_IMAGE] = al_load_bitmap("data/images/great_hall.png");
	image_list[CHURCH_IMAGE] = al_load_bitmap("data/images/church.png");
	image_list[STORE_IMAGE] = al_load_bitmap("data/images/store.png");
	image_list[BARRACKS_IMAGE] = al_load_bitmap("data/images/barracks.png");
	image_list[SCOUT_IMAGE] = al_load_bitmap("data/images/scout.png");
	image_list[QUARRY_IMAGE] = al_load_bitmap("data/images/quarry.png");
	image_list[WOODCUTTER_IMAGE] = al_load_bitmap("data/images/woodcutter.png");
	image_list[HUNTER_IMAGE] = al_load_bitmap("data/images/hunter.png");
	image_list[FISHERMAN_IMAGE] = al_load_bitmap("data/images/fisherman.png");
	image_list[APPLE_FARM_IMAGE] = al_load_bitmap("data/images/apple_farm.png");
	image_list[DAIRY_FARM_IMAGE] = al_load_bitmap("data/images/dairy_farm.png");
	image_list[WHEAT_FARM_IMAGE] = al_load_bitmap("data/images/wheat_farm.png");
	image_list[WINDMILL_IMAGE] = al_load_bitmap("data/images/windmill.png");
	image_list[CLAY_PIT_IMAGE] = al_load_bitmap("data/images/clay_pit.png");
	image_list[POTTERY_WORKSHOP_IMAGE] = al_load_bitmap("data/images/pottery_workshop.png");
	image_list[BRICKMAKER_IMAGE] = al_load_bitmap("data/images/brickmaker.png");
	image_list[MARBLE_QUARRY_IMAGE] = al_load_bitmap("data/images/marble_quarry.png");
	image_list[GOLD_MINE_IMAGE] = al_load_bitmap("data/images/gold_mine.png");
	image_list[COAL_MINE_IMAGE] = al_load_bitmap("data/images/coal_mine.png");
	image_list[IRON_MINE_IMAGE] = al_load_bitmap("data/images/iron_mine.png");
	image_list[SMITH_IMAGE] = al_load_bitmap("data/images/smith.png");
	image_list[ARMOURER_IMAGE] = al_load_bitmap("data/images/armourer.png");
	image_list[FLETCHER_IMAGE] = al_load_bitmap("data/images/fletcher.png");
	image_list[PALISADE_IMAGE] = al_load_bitmap("data/images/palisade.png");
	image_list[WALL_IMAGE] = al_load_bitmap("data/images/wall.png");
	image_list[TOWER_IMAGE] = al_load_bitmap("data/images/tower.png");
	image_list[STAIRS_IMAGE] = al_load_bitmap("data/images/stairs.png");
	image_list[GATE_IMAGE] = al_load_bitmap("data/images/gate.png");
	
	//people_images
	image_list[BOWMAN_IMAGE] = al_load_bitmap("data/images/bowman.png");
	image_list[AXEMAN_IMAGE] = al_load_bitmap("data/images/axeman.png");
	image_list[SPEARMAN_IMAGE] = al_load_bitmap("data/images/spearman.png");
	image_list[SWORDSMAN_IMAGE] = al_load_bitmap("data/images/swordsman.png");
	image_list[CATAPULT_IMAGE] = al_load_bitmap("data/images/catapult.png");
	image_list[CARRIER_IMAGE] = al_load_bitmap("data/images/carrier.png");
	
//	image_list[NATURE_IMAGE] = al_load_bitmap("data/images/nature.png");
//	image_list[AXE_IMAGE] = al_load_bitmap("data/images/sekera.png");
	image_list[ARROW_IMAGE] = al_load_bitmap("data/images/arrow.png");
	image_list[ARCHERS_ARROW_IMAGE] = al_load_bitmap("data/images/archers_arrow.png");
	image_list[CATAPULT_SHOT_IMAGE] = al_load_bitmap("data/images/catapult_shot.png");
	image_list[TEXTURE_GREY_IMAGE] = al_load_bitmap("data/images/texture_grey.png");
	image_list[GRASS_IMAGE] = al_load_bitmap("data/images/grass.png");
	image_list[PATHS_IMAGE] = al_load_bitmap("data/images/paths.png");
	image_list[OPEN_GATE_IMAGE] = al_load_bitmap("data/images/open_gate.png");
	image_list[ROTATE_LEFT_IMAGE] = al_load_bitmap("data/images/rotate_left.png");
	image_list[ROTATE_RIGHT_IMAGE] = al_load_bitmap("data/images/rotate_right.png");
	image_list[REMOVE_BUILDING_IMAGE] = al_load_bitmap("data/images/remove_building.png");
	image_list[STOP_WORKING_IMAGE] = al_load_bitmap("data/images/stop_working.png");
	image_list[NAVIGATION_BUTTONS_IMAGE] = al_load_bitmap("data/images/navigation_buttons.png");
	image_list[UPGRADE_IMAGE] = al_load_bitmap("data/images/upgrade.png");
	image_list[LOCK_IMAGE] = al_load_bitmap("data/images/lock.png");
	image_list[FEAST_IMAGE] = al_load_bitmap("data/images/feast.png");
	image_list[DETAILS_IMAGE] = al_load_bitmap("data/images/details.png");
	image_list[SMILE_IMAGE] = al_load_bitmap("data/images/smile.png");
	image_list[HONOUR_IMAGE] = al_load_bitmap("data/images/honour.png");
	image_list[STAR_IMAGE] = al_load_bitmap("data/images/star.png");
	image_list[TREE_IMAGE] = al_load_bitmap("data/images/tree.png");
	image_list[RESOURCES_IMAGE] = al_load_bitmap("data/images/resources.png");
	image_list[ROCK_RESOURCES_IMAGE] = al_load_bitmap("data/images/rock_resources.png");
	image_list[FERTILE_IMAGE] = al_load_bitmap("data/images/fertile.png");
	//image_list[STONE_IMAGE] = al_load_bitmap("data/images/stone.png");
	//image_list[IRON_IMAGE] = al_load_bitmap("data/images/iron.png");
	image_list[WATER_IMAGE] = al_load_bitmap("data/images/water.png");
	image_list[NO_WORKERS_IMAGE] = al_load_bitmap("data/images/no_workers.png");
	image_list[CLOSE_WINDOW_IMAGE] = al_load_bitmap("data/images/close_window.png");
	image_list[NO_IMAGE] = al_load_bitmap("data/images/no_image.png");

	bool ok = true;
	for(int i=0; i<LAST_IMAGE; i++)
	{
		if(!image_list[i])
		{
			ok = false;
			LOG("failed to load image number " << i);
			std::cout << "failed to load image number " << i << std::endl;
			//image_list[i] = al_load_bitmap("data/images/no_image.png");
			image_list[i] = al_create_bitmap(128, 64);
			al_set_target_bitmap(image_list[i]);
			al_clear_to_color(al_map_rgb(0,0,0));
			al_draw_text(font15, al_map_rgb(200, 50, 50), 64, 32, ALLEGRO_ALIGN_CENTRE, "No image.");
		}
	}
	return ok;
}

int delete_pictures()
{
	for(int i=0; i<LAST_IMAGE; i++)
	{
		al_destroy_bitmap(image_list[i]);
	}
	delete[] image_list;
	return 0;
}
/*Load fonts. If not successfull returns false.*/
bool load_fonts()
{
	font15 = al_load_font("data/arial.ttf", 15, 0);
	font20 = al_load_font("data/arial.ttf", 20, 0);
	font25 = al_load_font("data/arial.ttf", 25, 0);
	font30 = al_load_font("data/arial.ttf", 30, 0);
	
	if(!font15)
	{
		LOG("cant load fonts");
		return false;
	}
	return true;
}

int delete_fonts()
{
	al_destroy_font(font15);	
	al_destroy_font(font20);
	al_destroy_font(font25);
	al_destroy_font(font30);

	return 0;
}

int pow(int a, int b)
{
	int c = 1;
	for(int i=0; i<b; i++)
	{
		c = c * a;
	}
	return c;
}
/* Stops game (timer) and draws text on display. Waits for ENTER or mouse click from player, then resume game.*/
/*
int message(const char* text)
{
	LOG(text);

	al_stop_timer(game_info::timer);
	ALLEGRO_EVENT_QUEUE* queue = al_create_event_queue();
	ALLEGRO_EVENT* ev = new ALLEGRO_EVENT;
	ALLEGRO_MOUSE_STATE* mouse = new ALLEGRO_MOUSE_STATE; 
	
	al_register_event_source(queue, al_get_display_event_source(game_info::display));		
	al_register_event_source(queue, al_get_mouse_event_source());
	al_register_event_source(queue, al_get_keyboard_event_source());

	bool done = false;
	
	al_draw_filled_rectangle(100, 200, display_width - 100, display_height - 200, al_map_rgb(100, 100, 100));
	al_draw_text(font25, al_map_rgb(200, 200, 200), display_width/2, display_height/2, ALLEGRO_ALIGN_CENTRE, text);
	
	int x1 = display_width/2 - 100;
	int x2 = display_width/2 + 100;
	int y1 = display_height - 300;
	int y2 = display_height -250;


	al_draw_rectangle(x1, y1, x2, y2, al_map_rgb(150, 150, 150), 3);

	al_draw_text(font25, al_map_rgb(200, 200, 200), display_width/2, y1 + 10, ALLEGRO_ALIGN_CENTRE, "Continue");
	
	al_flip_display();

	while(!done)
	{
		al_wait_for_event(queue, ev);
		if(ev->type == ALLEGRO_EVENT_KEY_DOWN)
		{	
			if(ev->keyboard.keycode == ALLEGRO_KEY_ENTER)
				done = true;
		}
		
		else if(ev->type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN)
		{
			al_get_mouse_state(mouse);
			if(((mouse->x > x1) && (mouse->x < x2)) && ((mouse->y > y1) && (mouse->y < y2)))
				done = true;
		}
		else if(ev->type == ALLEGRO_EVENT_DISPLAY_RESIZE)
			resize_display(ev);		
	
		else if(ev->type == ALLEGRO_EVENT_DISPLAY_CLOSE)
		{
			game_info::close_display = true;
			done = true;
		}
	}

	al_destroy_event_queue(queue);

	delete ev;
	delete mouse;

	al_start_timer(game_info::timer);
	return 0;
}
*/
/*Change size of display. Should be called only if ALLEGRO_EVENT_DISPLAY_RESIZE was generated. Does not work with fullscreen display.*/
/*
int resize_display(ALLEGRO_EVENT* ev)
{
	al_acknowledge_resize(game_info::display);
	al_resize_display(game_info::display, ev->display.width, ev->display.height); 
	display_height = ev->display.height;
	display_width = ev->display.width;
	return 0;
}
*/
/*Loads fullscreen, display_width and height from file. If file not found, uses default values.*/
int load_settings()
{	
	//set default values
	game_info::fullscreen = true;
	game_info::fps = 60;
	game_info::music = false;
	display_width = 1366;
	display_height = 768;
	

	std::ifstream file;
	file.open("data/config.txt");
	if(!file)
		return 0;

	std::string line;

	while(!file.eof())
	{
		getline(file, line);
		
		LOG(line);

		std::vector<std::string> words = split(line);

		for(int i=0; i<words.size(); i++)
			lower_case(words[i]);
		
		if(words.size() >= 3)
		{
			if(words[1] == "=")
			{
				if(words[0] == "fullscreen")
				{
					if((words[2] == "true") || (words[2] == "1"))
						game_info::fullscreen = true;
					
					else if((words[2] == "false") || (words[2] == "0"))
						game_info::fullscreen = false;
				}
				
				if(words[0] == "music")
				{
					if((words[2] == "true") || (words[2] == "1"))
						game_info::music = true;
					
					else if((words[2] == "false") || (words[2] == "0"))
						game_info::music = false;
				}
				
				else if(words[0] == "display_width")
					display_width = string_to_int(words[2]);
				
				else if(words[0] == "display_height")
					display_height = string_to_int(words[2]);
				
				else if(words[0] == "fps")
					game_info::fps = string_to_int(words[2]);
			}
		}
	}
	return 0;
}

int string_to_int(const std::string& word)
{
	int number = 0;
	for(int i=0; i<word.length(); ++i)
	{
		if((word[i] >= static_cast<int>('0')) && (word[i] <= static_cast<int>('9')))
		{
			number *= 10;
			number += static_cast<int>(word[i]) - static_cast<int>('0');
		}
		else
		{
			LOG("cant convert " << word << " to integer");
			throw new std::exception;
		}
	}
	return number;
}

/*Makes all upper case letters lower case.*/
int lower_case(std::string& word)
{
	for(int i=0; i<word.size(); ++i)
	{
		if((word[i] >= 'A') && (word[i] <= 'Z'))
			word[i] = word[i] - int('A') + int('a');
	}
	return 0;
}
/*Splits line to words. Delimiters are only space and TAB. If "/" (comments) is found ignores rest of line. Number of words returns in variable number_of_words. To avoid memory leaks each word must be deleted and also array of words.*/
std::vector<std::string> split(std::string line)
{
	std::vector<std::string> words;
	
	bool done = false;

	while(!done)
	{
		while((line.find(" ") == 0) || (line.find("\t") == 0))		//remove white characters on the beginning
			line = line.substr(1);

		int space_index = line.find(" ");
		if(space_index == std::string::npos)
			space_index = line.find("\t");
			
		if(space_index == std::string::npos)
		{
			if(line.size() > 0)
				words.push_back(line);

			return words;
		}

		words.push_back(line.substr(0, space_index));
		line = line.substr(space_index + 1);
	}
	return words;
}

void remove_spaces_and_apostrophs(std::string & word)
{
	int start_index = 0;
	while((word[start_index] == ' ') || (word[start_index] == '\t'))
		start_index++;

	int end_index = word.size() - 1;
	while((word[end_index] == ' ') || (word[end_index] == '\t'))
		end_index--;

	if(word[start_index] == '"')
		start_index++;

	if(word[end_index] == '"')
		end_index--;

	word = word.substr(start_index, end_index - start_index + 1);
}
/*
struct save_window_parameters
{
	static const int start_x = 100;		//start of window with save files
	static const int start_y = 100;
	static const int height = 30;
	static const int space_after_end_y = 200;
	static const int option_distance = 150;
	static const int option_size = 100;

	static const std::vector<std::string> option_names()
	{
		return std::vector<std::string>{"Load", "Save", "Delete", "Return"};
	}
};

void draw_save_window(const std::vector<std::string>& save_files, const std::string& current_name, bool writing_name, int chosen_file, int first_file_index, int highlighted_option)
{	
	const int start_x = save_window_parameters::start_x;
	const int start_y = save_window_parameters::start_y;
	const int height = save_window_parameters::height;
	const int space_after_end_y = save_window_parameters::space_after_end_y;
	const int option_size = save_window_parameters::option_size;
	const int option_distance = save_window_parameters::option_distance;
	const std::vector<std::string> option_names = save_window_parameters::option_names();

	int drawed_x = 0;
	int drawed_y = 0;
	while(drawed_y < display_height)
	{
		while(drawed_x < display_width)
		{
			al_draw_bitmap(image_list[TEXTURE_GREY_IMAGE], drawed_x, drawed_y, 0);
			drawed_x += 1024;
		}
		drawed_y += 1024;
	}

	al_draw_text(font30, WRITING_COLOR, display_width / 2, 40, ALLEGRO_ALIGN_CENTRE, "Load / Save menu");
	al_draw_filled_rectangle(start_x, start_y, display_width - 100, display_height - space_after_end_y, BLACK_COLOR);

	for(int i=first_file_index; i<save_files.size(); ++i)
	{
		if((i - first_file_index) * 30 + start_y > display_height - space_after_end_y)
			break;

		ALLEGRO_COLOR color = WRITING_COLOR;
		if(i == chosen_file)
			color = YELLOW_COLOR;

		al_draw_textf(font20, color, start_x, start_y + (i - first_file_index)*height, ALLEGRO_ALIGN_LEFT, save_files[i].c_str());
	}

	al_draw_filled_rectangle(start_x, display_height - space_after_end_y + 30, display_width - 100, display_height - space_after_end_y + 60, BLACK_COLOR);
	ALLEGRO_COLOR c = WRITING_COLOR;
	if(writing_name)
		c = WHITE_COLOR;

	al_draw_textf(font20, c, start_x, display_height - space_after_end_y + 30, ALLEGRO_ALIGN_LEFT, current_name.c_str());

	for(int i=0; i<option_names.size(); ++i)
	{
		al_draw_rectangle(start_x + i*option_distance, display_height - 100, start_x + i*option_distance + option_size, display_height - 40, BLACK_COLOR, 4);
		ALLEGRO_COLOR color = WRITING_COLOR;
		if(i == highlighted_option)
			color = WHITE_COLOR;

		al_draw_textf(font25, color, start_x + i*option_distance + 15, display_height - 100 + 15, ALLEGRO_ALIGN_LEFT, option_names[i].c_str());
	}
	al_flip_display();
}

int compute_chosen_file(int x, int y, int first_file_index, const std::vector<std::string>& save_files)
{
	const int start_x = save_window_parameters::start_x;
	const int start_y = save_window_parameters::start_y;
	const int height = save_window_parameters::height;

	if((y > display_height - save_window_parameters::space_after_end_y) || (x > display_width - 100) || (x < start_x))
		return -1;

	for(int i=first_file_index; i<save_files.size(); ++i)
	{
		if((y > start_y + height*i) && (y < start_y + height*(i + 1)))
			return i;
	}
	return -1;
}

int compute_highlighted_option()
{
	static ALLEGRO_MOUSE_STATE* state = new ALLEGRO_MOUSE_STATE;
	al_get_mouse_state(state);
	
	int x = state->x;
	int y = state->y;

	if((y > display_height - 40) || (y < display_height - 100))
		return -1;
	
	const std::vector<std::string> option_names = save_window_parameters::option_names();
	const int start_x = save_window_parameters::start_x;
	const int option_size = save_window_parameters::option_size;
	const int option_distance = save_window_parameters::option_distance;

	for(int i=0; i<option_names.size(); ++i)
	{
		if((x > start_x + i*(option_distance)) && (x < start_x + i*(option_distance) + option_size))
			return i;
	}
	return -1;
}

bool click_to_writing_name(int x, int y)
{
	const int start_x = save_window_parameters::start_x;
	const int space_after_end_y = save_window_parameters::space_after_end_y;
	if((x > start_x) && (x < start_x - 100) && (y > display_height - space_after_end_y + 30) && (y < display_height - space_after_end_y + 60))
		return true;
	
	return false;
}

void execute_save_window_option(int option, bool& done, const std::string& current_name, std::vector<std::string>& file_names, int& chosen_file)
{
	switch(option)		
	{
		case(0):		//load
		{
			std::ifstream file_to_load;
			file_to_load.open("data/saves/" + current_name);
			if(!file_to_load.good())
			{
				LOG("cannot load: " << "data/saves/" + current_name);
				throw new std::exception;
				return;
			}
			boost::archive::text_iarchive archive(file_to_load);
			archive >> *session;
			for(int i=0; i<session->tile_list.size(); ++i)
			{
				for(int j=0; j<session->tile_list[i].size(); ++j)
					session->tile_list[i][j]->finish_serialization();
			}
		}
		break;
		case(1):		//save
		{
			std::ofstream file_to_save("data/saves/" + current_name, std::ofstream::out);
			if(file_to_save.bad())
				return;
			
			for(int i=0; i<session->tile_list.size(); ++i)
			{
				for(int j=0; j<session->tile_list[i].size(); ++j)
					session->tile_list[i][j]->prepare_serialization();
			}

			boost::archive::text_oarchive archive(file_to_save);
			archive << *session;
			if(!contains(file_names, current_name))
				file_names.push_back(current_name);
		}
		break;
		case(2):		//delete
		{
			remove(("data/saves/" + current_name).c_str());
			for(int i=0; i<file_names.size(); ++i)
			{
				if(file_names[i] == current_name)
					file_names.erase(file_names.begin() + i);
			}
			chosen_file = -1;

		}
		break;
		case(3):		//return
		{
			done = true;
		}
		break;
	}
}

std::vector<std::string> find_save_files()
{
	std::string string_path = "data/saves/";
	boost::filesystem::path path_to_saves(string_path);
	boost::filesystem::directory_iterator files_it(path_to_saves);
	boost::filesystem::directory_iterator end_it;
	
	std::vector<std::string> file_names;

	while(files_it != end_it)
	{
		boost::filesystem::path file = *files_it;
		std::string file_name = file.string().substr(string_path.size());
		file_names.push_back(file_name);
		files_it++;
	}
	return file_names;	
}

void save_window(bool& display_closed)
{
	std::vector<std::string> save_files = find_save_files();

	std::string current_name = "";
	bool writing_name = true;
	int chosen_file = -1;
	int first_file_index = 0;
	int highlighted_option = -1;
	draw_save_window(save_files, current_name, writing_name, chosen_file, first_file_index, highlighted_option);
	
	ALLEGRO_EVENT* ev = new ALLEGRO_EVENT;
	ALLEGRO_EVENT* next_event = new ALLEGRO_EVENT;
	ALLEGRO_EVENT_QUEUE* queue = al_create_event_queue();
	ALLEGRO_TIMER* t = al_create_timer(1.0/60.0);
	al_register_event_source(queue, al_get_timer_event_source(t));
	al_register_event_source(queue, al_get_display_event_source(game_info::display));
	al_register_event_source(queue, al_get_mouse_event_source());
	al_register_event_source(queue, al_get_keyboard_event_source());

	std::unique_ptr<ALLEGRO_KEYBOARD_STATE> state = std::unique_ptr<ALLEGRO_KEYBOARD_STATE>(new ALLEGRO_KEYBOARD_STATE);
	
	al_start_timer(t);
	al_flush_event_queue(queue);

	bool done = false;
	while(!done)
	{
		al_wait_for_event(queue, ev);
		
		if(ev->type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN)
		{
			int new_chosen_file = compute_chosen_file(ev->mouse.x, ev->mouse.y, first_file_index, save_files);
			if(new_chosen_file != -1)
			{
				current_name = save_files[new_chosen_file];
				writing_name = false;
				chosen_file = new_chosen_file;
			}

			int new_highlighted_option = compute_highlighted_option();
			if(new_highlighted_option != -1)
			{
				execute_save_window_option(new_highlighted_option, done, current_name, save_files, chosen_file);
			}
			if(click_to_writing_name)
			{
				writing_name = true;
			}
		}
		else if(ev->type == ALLEGRO_EVENT_MOUSE_AXES)
		{
			highlighted_option = compute_highlighted_option();
		}
		else if(ev->type == ALLEGRO_EVENT_KEY_DOWN)
		{
			if(ev->keyboard.keycode == ALLEGRO_KEY_ESCAPE)
				done = true;
			
			else if(ev->keyboard.keycode == ALLEGRO_KEY_ENTER)
			{
				writing_name = false;
			}
			else if(ev->keyboard.keycode == ALLEGRO_KEY_BACKSPACE)
			{
				if(current_name.size() > 0)
					current_name.pop_back();
			}
			else if((ev->keyboard.keycode >= ALLEGRO_KEY_A) && (ev->keyboard.keycode <= ALLEGRO_KEY_Z))
			{
				current_name.push_back(static_cast<char>(ev->keyboard.keycode - ALLEGRO_KEY_A) + 'a');
			}
			else if((ev->keyboard.keycode >= ALLEGRO_KEY_0) && (ev->keyboard.keycode <= ALLEGRO_KEY_9))
			{
				current_name.push_back(static_cast<char>(ev->keyboard.keycode - ALLEGRO_KEY_0) + '0');
			}
			else if(ev->keyboard.keycode == ALLEGRO_KEY_MINUS)
			{
				current_name.push_back('_');
			}

			if(current_name.size() > 50)
				current_name.erase(current_name.begin() + 50, current_name.end());
		}
		else if(ev->type == ALLEGRO_EVENT_TIMER)
		{
			if(writing_name)
			{
				al_get_keyboard_state(state.get());
				if((al_key_down(state.get(), ALLEGRO_KEY_BACKSPACE)) && (current_name.size() > 0))
				{
					current_name.pop_back();
				}
			}
			draw_save_window(save_files, current_name, writing_name, chosen_file, first_file_index, highlighted_option);
		}
		else if(ev->type == ALLEGRO_EVENT_DISPLAY_CLOSE)
		{	
			game_info::close_display = true;
			done = true;
			display_closed = true;
		}
		else if(ev->type == ALLEGRO_EVENT_DISPLAY_RESIZE)
			resize_display(ev);
	}

	delete ev;
	al_destroy_timer(t);
	al_destroy_event_queue(queue);
}
*/
