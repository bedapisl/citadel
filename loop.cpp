
#include "core.h"

extern std::ofstream log_file;
extern game_session* session;
extern int display_width;
extern int display_height;

extern ALLEGRO_FONT* font20;
extern ALLEGRO_FONT* font25;
extern ALLEGRO_FONT* font30;

BOOST_CLASS_EXPORT(warehouse)
BOOST_CLASS_EXPORT(tower)
BOOST_CLASS_EXPORT(barracks)
BOOST_CLASS_EXPORT(production_building)
BOOST_CLASS_EXPORT(wall)
BOOST_CLASS_EXPORT(gate)
BOOST_CLASS_EXPORT(house)
BOOST_CLASS_EXPORT(market)
BOOST_CLASS_EXPORT(great_hall)
BOOST_CLASS_EXPORT(church)
BOOST_CLASS_EXPORT(store)
BOOST_CLASS_EXPORT(scout)
BOOST_CLASS_EXPORT(stairs)

BOOST_CLASS_EXPORT(carrier)
BOOST_CLASS_EXPORT(warrior)

event_handler::event_handler() : queue(al_create_event_queue()), ev(new ALLEGRO_EVENT), next_event(new ALLEGRO_EVENT), timer(al_create_timer(1.0 / game_info::fps)), mouse_state(new ALLEGRO_MOUSE_STATE) , current_loop(nullptr), done(false)
{
	al_register_event_source(queue, al_get_display_event_source(game_info::display));
	al_register_event_source(queue, al_get_mouse_event_source());
	al_register_event_source(queue, al_get_timer_event_source(timer));
	al_register_event_source(queue, al_get_keyboard_event_source());

	loops.clear();
	loops.push_back(boost::shared_ptr<loop>(new game_loop));
	loops.push_back(boost::shared_ptr<loop>(new main_menu));
	loops.push_back(boost::shared_ptr<loop>(new settings_menu));
	loops.push_back(boost::shared_ptr<loop>(new ingame_menu));
	loops.push_back(boost::shared_ptr<loop>(new save_menu));
	loops.push_back(boost::shared_ptr<loop>(new random_game_settings));

	al_start_timer(timer);
}

event_handler::~event_handler()
{
	al_destroy_event_queue(queue);
	delete ev;
	delete next_event;
	delete mouse_state;
	al_destroy_timer(timer);
}

event_handler& event_handler::get_instance()
{
	static event_handler handler;
	return handler;
}

void event_handler::run()
{
	if(current_loop == nullptr)
		throw new std::exception;

	done = false;

	bool timer_event = false;

	while(!done)
	{
		al_wait_for_event(queue, ev);
		//al_get_next_event(queue, ev);
		{
			switch(ev->type)
			{
				case(ALLEGRO_EVENT_KEY_DOWN):
					handle_key_down(ev);
					break;
				case(ALLEGRO_EVENT_KEY_UP):
					current_loop->key_up(ev);
					break;
				case(ALLEGRO_EVENT_KEY_CHAR):
					current_loop->key_char(ev);
					break;
				case(ALLEGRO_EVENT_MOUSE_AXES):
					current_loop->mouse_axes(ev);
					break;
				case(ALLEGRO_EVENT_MOUSE_BUTTON_DOWN):
				{
					if(ev->mouse.button == 1)
						current_loop->mouse_left_down(ev);
					else if(ev->mouse.button == 2)
						current_loop->mouse_right_down(ev);
				}
				break;
				case(ALLEGRO_EVENT_MOUSE_BUTTON_UP):
				{
					if(ev->mouse.button == 1)
						current_loop->mouse_left_up(ev);
					else if(ev->mouse.button == 2)
						current_loop->mouse_right_up(ev);
				}
				break;
				case(ALLEGRO_EVENT_TIMER):
					timer_event = true;		//this will happen after everything else is done
					break;
				case(ALLEGRO_EVENT_DISPLAY_RESIZE):
					resize_window(ev);
					break;
				case(ALLEGRO_EVENT_DISPLAY_CLOSE):
					display_close();
					break;
				default:
					{}
			}
			
			if(timer_event && al_is_event_queue_empty(queue))
			{
				al_get_mouse_state(mouse_state);
				music::get_instance().update();
				current_loop->timer(ev, mouse_state->x, mouse_state->y);
			}
		}
	}
}

void event_handler::change_state(game_state state)
{
	if(current_loop != nullptr)
		current_loop->end();

	current_loop = loops[static_cast<int>(state)].get();
	current_loop->start();
}

void event_handler::handle_key_down(ALLEGRO_EVENT* ev)
{
	switch(ev->keyboard.keycode)
	{
		case(ALLEGRO_KEY_ESCAPE):
			current_loop->escape_down(ev);
			break;
		case(ALLEGRO_KEY_ENTER):
			current_loop->enter_down(ev);
			break;
		case(ALLEGRO_KEY_UP):
			current_loop->up_arrow_down(ev);
			break;
		case(ALLEGRO_KEY_DOWN):
			current_loop->down_arrow_down(ev);
			break;
		case(ALLEGRO_KEY_LEFT):
			current_loop->left_arrow_down(ev);
			break;
		case(ALLEGRO_KEY_RIGHT):
			current_loop->right_arrow_down(ev);
			break;
		default:
			current_loop->other_key_down(ev);
	}
}

void event_handler::resize_window(ALLEGRO_EVENT* ev)
{
	al_acknowledge_resize(game_info::display);
	al_resize_display(game_info::display, ev->display.width, ev->display.height);
	display_height = ev->display.height;
	display_width = ev->display.width;
}

void event_handler::display_close()
{
	if(current_loop != nullptr)
		current_loop->end();
	done = true;
}

game_loop::game_loop() : mouse(new game_mouse), minimap(nullptr), keyboard_state(new ALLEGRO_KEYBOARD_STATE), screen_position_x(0), screen_position_y(0), number_of_frames(0)
{
}

game_loop::~game_loop()
{
	delete mouse;
	delete keyboard_state;

	if(minimap != nullptr)
		al_destroy_bitmap(minimap);
}

void game_loop::escape_down(ALLEGRO_EVENT* ev)
{
	event_handler::get_instance().change_state(game_state::INGAME_MENU);
}

void game_loop::enter_down(ALLEGRO_EVENT* ev)
{
	LOG("Enter");
	mouse->left_button_go_down(screen_position_x, screen_position_y, new rotation);
	mouse->left_button_go_up(screen_position_x, screen_position_y);
}

void game_loop::other_key_down(ALLEGRO_EVENT* ev)
{
	if((ev->keyboard.keycode >= ALLEGRO_KEY_0) && (ev->keyboard.keycode <= ALLEGRO_KEY_9))
	{							//shortcuts
		LOG("shortcut key down");
		int button_index = ev->keyboard.keycode - ALLEGRO_KEY_1;
		if(button_index == -1)
			button_index += 10;
		if(mouse->chosen_building.expired())		//build new building
		{
			if(button_index < (*button::button_list).size())
			{
				mouse->choose_button((*button::button_list)[button_index]);
			}
		}
		else if(!mouse->chosen_building.expired())	//actions of building (e.g. make warrior) 
		{	
			mouse->chosen_building.lock()->function_click(button_index * BUTTON_SIZE + BUTTON_SIZE/2, display_height - BUTTON_SIZE/2);
		}
	}
}

void game_loop::mouse_axes(ALLEGRO_EVENT* ev)
{
	mouse->move(ev->mouse.x, ev->mouse.y);
}

void game_loop::mouse_left_down(ALLEGRO_EVENT* ev)
{
	rotation rotate = NO_ROTATION;
	mouse->left_button_go_down(screen_position_x, screen_position_y, &rotate);

	if(rotate == RIGHT_ROTATION)
		rotate_map(true, &minimap, &screen_position_x, &screen_position_y);
	else if(rotate == LEFT_ROTATION)
		rotate_map(false, &minimap, &screen_position_x, &screen_position_y);
}

void game_loop::mouse_left_up(ALLEGRO_EVENT* ev)
{
	mouse->left_button_go_up(screen_position_x, screen_position_y);
}

void game_loop::mouse_right_down(ALLEGRO_EVENT* ev)
{
	mouse->right_button_go_down();
}

void game_loop::mouse_right_up(ALLEGRO_EVENT* ev)
{
	mouse->right_button_go_up();
}

void game_loop::timer(ALLEGRO_EVENT* ev, int mouse_x, int mouse_y)
{
	gettimeofday(&time, NULL);
	starting_time = time.tv_usec + time.tv_sec*1000000;

	number_of_frames++;
	LOG("EVENT_TIMER - frame number:" << number_of_frames);

	screen_move(mouse->state, keyboard_state, screen_position_x, screen_position_y);
	mouse->move(screen_position_x, screen_position_y);	//if screen moves, mouse "pseudo" moves too
	
	draw();

	gettimeofday(&time, NULL);
	drawing_time = time.tv_usec + time.tv_sec*1000000;

	LOG("updating people");

	for(int i = 0; i<session->people_list.size(); i++)
	{
		session->people_list[i]->update();
	}

	gettimeofday(&time, NULL);
	people_time = time.tv_usec + time.tv_sec*1000000;

	LOG("updating buildings");

	building::assign_workers();
	for(int i=0; i<session->building_list.size(); i++)
	{
		session->building_list[i]->update();
	}

	gettimeofday(&time, NULL);
	buildings_time = time.tv_usec + time.tv_sec*1000000;

	LOG("updating missiles");

	for(int i = 0; i<session->missile_list.size(); i++)
	{
		session->missile_list[i]->draw_missile(screen_position_x, screen_position_y);
	}
	
	update_minimap(minimap);
	
	bool done = false;
	session->update(mouse, done);
	if(done)
		event_handler::get_instance().change_state(game_state::MAIN_MENU);
		
	gettimeofday(&time, NULL);
	session_time = time.tv_usec + time.tv_sec*1000000;
	delete_death(mouse);

	gettimeofday(&time, NULL);
	rest = time.tv_usec + time.tv_sec*1000000;

	LOG("drawing: " << drawing_time - starting_time << " updating people: " << people_time - drawing_time << 
			" updating buildings: " << buildings_time - people_time << " updating session " << session_time - buildings_time << 
			" rest: " << rest - session_time);

	LOG("end of frame");
}

void game_loop::start()
{
	if(minimap != nullptr)
		al_destroy_bitmap(minimap);
	minimap = create_minimap();
}

void game_loop::draw()
{
	draw_map(screen_position_x, screen_position_y, mouse);
	window::draw_active_windows();
	draw_main_panel(minimap, screen_position_x, screen_position_y, mouse, session);
	
	al_flip_display();
}

ingame_menu::ingame_menu() : chosen_option(ingame_menu_options::CONTINUE)
{ }

void ingame_menu::escape_down(ALLEGRO_EVENT* ev)
{
	event_handler::get_instance().change_state(game_state::GAME);
}

void ingame_menu::enter_down(ALLEGRO_EVENT* ev)
{
	execute_option(chosen_option);
}

void ingame_menu::up_arrow_down(ALLEGRO_EVENT* ev)
{
	chosen_option = static_cast<ingame_menu_options>(static_cast<int>(chosen_option) - 1);
	if(static_cast<int>(chosen_option) < 0)
		chosen_option = ingame_menu_options::EXIT;
}

void ingame_menu::down_arrow_down(ALLEGRO_EVENT* ev)
{
	chosen_option = static_cast<ingame_menu_options>(static_cast<int>(chosen_option) + 1);
	if(static_cast<int>(chosen_option) > static_cast<int>(ingame_menu_options::EXIT))
		chosen_option = ingame_menu_options::CONTINUE;
}

void ingame_menu::mouse_axes(ALLEGRO_EVENT* ev)
{
	chosen_option = compute_ingame_menu_options(ev->mouse.x, ev->mouse.y);
}

void ingame_menu::mouse_left_down(ALLEGRO_EVENT* ev)
{
	ingame_menu_options clicked_option = compute_ingame_menu_options(ev->mouse.x, ev->mouse.y);
	execute_option(clicked_option);
}

void ingame_menu::timer(ALLEGRO_EVENT* ev, int mouse_x, int mouse_y)
{
	draw_ingame_menu();
}

void ingame_menu::start()
{
	game_bitmap = al_clone_bitmap(al_get_backbuffer(game_info::display));
	chosen_option = ingame_menu_options::CONTINUE;
}

void ingame_menu::end()
{
	al_destroy_bitmap(game_bitmap);
}

void ingame_menu::execute_option(ingame_menu_options option)
{
	switch(option)
	{
		case(ingame_menu_options::CONTINUE):
			event_handler::get_instance().change_state(game_state::GAME);
			break;
		case(ingame_menu_options::SAVE):
			event_handler::get_instance().change_state(game_state::SAVE_MENU);
			break;
		case(ingame_menu_options::EXIT):
		{
			delete session;
			session = nullptr;
			event_handler::get_instance().change_state(game_state::MAIN_MENU);
		}
			break;
		case(ingame_menu_options::NO_OPTION):
			break;
		default:
			throw new std::exception;
	}
}

void ingame_menu::draw_ingame_menu()
{
	al_draw_bitmap(game_bitmap, 0, 0, 0);		//draws map and panel

	int menu_start_x = (display_width - menu_width) / 2;
	int menu_end_x = (display_width + menu_width) / 2;
	al_draw_bitmap_region(image_list[TEXTURE_GREY_IMAGE], 0, 0, menu_width, options_names.size() * button_distance, menu_start_x, menu_start_y, 0);

	int chosen_option_number = static_cast<int>(chosen_option);
	
	for(int i=0; i<options_names.size(); ++i)
	{
		int start_y = menu_start_y + button_start_y + i*button_distance;
		al_draw_filled_rectangle(menu_start_x + button_start_x, start_y, menu_end_x - button_start_x, start_y + button_heigth, BLACK_COLOR);
		
		ALLEGRO_COLOR color = WRITING_COLOR;
		ALLEGRO_FONT* font = font25;
		if(chosen_option_number == i)
		{
			color = WHITE_COLOR;
			font = font30;
		}

		al_draw_text(font, color, display_width / 2, start_y + 10, ALLEGRO_ALIGN_CENTRE, options_names[i].c_str());
	}
	al_flip_display();
}

ingame_menu_options ingame_menu::compute_ingame_menu_options(int x, int y)
{
	if(x < ((display_width - menu_width)/2 + button_start_x) ||
		x > ((display_width + menu_width)/2 - button_start_x))
		return ingame_menu_options::NO_OPTION;

	int start_y = menu_start_y + button_start_y;
	int end_y = menu_start_y + button_start_y + button_heigth;

	for(int i=0; i<options_names.size(); ++i)
	{
		if((y >= start_y) && (y <= end_y))
			return static_cast<ingame_menu_options>(i);
		
		start_y += button_distance;
		end_y += button_distance;
	}
	return ingame_menu_options::NO_OPTION;
}

save_menu::save_menu() : writing_name(true), chosen_file(-1), first_file_index(0), highlighted_option(-1)
{ }

void save_menu::escape_down(ALLEGRO_EVENT* ev)
{
	if(session == nullptr)
		event_handler::get_instance().change_state(game_state::MAIN_MENU);
	
	else
		event_handler::get_instance().change_state(game_state::GAME);
}

void save_menu::enter_down(ALLEGRO_EVENT* ev)
{
	writing_name = false;
}

void save_menu::up_arrow_down(ALLEGRO_EVENT* ev)
{
	chosen_file--;
	if(chosen_file < 0)
		chosen_file = save_files.size() - 1;
}
	
void save_menu::down_arrow_down(ALLEGRO_EVENT* ev)
{
	chosen_file++;
	if(chosen_file >= save_files.size())
		chosen_file = 0;
}

void save_menu::key_char(ALLEGRO_EVENT* ev)
{
	if(!writing_name)
		return;

	char c = ev->keyboard.unichar;
	if(((c >= 'a') && (c <= 'z')) || ((c >= 'A') && (c <= 'Z')) || ((c >= '0') && (c <= '9')) || (c == '_') || (c == '-'))
	{
		if(current_name.size() < 50)
			current_name += c;
	}

	if((ev->keyboard.keycode == ALLEGRO_KEY_BACKSPACE) || (ev->keyboard.keycode == ALLEGRO_KEY_DELETE))
	{
		if(current_name.size() > 0)
			current_name.pop_back();
	}
}

void save_menu::mouse_axes(ALLEGRO_EVENT* ev)
{
	highlighted_option = compute_highlighted_option(ev->mouse.x, ev->mouse.y);
}

void save_menu::mouse_left_down(ALLEGRO_EVENT* ev)
{
	int new_chosen_file = compute_chosen_file(ev->mouse.x, ev->mouse.y);
	if(new_chosen_file != -1)
	{
		current_name = save_files[new_chosen_file];
		writing_name = false;
		chosen_file = new_chosen_file;
	}

	int new_highlighted_option = compute_highlighted_option(ev->mouse.x, ev->mouse.y);
	if(new_highlighted_option != -1)
		execute_save_window_option(new_highlighted_option);
	
	if(click_to_writing_name(ev->mouse.x, ev->mouse.y))
		writing_name = true;
}

void save_menu::timer(ALLEGRO_EVENT* ev, int x, int y)
{
	draw_save_window();
}

void save_menu::start()
{
	current_name = "";
	writing_name = true;
	chosen_file = -1;
	first_file_index = 0;
	highlighted_option = -1;
}

void save_menu::draw_save_window()
{	
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

		al_draw_textf(font20, color, start_x, start_y + (i - first_file_index)*height, ALLEGRO_ALIGN_LEFT, "%s", save_files[i].c_str());
	}

	al_draw_filled_rectangle(start_x, display_height - space_after_end_y + 30, display_width - 100, display_height - space_after_end_y + 60, BLACK_COLOR);
	ALLEGRO_COLOR c = WRITING_COLOR;
	if(writing_name)
		c = WHITE_COLOR;

	al_draw_textf(font20, c, start_x, display_height - space_after_end_y + 30, ALLEGRO_ALIGN_LEFT, "%s", current_name.c_str());

	for(int i=0; i<option_names.size(); ++i)
	{
		al_draw_rectangle(start_x + i*option_distance, display_height - 100, start_x + i*option_distance + option_size, display_height - 40, BLACK_COLOR, 4);
		ALLEGRO_COLOR color = WRITING_COLOR;
		if(i == highlighted_option)
			color = WHITE_COLOR;

		al_draw_textf(font25, color, start_x + i*option_distance + 15, display_height - 100 + 15, ALLEGRO_ALIGN_LEFT, "%s", option_names[i].c_str());
	}
	al_flip_display();
}

int save_menu::compute_chosen_file(int x, int y)
{
	if((y > display_height - space_after_end_y) || (x > display_width - 100) || (x < start_x))
		return -1;

	for(int i=first_file_index; i<save_files.size(); ++i)
	{
		if((y > start_y + height*i) && (y < start_y + height*(i + 1)))
			return i;
	}
	return -1;
}

int save_menu::compute_highlighted_option(int x, int y)
{
	if((y > display_height - 40) || (y < display_height - 100))
		return -1;
	
	for(int i=0; i<option_names.size(); ++i)
	{
		if((x > start_x + i*(option_distance)) && (x < start_x + i*(option_distance) + option_size))
			return i;
	}
	return -1;
}

bool save_menu::click_to_writing_name(int x, int y)
{
	if((x > start_x) && (x < display_width - 100) && (y > display_height - space_after_end_y + 30) && (y < display_height - space_after_end_y + 60))
		return true;
	
	return false;
}

void save_menu::execute_save_window_option(int option)
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
			delete session;
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
			if(current_name.size() == 0)
				return;
			if(session == nullptr)
				return;

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
			if(!contains(save_files, current_name))
				save_files.push_back(current_name);
		}
		break;
		case(2):		//delete
		{
			remove(("data/saves/" + current_name).c_str());
			for(int i=0; i<save_files.size(); ++i)
			{
				if(save_files[i] == current_name)
					save_files.erase(save_files.begin() + i);
			}
			chosen_file = -1;

		}
		break;
		case(3):		//return
		{
			game_state new_state = game_state::MAIN_MENU;
			if(session != nullptr)
				new_state = game_state::GAME;
			event_handler::get_instance().change_state(new_state);
		}
		break;
	}
}

std::vector<std::string> save_menu::find_save_files()
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

main_menu::main_menu() : chosen_option(RANDOM_GAME)
{ }
 
void main_menu::escape_down(ALLEGRO_EVENT* ev)
{
	event_handler::get_instance().quit();
}

void main_menu::enter_down(ALLEGRO_EVENT* ev)
{
	execute_option(chosen_option);
}

void main_menu::up_arrow_down(ALLEGRO_EVENT* ev)
{
	chosen_option = (main_menu_option)((int)chosen_option - 1);			//the same as chosen_option-- 
	if(chosen_option < 0)
		chosen_option = QUIT;
}

void main_menu::down_arrow_down(ALLEGRO_EVENT* ev)
{
	chosen_option = (main_menu_option)((int)chosen_option + 1);			//the same as chosen_option++ 
	if(chosen_option > 4)
		chosen_option = static_cast<main_menu_option>(0);
}

void main_menu::mouse_axes(ALLEGRO_EVENT* ev)
{
	chosen_option = compute_main_menu_option(ev->mouse.x, ev->mouse.y);
}

void main_menu::mouse_left_down(ALLEGRO_EVENT* ev)
{
	execute_option(compute_main_menu_option(ev->mouse.x, ev->mouse.y));
}

void main_menu::timer(ALLEGRO_EVENT* ev, int x, int y)
{
	draw_main_menu();
}

void main_menu::execute_option(main_menu_option option)
{
	switch(option)
	{
		case(RANDOM_GAME):
		{	
			event_handler::get_instance().change_state(game_state::RANDOM_GAME_SETTINGS);
		}
		break;
		case(CAMPAIGN):
		{
			//todo
		}
		break;
		case(LOAD_GAME):
		{
			event_handler::get_instance().change_state(game_state::SAVE_MENU);
		}
		break;
		case(SETTINGS):
		{
			event_handler::get_instance().change_state(game_state::SETTINGS);
		}
		break;
		case(QUIT):
		{
			event_handler::get_instance().quit();
		}
		break;
		case(NO_OPTION):
		{ }
	}
}

main_menu_option main_menu::compute_main_menu_option(int x, int y)
{
	if((x > display_width/2 - 60) & (x < display_width/2 + 60))
	{
		if(((y - 100) / 80) < static_cast<int>(NO_OPTION))			
		{
			return (main_menu_option)((y - 100) / 80);	//???
		}
	}
	return NO_OPTION;
}

void main_menu::draw_main_menu()
{
	al_clear_to_color(al_map_rgb(0,0,0));

	std::vector<std::string> options_names{"Random game", "Campaign", "Load game", "Settings", "Quit"};
	
	for(int i=0; i<options_names.size(); i++)
	{
		if(chosen_option != i)
			al_draw_textf(font25, al_map_rgb(150, 150, 150), display_width/2, 100 + 80*i, ALLEGRO_ALIGN_CENTRE, "%s", options_names[i].c_str());  
		else
			al_draw_textf(font30, al_map_rgb(200, 200, 200), display_width/2, 100 + 80*i, ALLEGRO_ALIGN_CENTRE, "%s", options_names[i].c_str()); 
	}
	
	al_flip_display();
}

settings_menu::settings_menu() : done_button("Done")
{
	buttons.push_back(switch_button("Fullscreeen", game_info::fullscreen));
	buttons.push_back(switch_button("Music", game_info::music));
	
	text_fields.push_back(text_field("Display width", std::to_string(display_width), true));
	text_fields.push_back(text_field("Display height", std::to_string(display_height), true));
	text_fields.push_back(text_field("FPS", std::to_string(game_info::fps), true));

	update_gui_elements_position();
}

void settings_menu::escape_down(ALLEGRO_EVENT* ev)
{
	event_handler::get_instance().change_state(game_state::MAIN_MENU);
}

void settings_menu::key_char(ALLEGRO_EVENT* ev)
{
	for(text_field& f : text_fields)
		f.key_char(ev);
}

void settings_menu::mouse_axes(ALLEGRO_EVENT* ev)
{
	for(switch_button& b : buttons)
		b.update_mouse_position(ev->mouse.x, ev->mouse.y);

	done_button.update_mouse_position(ev->mouse.x, ev->mouse.y);
}

void settings_menu::mouse_left_down(ALLEGRO_EVENT* ev)
{
	for(switch_button& b : buttons)
		b.mouse_down(ev->mouse.x, ev->mouse.y);

	for(text_field& f : text_fields)
		f.mouse_down(ev->mouse.x, ev->mouse.y);

	if(done_button.mouse_on_button(ev->mouse.x, ev->mouse.y))
	{
		event_handler::get_instance().change_state(game_state::MAIN_MENU);
	}
}

void settings_menu::timer(ALLEGRO_EVENT* ev, int x, int y)
{
	draw_settings();
}

void settings_menu::draw_settings()
{
	update_gui_elements_position();
	
	al_clear_to_color(BLACK_COLOR);

	al_draw_text(font30, WRITING_COLOR, display_width/2, header_height / 2, ALLEGRO_ALIGN_CENTRE, "Settings");

	for(const switch_button& b : buttons)
		b.draw();
	
	for(const text_field& t : text_fields)
		t.draw();
	
	done_button.draw();
	al_flip_display();
}

void settings_menu::update_gui_elements_position()
{
	int x = 100;
	int y = header_height;
	
	for(switch_button& b : buttons)
	{
		b.update_position(x, y);
		y += 50;
	}

	for(text_field& t : text_fields)
	{
		t.update_position(x, y);
		y += 50;
	}

	x = display_width - 200;
	y = std::max(display_height - 100, y);

	done_button.update_position(x, y);
}

void settings_menu::start()
{
}

void settings_menu::end()
{
	std::ofstream file;
	file.open("data/config.txt", std::ios::out);

	file << "fullscreen = ";
	if(buttons[0].get_value())
		file << "true\n";

	else
		file << "false\n";

	file << "display_width = " << text_fields[0].get_value() << std::endl << "display_height = " << text_fields[1].get_value() << std::endl << "fps = " << text_fields[2].get_value() << std::endl << "music = ";

	if(buttons[1].get_value())
		file << "true" << std::endl;	
	else
		file << "false" << std::endl;

	file.close();
}
	
random_game_settings::random_game_settings()
{
	text_fields.push_back(text_field("Honour", "0", true));
	for(int i=0; i<resources_names.size(); ++i)
		text_fields.push_back(text_field(resources_names[i], resources_initial_values[i], true));

	sliders.push_back(slider("Enemies", 1, 2));
	sliders.push_back(slider("Mountains", 1, 2));
	
	for(int i=0; i<natural_resources_names.size(); ++i)
		sliders.push_back(slider(natural_resources_names[i], 1, 2));
	
	buttons.push_back(menu_button("Back"));
	buttons.push_back(menu_button("Start game"));

	highlighted_option = -1;

	random_game_settings::update_gui_elements_position();
}

void random_game_settings::escape_down(ALLEGRO_EVENT* ev)
{
	event_handler::get_instance().change_state(game_state::MAIN_MENU);
}

void random_game_settings::key_char(ALLEGRO_EVENT* ev)
{
	for(int i=0; i<text_fields.size(); ++i)
		text_fields[i].key_char(ev);
}

void random_game_settings::mouse_axes(ALLEGRO_EVENT* ev)
{
	for(int i=0; i<sliders.size(); ++i)
		sliders[i].update_mouse_position(ev->mouse.x, ev->mouse.y);
	
	for(int i=0; i<buttons.size(); ++i)
		buttons[i].update_mouse_position(ev->mouse.x, ev->mouse.y);

	highlighted_option = compute_button_number(ev->mouse.x, ev->mouse.y);
}

void random_game_settings::mouse_left_down(ALLEGRO_EVENT* ev)
{
	for(int i=0; i<sliders.size(); ++i)
		sliders[i].mouse_down(ev->mouse.x, ev->mouse.y);

	for(int i=0; i<text_fields.size(); ++i)
		text_fields[i].mouse_down(ev->mouse.x, ev->mouse.y);

	int execute_option_number = compute_button_number(ev->mouse.x, ev->mouse.y);

	if(execute_option_number == 0)
		event_handler::get_instance().change_state(game_state::MAIN_MENU);

	else if(execute_option_number == 1)
		start_new_game();
}

void random_game_settings::mouse_up(ALLEGRO_EVENT* ev)
{
	for(int i=0; i<sliders.size(); ++i)
		sliders[i].mouse_up(ev->mouse.x, ev->mouse.y);
}

void random_game_settings::timer(ALLEGRO_EVENT* ev, int x, int y)
{
	update_gui_elements_position();
	draw();
}

void random_game_settings::draw()
{
	al_clear_to_color(BLACK_COLOR);
	al_draw_textf(font30, WRITING_COLOR, display_width/2, 50, ALLEGRO_ALIGN_CENTRE, "Game settings");

	for(int i=0; i<text_fields.size(); ++i)
		text_fields[i].draw();
	
	for(int i=0; i<sliders.size(); ++i)
		sliders[i].draw();

	for(int i=0; i<buttons.size(); ++i)
		buttons[i].draw();
	
	al_flip_display();
}

void random_game_settings::start_new_game()
{
	al_clear_to_color(BLACK_COLOR);
	al_draw_textf(font30, WRITING_COLOR, display_width/2, (display_height)/3, ALLEGRO_ALIGN_CENTRE, "Generating map");
	al_flip_display();			

	int starting_honour = std::stoi(text_fields[0].get_value());
	if((starting_honour < 0) || (starting_honour > 1000000))
		starting_honour = 1000000;

	std::vector<int> initial_resources(NUMBER_OF_RESOURCES, 0);
	std::vector<resources> resource_type{WOOD, STONE, MARBLE, BRICKS};
	for(int i=1; i<text_fields.size(); ++i)
	{
		int value = std::stoi(text_fields[i].get_value());
		if((value < 0) || (value > 1000000))
			value = 1000000;
		initial_resources[resource_type[i-1]] = value;
	}

	int amount_of_enemies = sliders[0].get_value();
	int amount_of_mountains = sliders[1].get_value();
	
	std::vector<int> natural_resources;
	
	for(int i=2; i<sliders.size(); ++i)
		natural_resources.push_back(sliders[i].get_value());
	

	if(session != nullptr)
		throw new std::exception;		//session wasnt deleted!!!

	session = new game_session(initial_resources, starting_honour, amount_of_enemies, natural_resources, amount_of_mountains);
	
	event_handler::get_instance().change_state(game_state::GAME);
}

int random_game_settings::compute_button_number(int x, int y)
{
	for(int i=0; i<buttons.size(); ++i)
	{
		if(buttons[i].mouse_on_button(x, y))
			return i;
	}
	return -1;
}

void random_game_settings::update_gui_elements_position()
{
	first_column = 100;
	second_column = std::max(first_column + 400, display_width/2);
	buttons_y = std::max(static_cast<int>((text_fields.size() + 2)*50 + header_height), display_height - 100);

	for(int i=0; i < text_fields.size(); ++i)
		text_fields[i].update_position(first_column, header_height + 50 * i);
	
	sliders[0].update_position(first_column, text_fields.size()*50 + header_height);		//enemies
	sliders[1].update_position(first_column, (text_fields.size() + 1)*50 + header_height);		//mountains
	
	buttons[0].update_position(first_column, buttons_y);
	
	for(int i=2; i < sliders.size(); ++i)
		sliders[i].update_position(second_column, (i - 2) * 50 + header_height);
	
	buttons[1].update_position(second_column, buttons_y);
}



