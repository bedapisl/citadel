#include "core.h"

std::ofstream log_file("log", std::ios::out); 

int display_width;
int display_height;

ALLEGRO_FONT* font15;
ALLEGRO_FONT* font20;
ALLEGRO_FONT* font25;
ALLEGRO_FONT* font30;
ALLEGRO_FONT* font35;
ALLEGRO_FONT* font40;
ALLEGRO_FONT* font45;

ALLEGRO_BITMAP** image_list;

game_session* session = NULL;

int main(int argc, char** argv)
{
	if((argc == 2) && (std::strcmp(argv[1], "-v")  || std::strcmp(argv[1], "--version")))
	{
		std::cout << VERSION << std::endl;
		return 0;
	}

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
	game_info::load_game_info();

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
	ALLEGRO_DISPLAY_MODE display_mode;

	if(game_info::fullscreen)
	{
		al_set_new_display_flags(ALLEGRO_FULLSCREEN_WINDOW);

		for(int i=0; i<al_get_num_display_modes(); ++i)
		{
			al_get_display_mode(i, &display_mode);

			if(display_width < display_mode.width)
			{
				display_width = display_mode.width;
				display_height = display_mode.height;
			}
		}
		LOG("Fullscreen display");
	}
	else
	{
		al_set_new_display_flags(ALLEGRO_WINDOWED);
		al_set_new_display_flags(ALLEGRO_RESIZABLE);
		LOG("Windowed display");
	}

	al_set_new_window_position(10, 10);

	LOG("Creating display - width: " << display_width << " height: " << display_height);
	game_info::display = al_create_display(display_width, display_height);

	if(!game_info::fullscreen)		//Dont delete this. It looks useless, but it probably prevents bug in Allegro, which causes
						//wrong mouse cursor location. I don't know how to reproduce this bug, it sometimes happens, so even if game works
						//without this, dont delete it.
	{
		al_acknowledge_resize(game_info::display);
		al_resize_display(game_info::display, display_width, display_height);
	}
}

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
	image_list[BUILDING_SELECTION_IMAGE] = al_load_bitmap("data/images/building_selection.png");
	//image_list[BUTTON_BACKGROUND_IMAGE] = al_load_bitmap("data/images/button_background.png");
	//image_list[BIGGER_BUTTON_BACKGROUND_IMAGE] = al_load_bitmap("data/images/bigger_button_background.png");
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
	font35 = al_load_font("data/arial.ttf", 35, 0);
	font40 = al_load_font("data/arial.ttf", 40, 0);
	font45 = al_load_font("data/arial.ttf", 45, 0);

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

