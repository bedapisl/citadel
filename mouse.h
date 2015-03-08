#ifndef mouse_h_bedas_guard
#define mouse_h_bedas_guard

#include "core.h"
#include "others.h"

class game_mouse
{
public:
	void left_button_go_down(int &screen_position_x, int &screen_position_y, rotation* rotate);
	void right_button_go_down();
	void left_button_go_up(int screen_position_x, int screen_position_y);
	void right_button_go_up();
	int move(int & screen_position_x, int & screen_position_y); 
	std::pair<int, int> find_mouse_location(int screen_position_x, int screen_position_y);
	int draw_mouse(int screen_position_x, int screen_position_y);
	ALLEGRO_MOUSE_STATE* state;
	int show_tile_x() {return tile_x;}
	int show_tile_y() {return tile_y;}
	int mouse_x() {al_get_mouse_state(state); return state->x;} 
	int mouse_y() {al_get_mouse_state(state); return state->y;}
	bool is_building_chosen() {return !chosen_building.expired();}
	int check_death();
	//int show_number_of_chosen_people() {return chosen_people.size();}
	int draw_button_info();
	int choose_button(boost::shared_ptr<button> button_to_choose);
	int unchoose_button();
	game_mouse();
	~game_mouse();
	boost::weak_ptr<building> chosen_building;
	std::vector<boost::weak_ptr<people>> chosen_people;
	//chosen is_chosen;
	boost::weak_ptr<button> chosen_button;

private:
	//int number_of_chosen_people;
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

int* mouse_over(int mouse_x, int mouse_y, int screen_position_x, int screen_position_y, const std::vector<std::vector<boost::shared_ptr<tile>>> & tile_list);

#endif
