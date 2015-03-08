#ifndef object_h_bedas_guard
#define object_h_bedas_guard

#include "core.h"

class chosen_mouse;

class tile;

extern std::ofstream log_file;

int compute_game_x(int tile_x, int tile_y);		//defined in core.h
int compute_game_y(int tile_x, int tile_y);

class game_object {
public:
	game_object(int tile_x, int tile_y, int surface_height, bool is_real, picture image, int number_of_floors, game_object_type type_of_object);
	int update(int tile_x, int tile_y, int surface_height);
	virtual std::vector<game_object*> draw(int screen_position_x, int screen_position_y) = 0;
	int show_surface_height() {return surface_height;}
	int show_current_drawing_height() {return surface_height + drawing_floor;}
	int show_tile_x() {return tile_x;}
	int show_tile_y() {return tile_y;}
	int show_game_x() {return compute_game_x(tile_x, tile_y);}
	int show_game_y() {return compute_game_y(tile_x, tile_y);}
	virtual bool show_visible() {return true;}
	virtual void set_surface_height(int height);	
	virtual void rotate(int tile_x, int tile_y, bool clockwise) {update(tile_x, tile_y, this->surface_height);}
	
	static int highest_surface;
	bool draw_green;
	game_object_type type_of_object;
	
	friend class boost::serialization::access;

	template <class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		LOG("serializing");
		ar & highest_surface;
		ar & draw_green;
		ar & type_of_object;
		ar & tile_x;
		ar & tile_y;
		ar & game_x;
		ar & game_y;
		ar & number_of_floors;
		ar & drawing_floor;
		ar & is_real;
		ar & image;
		ar & surface_height;
	}

protected:
	int tile_x;
	int tile_y;
	int game_x;
	int game_y;
	int number_of_floors;
	int drawing_floor;
	bool is_real;
	picture image;
	game_object() {}		//for boost serialization

private:
	int surface_height;
	

};

#endif
