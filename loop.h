
/**
 * \file loop.h
 * \brief Event handling and loops for all states of game.
 */

#ifndef loop_h_bedas_guard
#define loop_h_bedas_guard

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

/**
 * \brief Manages run of whole program.
 */

class event_handler
{
public:
	virtual ~event_handler();
	static event_handler& get_instance();

	void run();			///< Main function. Starts executing main_menu_loop.
	void quit() {done = true;}	///< Halts the program.
	void change_state(game_state state);	///< Go to another state (for example from "main menu" to "load/save menu).

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

/**
 * \brief General loop interface. All loops are managed by event_handler which executes them by calling their functions when event comes.
 */
class loop
{
public:
	loop() {}
	virtual ~loop() {}
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
	virtual void start() {}		///< Called before the loop will start executing.
	virtual void end() {}		///< Called after the loop finishes.

private:

};

/**
 * \brief This loop controls the regular game, when player is not in menus. 
 */
class game_loop : public loop
{
public:
	game_loop();
	virtual ~game_loop();
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
	void end();
	void draw();

private:
	game_mouse* mouse;
	ALLEGRO_BITMAP* minimap;
	ALLEGRO_KEYBOARD_STATE* keyboard_state;
	int screen_position_x;
	int screen_position_y;
	int number_of_frames;

#ifdef UNIX
	timeval time;
	long long int starting_time, drawing_time, people_time, buildings_time, session_time, rest;
#endif
};
/**
 * \brief Base class for all menu loops.
 */
class menu_loop : public loop
{
public:
	menu_loop() { }
	virtual void escape_down(ALLEGRO_EVENT* ev) {}
	virtual void enter_down(ALLEGRO_EVENT* ev);
	virtual void up_arrow_down(ALLEGRO_EVENT* ev);
	virtual void down_arrow_down(ALLEGRO_EVENT* ev);
	virtual void left_arrow_down(ALLEGRO_EVENT* ev) {}
	virtual void right_arrow_down(ALLEGRO_EVENT* ev) {}
	virtual void other_key_down(ALLEGRO_EVENT* ev) {}
	virtual void key_up(ALLEGRO_EVENT* ev) {}
	virtual void key_char(ALLEGRO_EVENT* ev);
	virtual void mouse_axes(ALLEGRO_EVENT* ev);
	virtual void mouse_left_down(ALLEGRO_EVENT* ev);
	virtual void mouse_left_up(ALLEGRO_EVENT* ev);
	virtual void mouse_right_down(ALLEGRO_EVENT* ev) {}
	virtual void mouse_right_up(ALLEGRO_EVENT* ev) {}
	virtual void timer(ALLEGRO_EVENT* ev, int mouse_x, int mouse_y);
	virtual void start() {}
	virtual void end() {}
	
protected:
	virtual void draw() = 0;
	virtual void update_gui_blocks_position() = 0;
	virtual void check_clicked_buttons() = 0;

	std::vector<gui_block> blocks;
};
/**
 * \brief Represents menu when game is only paused.
 */
class ingame_menu : public menu_loop
{
public:
	ingame_menu();
	void escape_down(ALLEGRO_EVENT* ev); 
	void start();
	void end();

private:
	void draw();
	void update_gui_blocks_position();
	void check_clicked_buttons();

	std::vector<boost::shared_ptr<menu_button>> buttons;
	const std::vector<std::string> options_names = std::vector<std::string>{"Continue", "Save", "Exit"};

	ALLEGRO_BITMAP* game_bitmap;
};

/**
 * \brief Load/save menu.
 */
class save_menu : public menu_loop
{
public:
	save_menu();
	void escape_down(ALLEGRO_EVENT* ev);
	void start();

protected:
	void draw();
	void update_gui_blocks_position();
	void check_clicked_buttons();
	void execute_save_window_option(int option);
	std::vector<std::string> find_save_files();

	static const int header_height = 100;
	const std::vector<std::string> option_names = std::vector<std::string>{"Load", "Save", "Delete", "Return"};
	
	std::vector<boost::shared_ptr<menu_button>> real_buttons;
	std::vector<boost::shared_ptr<menu_button>> file_buttons;
	boost::shared_ptr<text_field> file_name;
};

/**
 * \brief Manages main menu.
 */
class main_menu : public menu_loop
{
public:	
	main_menu();
	void escape_down(ALLEGRO_EVENT* ev); 

private:
	void draw();
	void update_gui_blocks_position();
	void check_clicked_buttons();
	std::vector<boost::shared_ptr<menu_button>> buttons;
	
//	const int header_height = 100;
//	const int element_height = 90;
};

/**
 * \brief Manages screen with general settings.
 */
class settings_menu : public menu_loop
{
public:
	settings_menu();
	void escape_down(ALLEGRO_EVENT* ev);
	void start();
	void end();

private:
	void draw();	
	void update_gui_blocks_position();
	void check_clicked_buttons();
	
	const int header_height = 100;

	std::vector<boost::shared_ptr<switch_button>> buttons;
	std::vector<boost::shared_ptr<text_field>> text_fields;	
	boost::shared_ptr<menu_button> done_button;
};

class slider;

/**
 * \brief Manages screen before start of random game with settings about it.
 */
class random_game_settings : public menu_loop
{
public:
	random_game_settings();
	void escape_down(ALLEGRO_EVENT* ev);

private:
	const int header_height = 100;

	void draw();
	void update_gui_blocks_position();
	void check_clicked_buttons();
	void start_new_game();
	const std::vector<std::string> resources_names = std::vector<std::string>{"Wood", "Stone", "Marble", "Bricks"};
	const std::vector<std::string> resources_initial_values = std::vector<std::string>{"200", "100", "10", "10"};
	const std::vector<std::string> natural_resources_names = std::vector<std::string>{"Water", "Wood", "Fertile soil", "Iron", "Coal", "Marble", "Gold"};
	std::vector<boost::shared_ptr<text_field>> text_fields;
	std::vector<boost::shared_ptr<slider>> sliders;
	std::vector<boost::shared_ptr<menu_button>> buttons;
	std::vector<std::string> button_names = std::vector<std::string>{"Back", "Start game"};
};

/**
 * \brief Manages window which will be visible after player loses the game.
 */
class end_of_game : public menu_loop
{
public:
	end_of_game();
	void escape_down(ALLEGRO_EVENT* ev);
	void start();
	void end();

private:
	void draw();
	void update_gui_blocks_position();
	void check_clicked_buttons();
	void start_new_game();
	
	boost::shared_ptr<text_element> message;
	boost::shared_ptr<menu_button> done_button;
	ALLEGRO_BITMAP* game_bitmap;
};

#endif



