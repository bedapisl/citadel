#ifndef building_h
#define building_h

#include "core.h"

#define FRAMES_TO_PRODUCE 6*game_info::fps
#define LENGHT_OF_BUILDING_ACTION_QUEUE 10	//used only by barracks - how much warriors or archers can be in queue to be created
#define HOUSE_MARKET_DISTANCE 15		//path lenght between market and house such that house can take supplies from market
#define TIME_TO_EAT (40*game_info::fps)
#define MAX_WORKERS_DISTANCE_FROM_HOUSE 10	//how far can be house from building where can workers from house work

class people;
class missile;
class game_session;

struct building_info;
struct commodities_list;

extern ALLEGRO_BITMAP** image_list;

class building : public game_object, public boost::enable_shared_from_this<building>
{
public:
	virtual void draw_specific_interface() {}
	int draw_interface();
	virtual void function_click(int mouse_x, int mouse_y);
	void draw_function_info(int mouse_x, int mouse_y);
	virtual void specific_draw_function_info(int mouse_x, int mouse_y) {}
	int update();
	virtual int specific_update() {return 0;}
	virtual void upgrade() {throw new std::exception;}
	void general_upgrade(); 				//subtract price of upgrade and calls upgrade() 
	std::vector<game_object*> draw(int screen_position_x, int screen_position_y);
	int show_tile_x() {return tile_x;}
	int show_tile_y() {return tile_y;}
	building_type show_type() {return type;}
	bool is_death() {return bIs_death;}
	player show_owner() {return owner;}
	int show_actual_workers() {return actual_workers;}
	int show_required_workers() {return required_workers;}
	int show_action_duration() {return action_duration;}
	building_size show_size() {return size;}
	bool show_stopped() {return stopped;}
	int damage(int damage);
	int destroy_building();
	int change_working() {stopped = !stopped; return 0;}
	int stop_working();
	int draw_life_bar(int screen_position_x, int screen_position_y);
	bool can_build_here(tile* here);
	bool can_be_upgraded();
	virtual bool can_be_stopped() {return false;}
	virtual int rotate(int new_tile_x, int new_tile_y, bool clockwise);
	void set_drawing_tile();
	virtual bool has_carrier_output() {return false;}
	virtual boost::shared_ptr<carrier_output> show_carrier_output() {return NULL;}
	virtual ~building() {}
	static void assign_workers();

	static boost::shared_ptr<building> create_building(building_type type, int tile_x, int tile_y, int surface_height, player owner, bool is_real);

	int actual_workers;
	building_size size;
	building_type type;
	int id;
	static int next_id;
	const int minimal_distance_from_enemies_for_building = 10;

	friend class boost::serialization::access;

	template <class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & boost::serialization::base_object<game_object>(*this);
		ar & actual_workers;
		ar & size;
		ar & type;
		ar & id;
		ar & next_id;
		ar & action_duration;
		ar & life;
		ar & armor;
		ar & height_of_life_bar;
		ar & max_life;
		ar & owner;
		ar & bIs_death;
		ar & required_workers;
		ar & stopped;
		ar & upgrade_level;
	}

protected:
	building() {}	//for boost::serialization

	building(building_type type, int tile_x, int tile_y, int surface_height, player owner, bool is_real);
	int compute_button_number(int mouse_x, int mouse_y);
	virtual void set_carrier_output() {}
	int action_duration;		
	int life;
	int armor;
	int height_of_life_bar;
	int max_life;
	player owner;
	bool bIs_death;
	int required_workers;
	bool stopped;			//building is stopped by player
	int upgrade_level;
};

class tower : public building
{
public:
	tower(building_type type, int tile_x, int tile_y, int surface_height, player owner, bool is_real);
	~tower();
	std::vector<game_object*> draw(int screen_position_x, int screen_position_y);
	int rotate(int tile_x, int tile_y, bool clockwise);
	int specific_update();
	tile* doors_tile;
	void add_people_to_draw(boost::weak_ptr<people> p) {people_to_draw.push_back(p);}
	void reset_people_to_draw() {people_to_draw.clear();}

	friend class boost::serialization::access;

	template <class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{	
		ar & boost::serialization::base_object<building>(*this);
		ar & people_to_draw;
	}

private:
	tower() {}	//for boost serialization
	std::vector<boost::weak_ptr<people>> people_to_draw;
	//int attack;
	//int range;
};

class barracks : public building
{
public:
	barracks(building_type type, int tile_x, int tile_y, int surface_height, player owner, bool is_real);
	int specific_update();
	void function_click(int mouse_x, int mouse_y);
	void specific_draw_function_info(int mouse_x, int mouse_y);
	void draw_specific_interface();
	int show_number_of_actions() {return actions.size();}
	bool has_carrier_output() {return true;}
	void upgrade();
	boost::shared_ptr<carrier_output> show_carrier_output() {return output;}
	void check_death_supported_units();
	
	friend class boost::serialization::access;

	template <class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{	
		ar & boost::serialization::base_object<building>(*this);
		ar & supported_units;
		ar & actions;
		ar & output;
		ar & number_of_functions;
	}
private:	
	barracks() {}	//for boost serialization

	void update_supported_units();
	std::vector<std::pair<boost::weak_ptr<warrior>, int>> supported_units;
	std::vector<people_type> actions;
	boost::shared_ptr<carrier_output> output;
	const int time_to_make_unit = 6*game_info::fps;
	const std::vector<int> max_units{4, 5, 6, 7};
	const int time_to_support_unit = 30*game_info::fps;
	const int max_actions = 7;
	int number_of_functions;
};

class warehouse : public building
{
public:
	warehouse(building_type type, int tile_x, int tile_y, int surface_height, player owner, bool is_real);
	int specific_update();
	void draw_specific_interface();
	bool has_carrier_output() {return true;}
	boost::shared_ptr<carrier_output> show_carrier_output() {return output;}
	
	friend class boost::serialization::access;

	template <class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{	
		ar & boost::serialization::base_object<building>(*this);
		ar & output;	
	}
private:
	warehouse() {}
	boost::shared_ptr<carrier_output> output;

};

class production_building : public building
{
public:
	production_building(building_type type, int tile_x, int tile_y, int surface_height, player owner, bool is_real);
	int specific_update();
	void draw_specific_interface();
	bool has_carrier_output() {return true;}
	bool can_be_stopped() {return true;}
	boost::shared_ptr<carrier_output> show_carrier_output() {return output;}
	void upgrade();
	
	friend class boost::serialization::access;

	template <class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & boost::serialization::base_object<building>(*this);
		ar & output;
		ar & resource_produced;
		ar & needed_resources;
		ar & current_time_to_produce;
		ar & amount_produced;
	}

private:	
	production_building() {}
	boost::shared_ptr<carrier_output> output;
	resources resource_produced;
	std::vector<resources> needed_resources;
	const int time_to_produce = 6*game_info::fps;
	int current_time_to_produce;
	int amount_produced;
};

class wall : public building
{
public:
	wall(building_type type, int tile_x, int tile_y, int surface_height, player owner, bool is_real);
	~wall();
	int specific_update();
	std::vector<game_object*> draw(int screen_position_x, int screen_position_y);
	void add_people_to_draw(boost::weak_ptr<people> p) {people_to_draw.push_back(p);}
	void reset_people_to_draw() {people_to_draw.clear();}
	
	friend class boost::serialization::access;

	template <class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{	
		ar & boost::serialization::base_object<building>(*this);
		ar & people_to_draw;
		ar & borders;
	}

private:
	wall() {}
	std::vector<boost::weak_ptr<people>> people_to_draw;
	std::vector<bool> borders;

};

class gate : public building
{
public:
	gate(building_type type, int tile_x, int tile_y, int surface_height, player owner, bool is_real);
	~gate();
	int specific_update();
	void draw_specific_interface();
	void function_click(int mouse_x, int mouse_y);
	std::vector<game_object*> draw(int screen_position_x, int screen_position_y);
	int rotate(int tile_x, int tile_y, bool clockwise);
	void set_gate_tile();
	bool show_open() {return open;}
	
	friend class boost::serialization::access;

	template <class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & boost::serialization::base_object<building>(*this);
		ar & open;
		ar & image_to_draw;
		ar & northeast_southwest;
		ar & gate_tile;
	}

private:
	gate() {}
	bool open;
	int image_to_draw;
	bool northeast_southwest;
	tile* gate_tile;
};

class house : public building
{
public:
	house(building_type type, int tile_x, int tile_y, int surface_height, player owner, bool is_real);
	int specific_update();
	void draw_specific_interface();
	int show_happiness_modifier() {return base_happiness[upgrade_level] - 5*missing_resources;}
	int number_of_workers() {return workers_by_house_level[upgrade_level];}
	void upgrade() {}
	void set_idle_workers(int idle_workers) {this->idle_workers = idle_workers;}
		
	friend class boost::serialization::access;

	template <class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{	
		ar & boost::serialization::base_object<building>(*this);
		ar & starving;
		ar & has_market;
		ar & missing_resources;
		ar & idle_workers;
	}

private:
	house() {}
	bool starving;
	bool has_market;
	int missing_resources;
	const std::vector<int> workers_by_house_level{5,6,7,8};
	const std::vector<int> base_happiness{-5, -2, 2, 5};
	int idle_workers;
};

class market : public building
{
public:
	market(building_type type, int tile_x, int tile_y, int surface_height, player owner, bool is_real);
	int specific_update() {return 0;}
	int show_max_distance_to_house() {return max_distance_to_house;}
	void draw_specific_interface();
	bool has_carrier_output() {return true;}
	bool can_be_stopped() {return true;}
	boost::shared_ptr<carrier_output> show_carrier_output() {return output;}
	void upgrade();
	
	friend class boost::serialization::access;

	template <class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{	
		ar & boost::serialization::base_object<building>(*this);
		ar & max_distance_to_house;
		ar & output;
	}

private:
	market() {}
	int max_distance_to_house;
	boost::shared_ptr<carrier_output> output;
	const int upgrade_capacity_increase = 10;
	const int upgrade_house_distance_increase = 3;
};

class great_hall : public building
{
public:
	great_hall(building_type type, int tile_x, int tile_y, int surface_height, player owner, bool is_real);
	int specific_update() {return 0;}
	void draw_specific_interface();
	void function_click(int mouse_x, int mouse_y);
	void specific_draw_function_info(int mouse_x, int mouse_y);
	bool has_carrier_output() {return true;}
	bool can_be_stopped() {return true;}
	boost::shared_ptr<carrier_output> show_carrier_output() {return output;}
	void upgrade();
	
	friend class boost::serialization::access;

	template <class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & boost::serialization::base_object<building>(*this);
		ar & output;
	}

private:
	great_hall() {}
	boost::shared_ptr<carrier_output> output;
	const std::vector<int> honour_for_upgrade{10, 15, 20, 25};
	const std::vector<int> honour_for_feast{5, 20, 50, 100};
	const int food_for_smallest_feast = 2;
};

class church : public building
{
public:
	church(building_type type, int tile_x, int tile_y, int surface_height, player owner, bool is_real);
	int specific_update();  
	void draw_specific_interface();
	void upgrade() {}
	bool can_be_stopped() {return true;}
		
	friend class boost::serialization::access;

	template <class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & boost::serialization::base_object<building>(*this);
	}

private:
	church() {}
	const int time_to_produce_honour = 5 * game_info::fps;
	const std::vector<int> honour_produced{1,2,3,4};
};

class store : public building
{
public:
	store(building_type type, int tile_x, int tile_y, int surface_height, player owner, bool is_real);
	int specific_update();
	void draw_specific_interface();
	void function_click(int mouse_x, int mouse_y);
	void specific_draw_function_info(int mouse_x, int mouse_y);
	bool has_carrier_output() {return true;}
	bool can_be_stopped() {return true;}
	boost::shared_ptr<carrier_output> show_carrier_output() {return output;}
	void upgrade();
	void draw_window(int start_x, int start_y);
	void window_function_click(int relative_mouse_x, int relative_mouse_y);
	
	friend class boost::serialization::access;

	template <class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & boost::serialization::base_object<building>(*this);
		ar & output;
		ar & buying;
	}

private:
	store() {}
	boost::shared_ptr<carrier_output> output;
	std::vector<resource_status> buying;
	std::vector<int> show_selling_prices();
	std::vector<int> show_buying_prices();
	void draw_button(int x, int y, resources r);
	const int y_offset = 40;
	const int line_heigth = 30;
	const int button_length = 130;
	const int button_height = 25;
	const int button_start_x = 80;
	const int button_start_y = 5;
	const int exchange_rate_length = 150;
	const std::vector<int> time_to_trade{10*game_info::fps, 15*game_info::fps, 20*game_info::fps, 25*game_info::fps};
};

class scout : public building
{
public:
	scout(building_type type, int tile_x, int tile_y, int surface_height, player owner, bool is_real);
	int specific_update() {return 0;}
	void draw_specific_interface();
	bool has_carrier_output() {return true;}
	boost::shared_ptr<carrier_output> show_carrier_output() {return output;}
		
	friend class boost::serialization::access;

	template <class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & boost::serialization::base_object<building>(*this);
		ar & output;
	}

private:
	scout() {}
	boost::shared_ptr<carrier_output> output;
};

class stairs : public building
{
public:
	stairs(building_type type, int tile_x, int tile_y, int surface_height, player owner, bool is_real);
	~stairs();
	int specific_update() {return 0;}
	void draw_specific_interface();
	std::vector<game_object*> draw(int screen_position_x, int screen_position_y);
	int rotate(int tile_x, int tile_y, bool clockwise);
	
	friend class boost::serialization::access;

	template <class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & boost::serialization::base_object<building>(*this);
	}

private:
	stairs() {}
};


boost::weak_ptr<warrior> warrior_born_near(people_type type, int tile_x, int tile_y, player owner);
int show_building_price(building_type type, resources resource_type, upgrade_level upgrade);
std::vector<tile*> tiles_under_building(int tile_x, int tile_y, building_size type_of_building);
std::vector<std::pair<boost::shared_ptr<building>, int>> buildings_connected_by_path(std::vector<tile*> start, int distance);


#endif
