#ifndef loop_h
#define loop_h

#include <boost/serialization/base_object.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/assume_abstract.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/weak_ptr.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/export.hpp>
#include <boost/filesystem.hpp>

class game_session;
class loop;
class game_info;
class game_mouse;

class event_handler
{
public:
	virtual ~event_handler();
	static event_handler& get_instance();

	void run();
	void quit() {done = true;}
	void change_state(game_state state);

private:
	event_handler();
	
	void handle_key_down(ALLEGRO_EVENT* ev);
	void resize_window(ALLEGRO_EVENT* ev);
	void display_close();

	ALLEGRO_EVENT_QUEUE* queue;
	ALLEGRO_EVENT* ev;
	ALLEGRO_EVENT* next_event;
	ALLEGRO_TIMER* timer;
	ALLEGRO_MOUSE_STATE* mouse_state;
	
	loop* current_loop;
	std::vector<boost::shared_ptr<loop>> loops;
	bool done;
};

class loop
{
public:
	virtual void escape_down(ALLEGRO_EVENT* ev) {}
	virtual void enter_down(ALLEGRO_EVENT* ev) {}
	virtual void up_arrow_down(ALLEGRO_EVENT* ev) {}
	virtual void down_arrow_down(ALLEGRO_EVENT* ev) {}
	virtual void left_arrow_down(ALLEGRO_EVENT* ev) {}
	virtual void right_arrow_down(ALLEGRO_EVENT* ev) {}
	virtual void other_key_down(ALLEGRO_EVENT* ev) {}
	virtual void key_up(ALLEGRO_EVENT* ev) {}
	virtual void key_char(ALLEGRO_EVENT* ev) {}
	virtual void mouse_axes(ALLEGRO_EVENT* ev) {}
	virtual void mouse_left_down(ALLEGRO_EVENT* ev) {}
	virtual void mouse_left_up(ALLEGRO_EVENT* ev) {}
	virtual void mouse_right_down(ALLEGRO_EVENT* ev) {}
	virtual void mouse_right_up(ALLEGRO_EVENT* ev) {}
	virtual void timer(ALLEGRO_EVENT* ev, int mouse_x, int mouse_y) {}
	virtual void start() {}
	virtual void end() {}

private:

};

class game_loop : public loop
{
public:
	game_loop();
	~game_loop();
	void escape_down(ALLEGRO_EVENT* ev); 
	void enter_down(ALLEGRO_EVENT* ev);
	void other_key_down(ALLEGRO_EVENT* ev);
	void mouse_axes(ALLEGRO_EVENT* ev);
	void mouse_left_down(ALLEGRO_EVENT* ev);
	void mouse_left_up(ALLEGRO_EVENT* ev);
	void mouse_right_down(ALLEGRO_EVENT* ev);
	void mouse_right_up(ALLEGRO_EVENT* ev);
	void timer(ALLEGRO_EVENT* ev, int mouse_x, int mouse_y);
	void start();
	void draw();

private:
	game_mouse* mouse;
	ALLEGRO_BITMAP* minimap;
	ALLEGRO_KEYBOARD_STATE* keyboard_state;
	int screen_position_x;
	int screen_position_y;
	int number_of_frames;

	timeval time;
	long long int starting_time, drawing_time, people_time, buildings_time, session_time, rest;
};

class ingame_menu : public loop
{
public:
	ingame_menu();
	void escape_down(ALLEGRO_EVENT* ev); 
	void enter_down(ALLEGRO_EVENT* ev);
	void up_arrow_down(ALLEGRO_EVENT* ev);
	void down_arrow_down(ALLEGRO_EVENT* ev);
	void mouse_axes(ALLEGRO_EVENT* ev);
	void mouse_left_down(ALLEGRO_EVENT* ev);
	void timer(ALLEGRO_EVENT* ev, int mouse_x, int mouse_y);
	void start();
	void end();

private:
	const int menu_width = 400;
	const int menu_start_y = 100;
	const int button_start_x = 20;
	const int button_start_y = 20;
	const int button_distance = 100;
	const int button_heigth = 60;
	const std::vector<std::string> options_names{"Continue", "Save", "Exit"};

	ALLEGRO_BITMAP* game_bitmap;

	void execute_option(ingame_menu_options option);
	void draw_ingame_menu();
	ingame_menu_options compute_ingame_menu_options(int x, int y);

	ingame_menu_options chosen_option;
};

class save_menu : public loop
{
public:
	save_menu();
	void escape_down(ALLEGRO_EVENT* ev);
	void enter_down(ALLEGRO_EVENT* ev);
	void up_arrow_down(ALLEGRO_EVENT* ev);
	void down_arrow_down(ALLEGRO_EVENT* ev);
	//void left_arrow_down(ALLEGRO_EVENT* ev);
	//void right_arrow_down(ALLEGRO_EVENT* ev);
	void key_char(ALLEGRO_EVENT* ev);
	void mouse_axes(ALLEGRO_EVENT* ev);
	void mouse_left_down(ALLEGRO_EVENT* ev);
	void timer(ALLEGRO_EVENT* ev, int mouse_x, int mouse_y);
	void start();

private:
	void draw_save_window();
	int compute_chosen_file(int x, int y);
	int compute_highlighted_option(int x, int y);
	bool click_to_writing_name(int x, int y);
	void execute_save_window_option(int option);
	std::vector<std::string> find_save_files();

	const int start_x = 100;		//start of window with save files
	const int start_y = 100;
	const int height = 30;
	const int space_after_end_y = 200;
	const int option_distance = 150;
	const int option_size = 100;
	const std::vector<std::string> option_names{"Load", "Save", "Delete", "Return"};

	std::vector<std::string> save_files;
	std::string current_name;
	bool writing_name;
	int chosen_file;
	int first_file_index;
	int highlighted_option;
};

class main_menu : public loop
{
public:	
	main_menu();
	void escape_down(ALLEGRO_EVENT* ev); 
	void enter_down(ALLEGRO_EVENT* ev);
	void up_arrow_down(ALLEGRO_EVENT* ev);
	void down_arrow_down(ALLEGRO_EVENT* ev);
	void mouse_axes(ALLEGRO_EVENT* ev);
	void mouse_left_down(ALLEGRO_EVENT* ev);
	void timer(ALLEGRO_EVENT* ev, int mouse_x, int mouse_y);
	//void start();

private:
	void execute_option(main_menu_option option);
	main_menu_option compute_main_menu_option(int x, int y);
	void draw_main_menu();

	main_menu_option chosen_option;
};

class settings_menu : public loop
{
public:
	settings_menu();
	void escape_down(ALLEGRO_EVENT* ev);
	void enter_down(ALLEGRO_EVENT* ev);
	void up_arrow_down(ALLEGRO_EVENT* ev);
	void down_arrow_down(ALLEGRO_EVENT* ev);
	void other_key_down(ALLEGRO_EVENT* ev);
	void mouse_left_down(ALLEGRO_EVENT* ev);
	void timer(ALLEGRO_EVENT* ev, int x, int y);
	void start();
	void end();

private:
	void draw_settings();

	bool fullscreen_option;
	bool music_option;
	int display_width_option;
	int display_height_option;
	int fps_option;

	int number_of_options;
	int option_focus;
	std::string text;
};

class slider;

class random_game_settings : public loop
{
public:
	random_game_settings();
	void escape_down(ALLEGRO_EVENT* ev);
	//void enter_down(ALLEGRO_EVENT* ev);
	//void up_arrow_down(ALLEGRO_EVENT* ev);
	//void down_arrow_down(ALLEGRO_EVENT* ev);
	void key_char(ALLEGRO_EVENT* ev);
	void mouse_axes(ALLEGRO_EVENT* ev);
	void mouse_left_down(ALLEGRO_EVENT* ev);
	void mouse_up(ALLEGRO_EVENT* ev);
	void timer(ALLEGRO_EVENT* ev, int mouse_x, int mouse_y);

private:
	const int first_collumn = 100;
	const int second_collumn = display_width/2;
	const int header_height = 100;

	void draw();
	void start_new_game();
	int compute_button_number(int x, int y);
	//text_field honour_field;
	const std::vector<std::string> resources_names{"Wood", "Stone", "Marble", "Bricks"};
	const std::vector<std::string> resources_initial_values{"200", "100", "10", "10"};
	const std::vector<std::string> natural_resources_names{"Water", "Wood", "Fertile soil", "Iron", "Coal", "Marble", "Gold"};
	std::vector<text_field> text_fields;
	std::vector<slider> sliders;
	std::vector<std::string> button_names{"Back", "Start game"};
	int highlighted_option;
};


#endif



