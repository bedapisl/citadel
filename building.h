#ifndef building_h_bedas_guard
#define building_h_bedas_guard

#include "core.h"

#define FRAMES_TO_PRODUCE 6*game_info::fps
#define LENGHT_OF_BUILDING_ACTION_QUEUE 10	//used only by barracks - how much warriors or archers can be in queue to be created
#define HOUSE_MARKET_DISTANCE 15		//path lenght between market and house such that house can take supplies from market
#define TIME_TO_EAT (40*game_info::fps)
#define MAX_WORKERS_DISTANCE_FROM_HOUSE 10	//how far can be house from building where can workers from house work

class people;
class missile;
class game_session;

class building_info;
struct commodities_list;

extern ALLEGRO_BITMAP** image_list;

/**
 * \brief Virtual class representing buildings.
 */

class building : public game_object, public boost::enable_shared_from_this<building>
{
public:	
	static boost::shared_ptr<building> create_building(building_type type, int tile_x, int tile_y, int surface_height, player owner, bool is_real); 
										///< Creates building on tile.
	void update();								///< Is called once per frame. Updates/manages building.
	std::vector<game_object*> draw(int screen_position_x, int screen_position_y);	
			///< Draws one floor of building. Returns what will be drawn above it (e.g. people staying on walls, next floor of building).
	void draw_interface();							///< When building is selected, draws gui and info about it to main panel.
	virtual void function_click(int mouse_x, int mouse_y);			///< Handles clicks to main panel, when building is selected.
	void draw_function_info(int mouse_x, int mouse_y);			///< Draws info about buttons on main panel, when building is selected.
	void draw_life_bar(int screen_position_x, int screen_position_y);	///< Draws amount of life left above the building. Called when mouse points to building.
	void damage(int damage);						///< Processes attack on building. 
	virtual void rotate(int new_tile_x, int new_tile_y, bool clockwise);	///< Rotates building.
	void set_drawing_tile();						///< Chooses tile from which this building will be drawed. 
	virtual ~building() {}
	
	static void assign_workers();						///< Distributes all workers between buildings.
	static can_build_output can_build_here(tile* here, building_type type);	///< Returns if building can be build on given tile.
	
	virtual boost::shared_ptr<carrier_output> show_carrier_output() {return NULL;}
	virtual bool has_carrier_output() {return false;}			///< Returns whether carriers can carry resources to/from this building.
	building_type show_type() {return type;}				///< Returns type of building (e.g. WAREHOUSE, MARKET, QUARRY).
	bool is_death() {return bIs_death;}		
	player show_owner() {return owner;}					///< Returns owner of building. Currently all buildigns are owned by BLUE_PLAYER.
	int show_actual_workers() {return actual_workers;}
	int show_required_workers() {return required_workers;}
	building_size show_size() {return size;}
	bool show_stopped() {return stopped;}					///< Returns whether building is working or if it is stopped.
	
	building_size size;
	building_type type;
	bool draw_selection;				///< Indicates if white rectangle around building should be drawn.
	int id;
	static int next_id;
	static const int minimal_distance_from_enemies_for_building = 20;	///< It is not possible to build anything when enemies are this close.

	friend class boost::serialization::access;

	template <class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		using boost::serialization::make_nvp;
		ar & make_nvp("game_object", boost::serialization::base_object<game_object>(*this));
		ar & make_nvp("actual_workers", actual_workers);
		ar & make_nvp("size", size);
		ar & make_nvp("type", type);
		ar & make_nvp("id", id);
		ar & make_nvp("next_id", next_id);
		ar & make_nvp("action_duration", action_duration);
		ar & make_nvp("life", life);
		ar & make_nvp("armor", armor);
		ar & make_nvp("height_of_life_bar", height_of_life_bar);
		ar & make_nvp("max_life", max_life);
		ar & make_nvp("owner", owner);
		ar & make_nvp("bIs_death", bIs_death);
		ar & make_nvp("required_workers", required_workers);
		ar & make_nvp("stopped", stopped);
		ar & make_nvp("upgrade_level", upgrade_level);
	}

protected:
	building() {}	//for boost::serialization
	building(building_type type, int tile_x, int tile_y, int surface_height, player owner, bool is_real);
	void destroy_building();
	virtual void draw_specific_interface() {}	///< Draws gui and info specific to the kind of building to main panel. 
	virtual void specific_draw_function_info(int mouse_x, int mouse_y) {}
	virtual void specific_update() { }
	virtual void upgrade() {throw new std::exception;}
	void change_working() {stopped = !stopped;}
	virtual bool can_be_stopped() {return false;}
	void stop_working() {stopped = true;}
	bool can_be_upgraded();		
	void general_upgrade(); 				//subtract price of upgrade and calls upgrade() 
	int compute_button_number(int mouse_x, int mouse_y);
	
	static can_build_output enough_resources(building_type type, int start_tile_x, int start_tile_y, int end_tile_x, int end_tile_y);

	virtual void set_carrier_output() {}
	int actual_workers;
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
	std::vector<game_object*> draw(int screen_position_x, int screen_position_y); 	///< Draws one floor. Returns objects which should be drawed above it.
	void rotate(int tile_x, int tile_y, bool clockwise);				///< Rotates tower.
	void add_people_to_draw(boost::weak_ptr<people> p) {people_to_draw.push_back(p);}	///< Register some people to be drawn on top of tower.
	void reset_people_to_draw() {people_to_draw.clear();}				///< No people will be drawed on top of tower.

	tile* doors_tile;								///< Tile where the doors are, which can be used to access tower.
	
	friend class boost::serialization::access;

	template <class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{	
		using boost::serialization::make_nvp;
		ar & make_nvp("building", boost::serialization::base_object<building>(*this));
		ar & make_nvp("people_to_draw", people_to_draw);
	}

private:
	tower() {}	//for boost serialization
	std::vector<boost::weak_ptr<people>> people_to_draw;	///< People which will be drawn on top of tower.
};

class barracks : public building
{
public:
	barracks(building_type type, int tile_x, int tile_y, int surface_height, player owner, bool is_real);
	void specific_update();				///< Updates barracks (e.g. decreases time to produce units, create units). Should be called once every frame.  
	void function_click(int mouse_x, int mouse_y);	///< Handles player click to the panel.
	void specific_draw_function_info(int mouse_x, int mouse_y);	///< Draws info about soldiers, which can be created.
	void draw_specific_interface();					///< Draw info and gui to panel, should be called if this building is selected.
	int show_number_of_actions() {return actions.size();}		///< Returns how many kinds of soldiers can be created (one action for each kind)
	bool has_carrier_output() {return true;}
	void upgrade();							///< Upgrades barracks - unlocks new type of soldier for these barracks and increases capacity.
	boost::shared_ptr<carrier_output> show_carrier_output() {return output;}
	
	friend class boost::serialization::access;

	template <class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{	
		using boost::serialization::make_nvp;
		ar & make_nvp("building", boost::serialization::base_object<building>(*this));
		ar & make_nvp("supported_units", supported_units);
		ar & make_nvp("actions", actions);
		ar & make_nvp("output", output);
		ar & make_nvp("number_of_functions", number_of_functions);
	}
private:	
	barracks() {}	//for boost serialization

	void update_supported_units();
	void check_death_supported_units();				
	std::vector<std::pair<boost::weak_ptr<warrior>, int>> supported_units;
	std::vector<people_type> actions;
	boost::shared_ptr<carrier_output> output;
	const int time_to_make_unit = 6*game_info::fps;
	const std::vector<int> max_units = std::vector<int>{4, 5, 6, 7};
	const int time_to_support_unit = 30*game_info::fps;
	const int max_actions = 7;
	int number_of_functions;
};

class warehouse : public building
{
public:
	warehouse(building_type type, int tile_x, int tile_y, int surface_height, player owner, bool is_real);
	void draw_specific_interface();			///< Draws info and gui to the panel specific to warehouse. 
	bool has_carrier_output() {return true;}
	boost::shared_ptr<carrier_output> show_carrier_output() {return output;}
	
	friend class boost::serialization::access;

	template <class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{	
		using boost::serialization::make_nvp;
		ar & make_nvp("building", boost::serialization::base_object<building>(*this));
		ar & make_nvp("output", output);	
	}
private:
	warehouse() {}
	boost::shared_ptr<carrier_output> output;

};
/**
 * \brief Represents any building which produces resources (woodcutter, quarry, farms, ...)
 */
class production_building : public building
{
public:
	production_building(building_type type, int tile_x, int tile_y, int surface_height, player owner, bool is_real);
	void specific_update();			///< Updates production building - decreases time to produce resource, produces resources. Called once each frame.
	void draw_specific_interface();			///< Draws info and gui to the panel specific to production_buildings.
	bool has_carrier_output() {return true;}
	bool can_be_stopped() {return true;}
	boost::shared_ptr<carrier_output> show_carrier_output() {return output;}
	void upgrade();	
	
	friend class boost::serialization::access;

	template <class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		using boost::serialization::make_nvp; 
		ar & make_nvp("building", boost::serialization::base_object<building>(*this));
		ar & make_nvp("output", output);
		ar & make_nvp("resource_produced", resource_produced);
		ar & make_nvp("needed_resources", needed_resources);
		ar & make_nvp("current_time_to_produce", current_time_to_produce);
		ar & make_nvp("amount_produced", amount_produced);
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

/**
 * \brief Represents walls and palisades.
 */
class wall : public building
{
public:
	wall(building_type type, int tile_x, int tile_y, int surface_height, player owner, bool is_real);
	~wall();
	void specific_update();
	std::vector<game_object*> draw(int screen_position_x, int screen_position_y);
	void add_people_to_draw(boost::weak_ptr<people> p) {people_to_draw.push_back(p);}
	void reset_people_to_draw() {people_to_draw.clear();}
	
	friend class boost::serialization::access;

	template <class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{	
		using boost::serialization::make_nvp;
		ar & make_nvp("building", boost::serialization::base_object<building>(*this));
		ar & make_nvp("people_to_draw", people_to_draw);
		ar & make_nvp("borders", borders);
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
	void specific_update();
	void draw_specific_interface();			///< Draws info and gui to the panel specific to gate.
	void function_click(int mouse_x, int mouse_y);
	std::vector<game_object*> draw(int screen_position_x, int screen_position_y);
	void rotate(int tile_x, int tile_y, bool clockwise);
	bool show_open() {return open;}
	
	friend class boost::serialization::access;

	template <class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		using boost::serialization::make_nvp;
		ar & make_nvp("building", boost::serialization::base_object<building>(*this));
		ar & make_nvp("open", open);
		ar & make_nvp("image_to_draw", image_to_draw);
		ar & make_nvp("northeast_southwest", northeast_southwest);
		ar & make_nvp("gate_tile", gate_tile);
	}

private:
	void set_gate_tile();
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
	void specific_update();
	void draw_specific_interface();			///< Draws info and gui to the panel specific to house.
	int show_happiness_modifier() {return base_happiness[upgrade_level] - 5*missing_resources;}
	int number_of_workers() {return workers_by_house_level[upgrade_level];}
	void upgrade() {}
	void set_idle_workers(int idle_workers) {this->idle_workers = idle_workers;}
		
	friend class boost::serialization::access;

	template <class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{	
		using boost::serialization::make_nvp;
		ar & make_nvp("building", boost::serialization::base_object<building>(*this));
		ar & make_nvp("starving", starving);
		ar & make_nvp("has_market", has_market);
		ar & make_nvp("missing_resources", missing_resources);
		ar & make_nvp("idle_workers", idle_workers);
	}

private:
	house() {}
	bool starving;
	bool has_market;
	int missing_resources;
	const std::vector<int> workers_by_house_level = std::vector<int>{5,6,7,8};
	const std::vector<int> base_happiness = std::vector<int>{-5, -2, 2, 5};
	int idle_workers;
};

class market : public building
{
public:
	market(building_type type, int tile_x, int tile_y, int surface_height, player owner, bool is_real);
	int show_max_distance_to_house() {return max_distance_to_house;}
	void draw_specific_interface();			///< Draws info and gui to the panel specific to market.
	bool has_carrier_output() {return true;}
	bool can_be_stopped() {return true;}
	boost::shared_ptr<carrier_output> show_carrier_output() {return output;}
	void upgrade();
	
	friend class boost::serialization::access;

	template <class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{	
		using boost::serialization::make_nvp;
		ar & make_nvp("building", boost::serialization::base_object<building>(*this));
		ar & make_nvp("max_distance_to_house", max_distance_to_house);
		ar & make_nvp("output", output);
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
	void draw_specific_interface();			///< Draws info and gui to the panel specific to great hall.
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
		using boost::serialization::make_nvp;
		ar & make_nvp("building", boost::serialization::base_object<building>(*this));
		ar & make_nvp("output", output);
	}

private:
	great_hall() {}
	boost::shared_ptr<carrier_output> output;
	const std::vector<int> honour_for_upgrade = std::vector<int>{10, 15, 20, 25};
	const std::vector<int> honour_for_feast = std::vector<int>{5, 20, 50, 100};
	const int food_for_smallest_feast = 2;
};

class church : public building
{
public:
	church(building_type type, int tile_x, int tile_y, int surface_height, player owner, bool is_real);
	void specific_update();  
	void draw_specific_interface();			///< Draws info and gui to the panel specific to church.
	void upgrade() {}
	bool can_be_stopped() {return true;}
		
	friend class boost::serialization::access;

	template <class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		using boost::serialization::make_nvp;
		ar & make_nvp("building", boost::serialization::base_object<building>(*this));
	}

private:
	church() {}
	const int time_to_produce_honour = 5 * game_info::fps;
	const std::vector<int> honour_produced = std::vector<int>{1,2,3,4};
};

class store : public building
{
public:
	store(building_type type, int tile_x, int tile_y, int surface_height, player owner, bool is_real);
	void specific_update();
	void draw_specific_interface();			///< Draws info and gui to the panel specific to store.
	void function_click(int mouse_x, int mouse_y);
	void specific_draw_function_info(int mouse_x, int mouse_y);
	bool has_carrier_output() {return true;}
	bool can_be_stopped() {return true;}
	boost::shared_ptr<carrier_output> show_carrier_output() {return output;}
	void upgrade();
	void draw_window(int start_x, int start_y);	///< Draws window that manages store.
	void window_function_click(int relative_mouse_x, int relative_mouse_y);		///< Handles mouse click to store's window.
	
	friend class boost::serialization::access;

	template <class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		using boost::serialization::make_nvp;
		ar & make_nvp("building", boost::serialization::base_object<building>(*this));
		ar & make_nvp("output", output);
		ar & make_nvp("buying", buying);
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
	const std::vector<int> time_to_trade = std::vector<int>{10*game_info::fps, 15*game_info::fps, 20*game_info::fps, 25*game_info::fps};
};

/**
 * \brief Currently not used.
 */
class scout : public building
{
public:
	scout(building_type type, int tile_x, int tile_y, int surface_height, player owner, bool is_real);
	void draw_specific_interface();			///< Draws info and gui to the panel specific to scout.
	bool has_carrier_output() {return true;}
	boost::shared_ptr<carrier_output> show_carrier_output() {return output;}
		
	friend class boost::serialization::access;

	template <class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		using boost::serialization::make_nvp;
		ar & make_nvp("building", boost::serialization::base_object<building>(*this));
		ar & make_nvp("output", output);
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
	std::vector<game_object*> draw(int screen_position_x, int screen_position_y);
	void rotate(int tile_x, int tile_y, bool clockwise);
	
	friend class boost::serialization::access;

	template <class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		using boost::serialization::make_nvp;
		ar & make_nvp("building", boost::serialization::base_object<building>(*this));
	}

private:
	stairs() {}
};


boost::weak_ptr<warrior> warrior_born_near(people_type type, int tile_x, int tile_y, player owner);	///< Creates new soldier on given location (or near if location is not free)
int show_building_price(building_type type, resources resource_type, upgrade_level upgrade);	///< Shows amount of needed resource of given type for buying or upgrading building.
std::vector<tile*> tiles_under_building(int tile_x, int tile_y, building_size type_of_building);
std::vector<std::pair<boost::shared_ptr<building>, int>> buildings_connected_by_path(std::vector<tile*> start, int distance); ///< Returns buildings connected by path to any of start tiles and distances to them. 


#endif
