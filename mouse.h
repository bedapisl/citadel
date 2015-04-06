#ifndef mouse_h_bedas_guard
#define mouse_h_bedas_guard

#include "core.h"
#include "others.h"

/**
 * \brief Represents mouse.
 */
class game_mouse
{
public:
	void left_button_go_down(int &screen_position_x, int &screen_position_y, rotation* rotate); 	///< Handles pressing of left mouse button.
	void right_button_go_down();									///< Handles pressing of right mouse button.
	void left_button_go_up(int screen_position_x, int screen_position_y);				///< Handles release of left mouse button.
	void right_button_go_up();					///< Handles release of right mouse buttton.
	int move(int & screen_position_x, int & screen_position_y); 	///< Handles movement of mouse.
	int draw_mouse(int screen_position_x, int screen_position_y);	///< Higlights selected objects and objects under cursor.
	int show_tile_x() {return tile_x;}				///< Returns x coordinate of tile under mouse cursor.
	int show_tile_y() {return tile_y;}				///< Returns y coordinate of tile under mouse cursor.
	int mouse_x() {al_get_mouse_state(state); return state->x;} 	///< Returns where mouse is relative to upper-left corner of display
	int mouse_y() {al_get_mouse_state(state); return state->y;}	///< Returns where mouse is relative to upper-left corner of display
	bool is_building_chosen() {return !chosen_building.expired();}
	int check_death();						///< Removes pointers to dead objects from game_mouse inner structures.
	int draw_button_info();						///< Draws info about right button on panel.
	int choose_button(boost::shared_ptr<button> button_to_choose);	///< Makes button the active one, which will be used when player clicks to map.
	int unchoose_button();						
	game_mouse();
	~game_mouse();
	ALLEGRO_MOUSE_STATE* state;
	boost::weak_ptr<building> chosen_building;			///< The selected building.
	std::vector<boost::weak_ptr<people>> chosen_people;		///< Selected people.
	boost::weak_ptr<button> chosen_button;				///< Selected button.

private:
	std::pair<int, int> find_mouse_location(int screen_position_x, int screen_position_y);
	int button_down_game_x;					
	int button_down_game_y;
	int button_down_tile_x;
	int button_down_tile_y;
	bool is_left_button_down;
	bool is_right_button_down;
	int tile_x;
	int tile_y;
	int mouse_wheel_position;
	bool scrolling;
};

#endif
