#ifndef others_h
#define others_h

#define TIME_TO_PRODUCE_CARRIER 60

extern std::ofstream log_file;
extern int display_height;
extern ALLEGRO_FONT* font25;
extern ALLEGRO_BITMAP** image_list;

class carrier;
class building;

class game_info{
public:
	static int map_width;
	static int map_height;
	//static ALLEGRO_TIMER* timer;
	static ALLEGRO_DISPLAY* display;
	static int fps;
	static bool close_display;
	static bool fullscreen;
	static bool music;
};

class missile{
public:
	missile_type type;
	missile(missile_type type, tile* attacker_position, int damage, tile* goal);
	int draw_missile(int screen_position_x, int screen_position_y);
	bool is_death() {return bIs_death;}
	
	friend class boost::serialization::access;

	template <class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & game_x;
		ar & game_y;
		ar & damage;
		ar & attacker_position;
		ar & angle;
		ar & game_x_change;
		ar & game_y_change;
		ar & frames_to_goal;
		ar & goal;
		ar & image;
		ar & bIs_death;
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

possible_borders tile_borders(tile* cTile, direction way);

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

class stock
{
public:
	stock(int capacity) : capacity(capacity), stored(NUMBER_OF_RESOURCES, 0) {}	
	virtual int save(resources type, int amount);	//if cant save everyhing, returns how much remainds
	void save_list(std::vector<int> prices);		//what cant be saved is thrown away
	virtual bool try_subtract(resources type, int amount);	//if can subtract, returns true, otherwise returns false
	virtual bool try_subtract_list(std::vector<int> price);
	virtual void subtract(resources type, int amount);
	int show_amount(resources type) {return stored[type];}
	int show_capacity() {return capacity;}
	void draw_nonzero_resources(int real_x, bool with_capacity);
	void increase_capacity(int new_capacity);
	
	friend class boost::serialization::access;

	template <class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & capacity;
		ar & stored;
	}

protected:
	int capacity;
	std::vector<int> stored;

	stock() {}	//for boost serialization
};

class carrier_output : public stock
{
public:
	carrier_output(int capacity, std::vector<resources> in, std::vector<resources> out, int number_of_carriers);
	void update();
	bool reserve_transaction(resources resource_type, int amount, transaction_type type);	//returns true on success
	void delete_transaction(resources resource_type, int amount, transaction_type type);
	bool accomplish_transaction(resources resource_type, int amount, transaction_type type);
	int show_max_possible_transaction(resources resource_type, transaction_type type);
	int show_free_space(resources resource_type) {return capacity - stored[static_cast<int>(resource_type)] - reserved_in[static_cast<int>(resource_type)];}
	void add_idle_carrier(boost::weak_ptr<carrier> returned_carrier) {idle_carriers.push_back(returned_carrier);}
	void init(boost::shared_ptr<building> building_ptr) {this_building = building_ptr;}
	void change_accepted_resources(resources r, bool in, bool add);
	int show_idle_carrier_capacity();
	
	friend class boost::serialization::access;

	template <class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & boost::serialization::base_object<stock>(*this);
		ar & reserved_in;
		ar & reserved_out;
		ar & in;
		ar & out;
		ar & carriers;
		ar & idle_carriers;
		ar & this_building;
		ar & max_number_of_carriers;
		ar & time_to_produce_carrier;
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
