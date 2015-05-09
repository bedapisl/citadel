
/**
 * \file others.h
 * \brief Contains game_info, graphical_texts, \ref missile, \ref stock and carrier_output classess.
 */

#ifndef others_h
#define others_h

#define TIME_TO_PRODUCE_CARRIER 60

extern std::ofstream log_file;
extern int display_height;
extern ALLEGRO_FONT* font25;
extern ALLEGRO_BITMAP** image_list;

class carrier;
class building;

/**
 * \brief Class maintaing high level informations about game.
 * Info is loaded from data/config.txt file.
 * If file cannot be opened or does not contains values of variables in proper format, default values are used.
 */
class game_info{
public:
	static int map_width;
	static int map_height;
	static ALLEGRO_DISPLAY* display;
	static int fps;
	static bool close_display;
	static bool fullscreen;
	static bool music;
	static int map_generator_base_tile;
	static double map_generator_gauss_deviation;
	static double map_generator_base_ramp_probability;
	static double map_generator_ramp_start;
	static double map_generator_ramp_end;

	static void load_game_info();
private:
	static int lower_case(std::string& word);
	static std::vector<std::string> split(std::string line);
};

/**
 * \brief Class managing texts which appears on the map.
 * Texts automatically disappear after some time.
 * Function "draw_and_update" should be called each frame.
 */
class graphical_texts
{
public:
	graphical_texts() { }
	void add_hint(int game_x, int game_y, const std::string& hint_text, ALLEGRO_COLOR color) {hints.push_back(hint(game_x, game_y, hint_text, color));}
			///< Adds new texts to be drawn on given location.
	void draw_and_update(int screen_position_x, int screen_position_y);	///< Draws and updates texts - deletes old texts.

private:
	static const int standard_hint_time = 90;
	struct hint
	{
		hint(int x, int y, std::string text, ALLEGRO_COLOR color) : x(x), y(y), time_left(standard_hint_time), text(text), color(color) { }
		int x, y;
		int time_left;
		std::string text;
		ALLEGRO_COLOR color;
	};
	
	std::vector<hint> hints;
};
/**
 * \brief Represents any projectile in the game.
 */
class missile{
public:
	missile_type type;
	missile(missile_type type, tile* attacker_position, int damage, tile* goal);
	int draw_and_update(int screen_position_x, int screen_position_y);	///< Draws and update missile. If missile reached goal it will become dead.
	bool is_death() {return bIs_death;}
	
	friend class boost::serialization::access;

	template <class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		using boost::serialization::make_nvp;
		ar & make_nvp("game_x", game_x);
		ar & make_nvp("game_y", game_y);
		ar & make_nvp("damage", damage);
		ar & make_nvp("attacker_position", attacker_position);
		ar & make_nvp("angle", angle);
		ar & make_nvp("game_x_change", game_x_change);
		ar & make_nvp("game_y_change", game_y_change);
		ar & make_nvp("frames_to_goal", frames_to_goal);
		ar & make_nvp("goal", goal);
		ar & make_nvp("image", image);
		ar & make_nvp("bIs_death", bIs_death);
	}
private:
	missile() {}	//for boost::serialization

	int game_x;
	int game_y;
	int damage;
	tile* attacker_position;
	float angle;
	int game_x_change;
	int game_y_change;
	int frames_to_goal;
	tile* goal;
	picture image;
	bool bIs_death;
	
};

/// Returns whether element is in vector. Linear complexity.
template <typename T>
bool contains(std::vector<T> v, T element)
{
	for(size_t i=0; i<v.size(); ++i)
	{
		if(element == v[i])
			return true;
	}
	return false;
}
/**
 * \brief Represents any storage for ingame resources.
 */
class stock
{
public:
	stock(int capacity) : capacity(capacity), stored(NUMBER_OF_RESOURCES, 0) {}	
	virtual ~stock() {}
	virtual int save(resources type, int amount);		///< Saves as much of resource as possible, returns the rest.
	void save_list(std::vector<int> amount);		///< Saves as much as possible, throws away the rest.
	virtual bool try_subtract(resources type, int amount);	///< If possible subtract resource from stock and returns true. Otherwise do nothing and return false.
	virtual bool try_subtract_list(std::vector<int> amount);///< If possible subtract resource from stock and returns true. Otherwise do nothing and return false.
	virtual void subtract(resources type, int amount);	///< Subtract resources. If not possible, throws exception.
	int show_amount(resources type) {return stored[type];}	///< Returns amount of resources in stock. Doesn't take into account reservations from carrier_output.
	int show_capacity() {return capacity;}
	void draw_nonzero_resources(int real_x, bool with_capacity);	///< Draws resources from stock to panel.
	void increase_capacity(int new_capacity);
	
	friend class boost::serialization::access;

	template <class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		using boost::serialization::make_nvp;
		ar & make_nvp("capacity", capacity);
		ar & make_nvp("stored", stored);
	}

protected:
	int capacity;
	std::vector<int> stored;

	stock() {}	//for boost serialization
};
/**
 * \brief Manages transporting resources with carriers.
 * Class is meant to be part of some building, member pointer "this_building" should points to it.
 * Creates carriers, assigns them tasks and by base class manages resources in the building.
 * Must be updated (function "update()") each frame to work properly.
 */
class carrier_output : public stock
{
public:
	carrier_output(int capacity, std::vector<resources> in, std::vector<resources> out, int number_of_carriers);
	void update();		///< Should be called once per frame. Creates new carriers, assign work to idle ones.
	bool reserve_transaction(resources resource_type, int amount, transaction_type type);	///< Reserves resources or capacity for some transaction.
	void delete_transaction(resources resource_type, int amount, transaction_type type);	///< Removes previous reservation.
	bool accomplish_transaction(resources resource_type, int amount, transaction_type type);///< Fullfils previous reservation by accompilshing transaction.
	int show_max_possible_transaction(resources resource_type, transaction_type type);
	int show_free_space(resources resource_type) {return capacity - stored[static_cast<int>(resource_type)] - reserved_in[static_cast<int>(resource_type)];}
		///< How much more resources can be stored.
	int show_usable_amount(resources resource_type) {return stored[static_cast<int>(resource_type)] - reserved_out[static_cast<int>(resource_type)];}
	void add_idle_carrier(boost::weak_ptr<carrier> returned_carrier) {idle_carriers.push_back(returned_carrier);}	///< Carrier will start waiting for carrier_output orders.
	void init(boost::shared_ptr<building> building_ptr) {this_building = building_ptr;}	
	void change_accepted_resources(resources r, bool in, bool add);
	int show_idle_carrier_capacity();		///< How much resources can idle carriers in this carrier_output carry.
	
	friend class boost::serialization::access;

	template <class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		using boost::serialization::make_nvp;
		ar & make_nvp("stock", boost::serialization::base_object<stock>(*this));
		ar & make_nvp("reserved_in", reserved_in);
		ar & make_nvp("reserved_out", reserved_out);
		ar & make_nvp("in", in);
		ar & make_nvp("out", out);
		ar & make_nvp("carriers", carriers);
		ar & make_nvp("idle_carriers", idle_carriers);
		ar & make_nvp("this_building", this_building);
		ar & make_nvp("max_number_of_carriers", max_number_of_carriers);
		ar & make_nvp("time_to_produce_carrier", time_to_produce_carrier);
	}

private:
	void assign_tasks();
	std::vector<int> reserved_in;
	std::vector<int> reserved_out;
	std::vector<resources> in;
	std::vector<resources> out;
	std::vector<boost::weak_ptr<carrier>> carriers;
	std::vector<boost::weak_ptr<carrier>> idle_carriers;
	boost::weak_ptr<building> this_building;
	int max_number_of_carriers;
	int time_to_produce_carrier;

	carrier_output() {}		//for boost::serialization
};


#endif
