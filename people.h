#ifndef people_h
#define people_h

#include "core.h"

#define CARRIER_CAPACITY 1

class tile;
class chosen_mouse;

struct A_star_tiles;

struct target
{
	target(tile* tile_target, target_priority priority) : type(TILE), tile_target(tile_target), priority(priority) {}
	target(boost::weak_ptr<people> people_target, target_priority priority) : type(PEOPLE), tile_target(NULL), people_target(people_target), priority(priority) {}
	target(boost::weak_ptr<building> building_target, target_priority priority) : type(BUILDING), tile_target(NULL), 
											building_target(building_target), priority(priority) {}
	friend class boost::serialization::access;

	template <class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		using boost::serialization::make_nvp;

		ar & make_nvp("type", type);
		ar & make_nvp("tile_target", tile_target);
		ar & make_nvp("people_target", people_target);
		ar & make_nvp("building_target", building_target);
		ar & make_nvp("number", number);
		ar & make_nvp("priority", priority);
	}

	game_object_type type;
	tile* tile_target;
	boost::weak_ptr<people> people_target;
	boost::weak_ptr<building> building_target;
	int number;
	target_priority priority;					//from 0 to 10
private:
	target() {};		//for boost::serialization

};

class people : public game_object, public boost::enable_shared_from_this<people>
{
public:
	people(people_type type, int tile_x, int tile_y, int surface_height, player owner);
	virtual ~people() {}
	std::vector<game_object*> draw(int screen_position_x, int screen_position_y);
	player show_owner() {return owner;}
	direction show_direction() {return movement_direction;}
	people_type show_type() {return type;}
	virtual void update() = 0;
	void draw_interface();
	void draw_partial_interface(int button_number);
	virtual void specific_draw_interface() {}
	virtual void damage(int damage, tile* attacker_position, bool ranged) = 0;
	int die();
	bool is_death() {return bIs_death;}
	int draw_life_bar(int screen_position_x, int screen_position_y);
	virtual void check_death() = 0;
	static bool general_can_move(tile* from, tile* to);
	virtual bool can_go_here(tile* t) = 0;
	virtual bool can_move(tile* from, tile* to) = 0;
	void rotate(int tile_x, int tile_y, bool clockwise);
	
	friend class boost::serialization::access;

	template <class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{	
		LOG("serializing people");
		
		using boost::serialization::make_nvp;

		ar & make_nvp("game_object", boost::serialization::base_object<game_object>(*this));
		ar & make_nvp("frames_to_move", frames_to_move);
		ar & make_nvp("frames_to_attack", frames_to_attack);
		ar & make_nvp("action_duration", action_duration);
		ar & make_nvp("max_action_duration", max_action_duration);
		ar & make_nvp("type", type);
		ar & make_nvp("previous_tile", previous_tile);
		ar & make_nvp("next_tile", next_tile);
		ar & make_nvp("moving_diagonally", moving_diagonally);
		ar & make_nvp("movement_direction", movement_direction);
		ar & make_nvp("movement_elevation", movement_elevation);
		ar & make_nvp("path", path);
		ar & make_nvp("owner", owner);
		ar & make_nvp("max_life", max_life);
		ar & make_nvp("life", life);
		ar & make_nvp("armor", armor);
		ar & make_nvp("attack", attack);
		ar & make_nvp("bIs_death", bIs_death);
		ar & make_nvp("bMoving", bMoving);
		ar & make_nvp("bAttacking", bAttacking);
		ar & make_nvp("hidden", hidden);
	}

protected:
	int minor_draw_people(int picture_begining, int drawing_x, int drawing_y, int allegro_flag);
	bool move_to_next_tile();
	static int height_difference_for_moving(tile* from, tile* to);
	int small_move();

	std::vector<game_object*> draw_catapult(int drawing_x, int drawing_y);
	
	int frames_to_move;
	int frames_to_attack;
	int action_duration;
	int max_action_duration;
	people_type type;
	tile* previous_tile;
	tile* next_tile;
	bool moving_diagonally;	//NORTH is diagonally, because its longer then NORTHEAST
	direction movement_direction;
	elevation movement_elevation;
	std::vector<tile*> path;
	player owner;
	int max_life;
	int life;
	int armor;
	int attack;
	bool bIs_death;
	bool bMoving;
	bool bAttacking;
	bool hidden;		//used for going to towers

	people() {}		//for boost::serialization	

};

class warrior : public people
{
public:
	warrior(people_type type, int tile_x, int tile_y, int surface_height, player owner);
	void update();
	void specific_draw_interface();
	
	void damage(int damage, tile* attacker_position, bool ranged);
	bool try_attack_tile(tile* to);
	
	bool find_path_to_target();
	void add_target(boost::shared_ptr<target> t);
	int show_max_target_priority();
	void check_death();	
	
	bool can_go_here(tile* t);
	bool can_move(tile* from, tile* to);
	bool can_attack_people(tile* from, tile* to);
	bool can_attack_building(tile* from, tile* to);

	void find_free_tile_near();
	bool has_target() {return current_target != nullptr;}
	target_priority show_target_priority();
	bool starving;
	
	friend class boost::serialization::access;

	template <class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{		
		using boost::serialization::make_nvp;

		ar & make_nvp("people", boost::serialization::base_object<people>(*this));
		ar & make_nvp("starving", starving);
		ar & make_nvp("target_number", target_number);
		ar & make_nvp("time_to_search_for_enemies", time_to_search_for_enemies);
		ar & make_nvp("next_tile_to_attack", next_tile_to_attack);
		ar & make_nvp("attacking_here", attacking_here);
		ar & make_nvp("current_target", current_target);
		ar & make_nvp("regeneration_time", regeneration_time);
		ar & make_nvp("is_ranged", is_ranged);
		ar & make_nvp("range", range);
	}


protected:
	bool can_attack_ranged(tile* from, tile* to);

	void check_targets(); 		//zajisti aby byl v targets prvni target s nejvyssi prioritou a aby byl platny, pokud se zmeni
	bool try_fullfill_target(boost::shared_ptr<target> target_to_fullfill);	//vola se pokud se zmenil target - nastavi spravne path, pripadne tile_to_attack
	void clear_target();
	void find_near_enemies();	//can generates new target
	void execute_attack();
	void regenerate();
	
	int target_number;
	
	int time_to_search_for_enemies;
	const int interval_between_enemies_search = 20;
	tile* next_tile_to_attack;
	tile* attacking_here;
	boost::shared_ptr<target> current_target;

	int regeneration_time;
	const int max_regeneration_time = game_info::fps;	//every second

	bool is_ranged;
	int range;
	const int archers_range = 5;
	const int spearman_range = 3;
	const int catapult_range = 8;

	warrior() {}		//for boost::serialization

};

class carrier : public people
{
public:
	carrier(people_type type, int tile_x, int tile_y, int surface_height, player owner, boost::shared_ptr<building> home_building); 
	void update();
	void specific_draw_interface();
	void give_task(resources resource_type, int amount, transaction_type type, boost::shared_ptr<building> target, std::vector<tile*> path_to_target);
	void damage(int damage, tile* attacker_position, bool ranged);
	
	bool can_go_here(tile* t);
	bool can_move(tile* from, tile* to);
	static bool static_can_go_here(tile* t);		//doesnt take enemies in account
	static bool static_can_move(tile* from, tile* to);	//doesnt take enemies in account
	
	friend class boost::serialization::access;

	template <class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		using boost::serialization::make_nvp;

		ar & make_nvp("people", boost::serialization::base_object<people>(*this));
		ar & make_nvp("home_building", home_building);
		ar & make_nvp("target", target);
		ar & make_nvp("transaction_size", transaction_size);
		ar & make_nvp("resource_carried", resource_carried);
		ar & make_nvp("waiting", waiting);
		ar & make_nvp("type_of_transaction", type_of_transaction);
	}


protected:
	bool find_path_to_target();
	bool is_target_adjacent();
	void check_target();			//check if home_building or target exist. if not, carrier dies or finds new target
	void target_reached();
	void check_death() {check_target();}

	boost::weak_ptr<building> home_building;
	boost::weak_ptr<building> target;
	int transaction_size;
	resources resource_carried;
	bool waiting;
	transaction_type type_of_transaction;

	carrier() {} 		//for boost::serialization	

};

#endif
