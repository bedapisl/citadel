
#include "core.h"

extern std::ofstream log_file;
extern game_session* session;
extern int display_width;
extern int display_height;

extern ALLEGRO_FONT* font20;
extern ALLEGRO_FONT* font25;
extern ALLEGRO_FONT* font30;

//for each derived class which is serialized
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

BOOST_CLASS_EXPORT(game_object)
BOOST_CLASS_EXPORT(tile)

BOOST_CLASS_EXPORT(carrier_output)

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
	loops.push_back(boost::shared_ptr<loop>(new end_of_game));

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
		throw std::exception();

	done = false;

	bool timer_event = false;

	while(!done)
	{
		al_wait_for_event(queue, ev);
		{
			switch(ev->type)
			{
				case(ALLEGRO_EVENT_KEY_DOWN):
				{
					LOG("KEY_DOWN");
					handle_key_down(ev);
					break;
				}
				case(ALLEGRO_EVENT_KEY_UP):
				{
					LOG("KEY_UP");
					current_loop->key_up(ev);
					break;
				}
				case(ALLEGRO_EVENT_KEY_CHAR):
				{
					LOG("KEY_CHAR");
					current_loop->key_char(ev);
					break;
				}
				case(ALLEGRO_EVENT_MOUSE_AXES):
				{
					LOG("MOUSE_AXES");
					current_loop->mouse_axes(ev);
					break;
				}
				case(ALLEGRO_EVENT_MOUSE_BUTTON_DOWN):
				{
					LOG("MOUSE_BUTTON_DOWN");
					if(ev->mouse.button == 1)
						current_loop->mouse_left_down(ev);
					else if(ev->mouse.button == 2)
						current_loop->mouse_right_down(ev);
				}
				break;
				case(ALLEGRO_EVENT_MOUSE_BUTTON_UP):
				{
					LOG("MOUSE_BUTTON_UP");
					if(ev->mouse.button == 1)
						current_loop->mouse_left_up(ev);
					else if(ev->mouse.button == 2)
						current_loop->mouse_right_up(ev);
				}
				break;
				case(ALLEGRO_EVENT_TIMER):
				{
					LOG("TIMER");
					timer_event = true;		//this will happen after everything else is done
					break;
				}
				case(ALLEGRO_EVENT_DISPLAY_RESIZE):
				{
					LOG("RESIZE_DISPLAY");
					resize_window(ev);
					break;
				}
				case(ALLEGRO_EVENT_DISPLAY_CLOSE):
				{
					LOG("DISPLAY_CLOSE");
					display_close();
					break;
				}
				default:
					{}
			}
			
			if(timer_event && al_is_event_queue_empty(queue))
			{
				LOG("DRAW");
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
	rotation r;
	mouse->left_button_go_down(screen_position_x, screen_position_y, &r);
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
	mouse->move(screen_position_x, screen_position_y);
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

	if(((ev->mouse.x > display_width - 3*BUTTON_SIZE) && (ev->mouse.x < display_width - 2*BUTTON_SIZE)) &&
		((ev->mouse.y > display_height - BUTTON_SIZE) && (ev->mouse.y < display_height - BUTTON_SIZE/2)))
	{
		event_handler::get_instance().change_state(game_state::INGAME_MENU);
	}
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
#ifdef UNIX		//we are using unix time library which is not on other platforms
	gettimeofday(&time, NULL);
	starting_time = time.tv_usec + time.tv_sec*1000000;
#endif
	number_of_frames++;
	LOG("EVENT_TIMER - frame number:" << number_of_frames);

	screen_move(mouse->state, keyboard_state, screen_position_x, screen_position_y);
	mouse->move(screen_position_x, screen_position_y);	//if screen moves, mouse "pseudo" moves too
	
	draw();
#ifdef UNIX
	gettimeofday(&time, NULL);
	drawing_time = time.tv_usec + time.tv_sec*1000000;
#endif
	LOG("updating people");

	for(int i = 0; i<session->people_list.size(); i++)
	{
		session->people_list[i]->update();
	}
#ifdef UNIX
	gettimeofday(&time, NULL);
	people_time = time.tv_usec + time.tv_sec*1000000;
#endif

	LOG("updating buildings");

	building::assign_workers();
	for(int i=0; i<session->building_list.size(); i++)
	{
		session->building_list[i]->update();
	}
#ifdef UNIX
	gettimeofday(&time, NULL);
	buildings_time = time.tv_usec + time.tv_sec*1000000;
#endif
	LOG("updating missiles");

	for(int i = 0; i<session->missile_list.size(); i++)
	{
		session->missile_list[i]->draw_and_update(screen_position_x, screen_position_y);
	}
	
	update_minimap(minimap);
	
	bool done = false;
	session->update(mouse, done);
	if(done)
	{
		LOG("game lost");
		event_handler::get_instance().change_state(game_state::END_OF_GAME);
		return;
	}

#ifdef UNIX
	gettimeofday(&time, NULL);
	session_time = time.tv_usec + time.tv_sec*1000000;
#endif
	delete_death(mouse);
#ifdef UNIX
	gettimeofday(&time, NULL);
	rest = time.tv_usec + time.tv_sec*1000000;

	LOG("drawing: " << drawing_time - starting_time << " updating people: " << people_time - drawing_time << 
			" updating buildings: " << buildings_time - people_time << " updating session " << session_time - buildings_time << 
			" rest: " << rest - session_time);
#endif
	LOG("end of frame");
}

void game_loop::start()
{
	if(minimap != nullptr)
		al_destroy_bitmap(minimap);
	minimap = create_minimap();
}

void game_loop::end()
{
	al_destroy_bitmap(minimap);
	minimap = nullptr;
}

void game_loop::draw()
{
	draw_map(screen_position_x, screen_position_y, mouse);
	window::draw_active_windows();
	draw_main_panel(minimap, screen_position_x, screen_position_y, mouse, session);
	
	al_flip_display();
}

void menu_loop::enter_down(ALLEGRO_EVENT* ev)
{
	for(int i=0; i<blocks.size(); ++i)
		blocks[i].enter_down(ev);
	
	check_clicked_buttons();
}

void menu_loop::up_arrow_down(ALLEGRO_EVENT* ev)
{
	bool founded_active = false;
	for(int i=0; i<blocks.size(); ++i)
	{
		if(blocks[i].mouse_on_block)
		{
			blocks[i].up_arrow_down(ev);
			founded_active = true;
			break;
		}
	}
	if(!founded_active)
		blocks[0].up_arrow_down(ev);
}
	
void menu_loop::down_arrow_down(ALLEGRO_EVENT* ev)
{	
	bool founded_active = false;
	for(int i=0; i<blocks.size(); ++i)
	{
		if(blocks[i].mouse_on_block)
		{
			blocks[i].down_arrow_down(ev);
			founded_active = true;
			break;
		}
	}
	if(!founded_active)
		blocks[0].down_arrow_down(ev);
}

void menu_loop::key_char(ALLEGRO_EVENT* ev)
{
	for(int i=0; i<blocks.size(); ++i)
		blocks[i].key_char(ev);
}

void menu_loop::mouse_axes(ALLEGRO_EVENT* ev)
{
	for(int i=0; i<blocks.size(); ++i)
		blocks[i].mouse_axes(ev);
}

void menu_loop::mouse_left_down(ALLEGRO_EVENT* ev)
{
	for(int i=0; i<blocks.size(); ++i)
		blocks[i].mouse_down(ev);

	check_clicked_buttons();
}

void menu_loop::mouse_left_up(ALLEGRO_EVENT* ev)
{
	for(int i=0; i<blocks.size(); ++i)
		blocks[i].mouse_up(ev);
}

void menu_loop::timer(ALLEGRO_EVENT* ev, int x, int y)
{
	update_gui_blocks_position();
	draw();
}

ingame_menu::ingame_menu()
{
	blocks.push_back(gui_block(50, 50));
	for(int i=0; i<options_names.size(); ++i)
	{
		buttons.push_back(boost::shared_ptr<menu_button>(new menu_button(options_names[i], true)));
		blocks[0].add_gui_element(buttons[i]);
	}
	update_gui_blocks_position();
}


void ingame_menu::escape_down(ALLEGRO_EVENT* ev)
{
	event_handler::get_instance().change_state(game_state::GAME);
}

void ingame_menu::start()
{
	game_bitmap = al_clone_bitmap(al_get_backbuffer(game_info::display));
}

void ingame_menu::end()
{
	al_destroy_bitmap(game_bitmap);
}

void ingame_menu::draw()
{
	al_draw_bitmap(game_bitmap, 0, 0, 0);		//draws map and panel

	int black_vertical_border = 0;
	int black_horizontal_border = 0;
	al_draw_filled_rectangle(blocks[0].x - black_horizontal_border, blocks[0].y - black_vertical_border, blocks[0].x + blocks[0].length + black_horizontal_border, blocks[0].y + blocks[0].height + black_vertical_border, BLACK_COLOR);

	for(int i=0; i<blocks.size(); ++i)
		blocks[i].draw();

	al_flip_display();
}

void ingame_menu::update_gui_blocks_position()
{
	blocks[0].update_position((display_width - blocks[0].length)/2, (display_height - blocks[0].height)/2);
}

void ingame_menu::check_clicked_buttons()
{
	int chosen_option = -1;
	for(int i=0; i<buttons.size(); ++i)
	{
		if(buttons[i]->clicked())
			chosen_option = i;
	}
	
	switch(chosen_option)
	{
		case(0):
			event_handler::get_instance().change_state(game_state::GAME);
			break;
		case(1):
			event_handler::get_instance().change_state(game_state::SAVE_MENU);
			break;
		case(2):
		{
			delete session;
			session = nullptr;
			event_handler::get_instance().change_state(game_state::MAIN_MENU);
		}
			break;
		default:
		 	{ };
	}
}

save_menu::save_menu() : file_name(new text_field("File name", "", false, 30))
{
	for(int i=0; i<5; ++i)
		blocks.push_back(gui_block());

	std::vector<std::string> file_names = find_save_files(); 

	blocks[0].add_gui_element(file_name);
	
	for(int i=0; i<file_names.size(); ++i)
	{
		file_buttons.push_back(boost::shared_ptr<menu_button>(new menu_button(file_names[i], false, 25)));
		blocks[0].add_gui_element(file_buttons[i]);
	}

	for(int i=0; i<option_names.size(); ++i)
	{
		real_buttons.push_back(boost::shared_ptr<menu_button>(new menu_button(option_names[i], false, 25)));
		blocks[i + 1].add_gui_element(real_buttons.back());
	}

	update_gui_blocks_position();
}

void save_menu::escape_down(ALLEGRO_EVENT* ev)
{
	if(session == nullptr)
		event_handler::get_instance().change_state(game_state::MAIN_MENU);
	
	else
		event_handler::get_instance().change_state(game_state::GAME);
}

void save_menu::start()
{

}

void save_menu::draw()
{
	al_clear_to_color(BLACK_COLOR);
	for(int i=0; i<blocks.size(); ++i)
		blocks[i].draw();

	al_flip_display();
}

void save_menu::update_gui_blocks_position()
{
	blocks[0].update_position(100, header_height);
	blocks[0].set_height(std::max(300, display_height - 250));

	int current_x = 100;
	for(int i=1; i<blocks.size(); ++i)
	{
		blocks[i].update_position(current_x, std::max(header_height + 50 + blocks[0].height, display_height - 50 - blocks[i].height));
		current_x += blocks[i].length + 50;
	}
}

void save_menu::check_clicked_buttons()
{
	for(int i=0; i<file_buttons.size(); ++i)
	{
		if(file_buttons[i]->clicked())
			file_name->set_value(file_buttons[i]->show_name());
	}

	for(int i=0; i<real_buttons.size(); ++i)
	{
		if(real_buttons[i]->clicked())
			execute_save_window_option(i);
	}
}

void save_menu::execute_save_window_option(int option)
{
	switch(option)		
	{
		case(0):		//load
		{
			std::ifstream file_to_load;

			bool exist = false;
			for(int i=0; i<file_buttons.size(); ++i)
			{
				if(file_buttons[i]->show_name() == file_name->get_value())
					exist = true;
			}
			if(!exist)
				return;

			file_to_load.open("data/saves/" + file_name->get_value());
			if(!file_to_load.good())
			{
				LOG("cannot load: " << "data/saves/" + file_name->get_value());
				throw std::exception();
				return;
			}
			//boost::archive::text_iarchive archive(file_to_load);
			boost::archive::xml_iarchive archive(file_to_load);
			archive >> boost::serialization::make_nvp("session", session);

			session->finish_serialization();	
			event_handler::get_instance().change_state(game_state::GAME);
		}
		break;
		case(1):		//save
		{
			if(file_name->get_value().empty())
				return;

			if(session == nullptr)
				return;

			std::ofstream file_to_save("data/saves/" + file_name->get_value(), std::ofstream::out);
			if(file_to_save.bad())
				return;
			
			for(int i=0; i<session->tile_list.size(); ++i)
			{
				for(int j=0; j<session->tile_list[i].size(); ++j)
					session->tile_list[i][j]->prepare_serialization();
			}

			//boost::archive::text_oarchive archive(file_to_save);
			boost::archive::xml_oarchive archive(file_to_save);

			archive << boost::serialization::make_nvp("session", session);

			bool founded = false;
			for(int i=0; i<file_buttons.size(); ++i)
			{
				if(file_name->get_value() == file_buttons[i]->show_name())
					founded = true;
			}
			if(!founded)
			{
				file_buttons.push_back(boost::shared_ptr<menu_button>(new menu_button(file_name->get_value(), false, 25)));
				blocks[0].add_gui_element(file_buttons.back());
			}
		}
		break;
		case(2):		//delete
		{
			std::remove(("data/saves/" + file_name->get_value()).c_str());
			for(int i=0; i<file_buttons.size(); ++i)
			{
				if(file_buttons[i]->show_name() == file_name->get_value())
					file_buttons.erase(file_buttons.begin() + i);
			}
			blocks[0].remove_invalid_elements();
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

main_menu::main_menu() 
{ 
	blocks.push_back(gui_block());
	std::vector<std::string> options_names{"Random game", "Load game", "Settings", "Quit"};
	for(int i=0; i<options_names.size(); ++i)
	{
		buttons.push_back(boost::shared_ptr<menu_button>(new menu_button(options_names[i], true, 25, 200, 50)));
		blocks[0].add_gui_element(buttons.back());
	}

	update_gui_blocks_position();
}
 
void main_menu::escape_down(ALLEGRO_EVENT* ev)
{
	event_handler::get_instance().quit();
}

void main_menu::draw()
{
	al_clear_to_color(al_map_rgb(0,0,0));
	blocks[0].draw();
	al_flip_display();
}

void main_menu::update_gui_blocks_position()
{
	int y = std::max(100, (display_height - blocks[0].height) / 2);
	blocks[0].update_position(display_width/2 - blocks[0].length/2, y); 
}

void main_menu::check_clicked_buttons()
{
	int chosen_option = -1;
	for(int i=0; i<buttons.size(); ++i)
	{
		if(buttons[i]->clicked())
		{
			chosen_option = i;
			break;
		}
	}
	
	switch(chosen_option)
	{
		case(0):	//Random game
			event_handler::get_instance().change_state(game_state::RANDOM_GAME_SETTINGS);
			break;
		case(1):	//Load game
			event_handler::get_instance().change_state(game_state::SAVE_MENU);
			break;
		case(2):	//Settigns
			event_handler::get_instance().change_state(game_state::SETTINGS);
			break;
		case(3):	//Quit
			event_handler::get_instance().quit();
			break;
		default:
			{ }
	}
}
	
settings_menu::settings_menu()
{
	for(int i=0; i<2; ++i)
		blocks.push_back(gui_block());

	buttons.push_back(boost::shared_ptr<switch_button>(new switch_button("Fullscreeen", game_info::fullscreen)));
	buttons.push_back(boost::shared_ptr<switch_button>(new switch_button("Music", game_info::music)));
	
	text_fields.push_back(boost::shared_ptr<text_field>(new text_field("Display width", std::to_string(display_width), true, 6)));
	text_fields.push_back(boost::shared_ptr<text_field>(new text_field("Display height", std::to_string(display_height), true, 6)));
	text_fields.push_back(boost::shared_ptr<text_field>(new text_field("FPS", std::to_string(game_info::fps), true, 6)));
	
	for(int i=0; i<buttons.size(); ++i)
		blocks[0].add_gui_element(buttons[i]);
	
	for(int i=0; i<text_fields.size(); ++i)
		blocks[0].add_gui_element(text_fields[i]);

	done_button = boost::shared_ptr<menu_button>(new menu_button("Done", true, 25));
	blocks[1].add_gui_element(done_button);

	update_gui_blocks_position();
}

void settings_menu::escape_down(ALLEGRO_EVENT* ev)
{
	event_handler::get_instance().change_state(game_state::MAIN_MENU);
}

void settings_menu::draw()
{
	al_clear_to_color(BLACK_COLOR);

	al_draw_text(font30, WRITING_COLOR, display_width/2, header_height / 2, ALLEGRO_ALIGN_CENTRE, "Settings");
	
	for(int i=0; i<blocks.size(); ++i)
		blocks[i].draw();

	al_flip_display();
}

void settings_menu::update_gui_blocks_position()
{
	blocks[0].update_position(100, header_height);
	blocks[1].update_position(display_width - 50 - blocks[1].length, std::max(display_height - 100, blocks[0].height + 50));
}

void settings_menu::check_clicked_buttons()
{
	if(done_button->clicked())
		event_handler::get_instance().change_state(game_state::MAIN_MENU);

	if(buttons[1]->get_value())
		music::get_instance().play_background_music();
	else
		music::get_instance().stop_background_music();
}

void settings_menu::start()
{
}

void settings_menu::end()
{
	std::ofstream file;
	file.open("data/config.txt", std::ios::out);

	file << "fullscreen = ";
	if(buttons[0]->get_value())
		file << "true\n";

	else
		file << "false\n";

	file << "display_width = " << text_fields[0]->get_value() << std::endl << "display_height = " << text_fields[1]->get_value() << std::endl << "fps = " << text_fields[2]->get_value() << std::endl << "music = ";

	if(buttons[1]->get_value())
		file << "true" << std::endl;	
	else
		file << "false" << std::endl;

	file.close();
}
	
random_game_settings::random_game_settings()
{
	for(int i=0; i<4; ++i)
		blocks.push_back(gui_block());

	text_fields.push_back(boost::shared_ptr<text_field>(new text_field("Honour", "0", true, 6)));
	for(int i=0; i<resources_names.size(); ++i)
		text_fields.push_back(boost::shared_ptr<text_field>(new text_field(resources_names[i], resources_initial_values[i], true, 6)));

	for(int i=0; i<text_fields.size(); ++i)
		blocks[0].add_gui_element(text_fields[i]);

	sliders.push_back(boost::shared_ptr<slider>(new slider("Enemies", 1, 2)));
	sliders.push_back(boost::shared_ptr<slider>(new slider("Mountains", 1, 2)));

	blocks[0].add_gui_element(sliders[0]);
	blocks[0].add_gui_element(sliders[1]);
	
	for(int i=0; i<natural_resources_names.size(); ++i)
		sliders.push_back(boost::shared_ptr<slider>(new slider(natural_resources_names[i], 1, 2)));

	for(int i=2; i<sliders.size(); ++i)
		blocks[1].add_gui_element(sliders[i]);
	
	buttons.push_back(boost::shared_ptr<menu_button>(new menu_button("Back", true, 25)));
	buttons.push_back(boost::shared_ptr<menu_button>(new menu_button("Start game", true, 25)));

	blocks[2].add_gui_element(buttons[0]);
	blocks[3].add_gui_element(buttons[1]);

	random_game_settings::update_gui_blocks_position();
}

void random_game_settings::escape_down(ALLEGRO_EVENT* ev)
{
	event_handler::get_instance().change_state(game_state::MAIN_MENU);
}

void random_game_settings::draw()
{
	al_clear_to_color(BLACK_COLOR);
	al_draw_textf(font30, WRITING_COLOR, display_width/2, 50, ALLEGRO_ALIGN_CENTRE, "Game settings");

	for(int i=0; i<blocks.size(); ++i)
		blocks[i].draw();

	al_flip_display();
}

void random_game_settings::update_gui_blocks_position()
{
	int min_x_distance = 50;
	int min_y_distance = 50;
	int first_column = 100;
	int second_column = std::max(first_column + blocks[0].length + min_x_distance, display_width/2);
	int y_of_buttons = std::max(blocks[0].height + header_height + min_y_distance, display_height - 100);
	blocks[0].update_position(first_column, header_height);
	blocks[1].update_position(second_column, header_height);
	blocks[2].update_position(first_column, y_of_buttons);
	blocks[3].update_position(second_column, y_of_buttons);
}

void random_game_settings::check_clicked_buttons()
{
	if(buttons[0]->clicked())
		event_handler::get_instance().change_state(game_state::MAIN_MENU);

	else if(buttons[1]->clicked())
		start_new_game();
}

void random_game_settings::start_new_game()
{
	al_clear_to_color(BLACK_COLOR);
	al_draw_textf(font30, WRITING_COLOR, display_width/2, (display_height)/3, ALLEGRO_ALIGN_CENTRE, "Generating map");
	al_flip_display();			

	int starting_honour = std::stoi(text_fields[0]->get_value());
	if((starting_honour < 0) || (starting_honour > 1000000))
		starting_honour = 1000000;

	std::vector<int> initial_resources(NUMBER_OF_RESOURCES, 0);
	std::vector<resources> resource_type{WOOD, STONE, MARBLE, BRICKS};
	for(int i=1; i<text_fields.size(); ++i)
	{
		int value = std::stoi(text_fields[i]->get_value());
		if((value < 0) || (value > 1000000))
			value = 1000000;
		initial_resources[resource_type[i-1]] = value;
	}

	int amount_of_enemies = sliders[0]->get_value();
	int amount_of_mountains = sliders[1]->get_value();
	
	std::vector<int> natural_resources;
	
	for(int i=2; i<sliders.size(); ++i)
		natural_resources.push_back(sliders[i]->get_value());
	
	if(session != nullptr)
		throw std::exception();		//session wasnt deleted!!!

	session = new game_session(initial_resources, starting_honour, amount_of_enemies, natural_resources, amount_of_mountains);
	
	event_handler::get_instance().change_state(game_state::GAME);
}

end_of_game::end_of_game() : done_button(new menu_button("Done", true))
{	
}


void end_of_game::escape_down(ALLEGRO_EVENT* ev)
{
	event_handler::get_instance().change_state(game_state::MAIN_MENU);
}

void end_of_game::start()
{	
	blocks.clear();
	blocks.push_back(gui_block(50, 50));

	std::vector<std::string> text;
	text.push_back(std::string("End of game - your warehouse was lost."));
	text.push_back(std::string("You survived " + std::to_string(session->show_invasion_number()) + " invasions."));

	message = boost::shared_ptr<text_element>(new text_element(text));		//constructing new message
	blocks[0].add_gui_element(message);
	blocks[0].add_gui_element(done_button);
	update_gui_blocks_position();
	
	game_bitmap = al_clone_bitmap(al_get_backbuffer(game_info::display));
}

void end_of_game::end()
{
	blocks.clear();
	al_destroy_bitmap(game_bitmap);
}

void end_of_game::draw()
{
	al_draw_bitmap(game_bitmap, 0, 0, 0);		//draws map and panel

	int black_vertical_border = 0;
	int black_horizontal_border = 0;
	al_draw_filled_rectangle(blocks[0].x - black_horizontal_border, blocks[0].y - black_vertical_border, blocks[0].x + blocks[0].length + black_horizontal_border, blocks[0].y + blocks[0].height + black_vertical_border, BLACK_COLOR);

	for(int i=0; i<blocks.size(); ++i)
		blocks[i].draw();

	al_flip_display();
}

void end_of_game::update_gui_blocks_position()
{
	blocks[0].update_position((display_width - blocks[0].length)/2, (display_height - blocks[0].height)/2);
}

void end_of_game::check_clicked_buttons()
{
	if(done_button->clicked())
	{
		delete session;
		session = nullptr;
		event_handler::get_instance().change_state(game_state::MAIN_MENU);
	}
}

