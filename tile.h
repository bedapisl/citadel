#ifndef tile_h
#define tile_h

#include "core.h"

#define MAX_MINIMAP_UPDATES 512

class tree;
class button;
class people;
class building;
class warrior;

extern std::ofstream log_file;

class tile : public game_object
{
public:
	tile(tile_type type, int tile_x, int tile_y, int surface_height);
	std::vector<game_object*> draw(int screen_position_x, int screen_position_y);
	int set_right_drawing(std::vector<std::vector<boost::shared_ptr<tile>>>& map);
	void set_accessible_neighbours(std::vector<std::vector<boost::shared_ptr<tile>>> & map);
	void build(building_type type, player owner);
	boost::weak_ptr<warrior> warrior_born(people_type type, player owner);
	int people_die();
	int people_come(boost::shared_ptr<people> people_which_comes);
	int people_leave(boost::shared_ptr<people> people_which_leaves);
	int show_effective_height();
	borders show_border() {return border;}
	tile_type show_type() {return type;}
	bool show_visible() {return visible;}
	bool show_draw_building() {return draw_building;}
	void set_type(tile_type type);
	bool is_free() {return ((building_on_tile.expired()) && (!are_people_on_tile()) && (object == NOTHING));}
	bool is_ramp() {return type == RAMP;}
	bool are_people_on_tile();
	void check_death_people_on_tile();
	int set_draw_building(bool draw) {draw_building = draw; return 0;}
	int add_object(object_on_tile);
	int add_path(bool real);
	int remove_path(bool real);
	bool unreal_path_on_tile() {return path_on_tile;}
	bool real_path_on_tile() {return path_on_tile && bIs_path_real;}
	borders show_borders() {return border;}
	bool is_water_tile() {return type == WATER;}
	void add_water() {type = WATER;}
	bool is_fertile() {return fertile;}
	void set_tile_type(tile_type type) {this->type = type;}
	void add_fertile();
	boost::weak_ptr<building> building_on_tile;
	std::vector<boost::weak_ptr<people>> people_on_tile;
	boost::weak_ptr<button> action_on_tile;
	int number_of_tile_with_action;		//used by buttons with multiple selection to draw objects in right order
	object_on_tile object;
	bool can_go_inside_building;
	bool can_go_on_building;
	bool stairs_on_tile;
	picture tile_object_image;
	std::vector<tile*> accessible_neighbours;	//pointers to neighbour tiles, where can people move if there are no buildings, people, ... if they cant move, pointer is NULL
	std::vector<tile*> neighbours_with_path;	//only neighbours with real path
	int set_path_drawing();
	static void set_neighbours_with_path();
	static std::vector<tile*> minimap_updates;

	void prepare_serialization();
	void finish_serialization();
	void check_consistency();
	
	std::vector<std::pair<int, int>> serializable_accessible_neighbours;
	std::vector<std::pair<int, int>> serializable_neighbours_with_path;
	
	friend class boost::serialization::access;

	template <class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		LOG("serializing tile");
		
		ar & boost::serialization::base_object<game_object>(*this);
		
		ar & building_on_tile;
		ar & people_on_tile;
		//ar & action_on_tile;	
		ar & number_of_tile_with_action;
		ar & object;
		ar & can_go_inside_building;
		ar & can_go_on_building;
		ar & stairs_on_tile;
		ar & tile_object_image;
		//ar & accessible_neighbours;		//this causes stack overflow
		//ar & neighbours_with_path		//causes stack overflow
		ar & minimap_updates;
		
		ar & serializable_accessible_neighbours;
		ar & serializable_neighbours_with_path;
		ar & path_on_tile;
		ar & bIs_path_real;
		ar & visible;
		ar & border;
		ar & path_border;
		ar & type;
		ar & draw_building;
		ar & fertile;
	}

private:
public:
	tile() {}		//for boost serialization

	bool path_on_tile;
	bool bIs_path_real;
	bool visible;
	borders border;
	borders path_border;
	tile_type type;
	bool draw_building;
	bool fertile;
};

#endif
