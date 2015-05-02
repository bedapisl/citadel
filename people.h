#ifndef people_h
#define people_h

#include "core.h"

#define CARRIER_CAPACITY 1

class tile;
class chosen_mouse;

struct A_star_tiles;

/**
 * \brief Represents target that some unit want achieve. 
 */
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
		ar & make_nvp("priority", priority);
	}

	game_object_type type;
	tile* tile_target;				///< Where the target is.
	boost::weak_ptr<people> people_target;		///< If target is people, pointer to the concrete units.
	boost::weak_ptr<building> building_target;	///< If target is building, pointer to the concrete building.
	target_priority priority;			///< From 0 to 10. 10 is highest priority.

private:
	target() {};		//for boost::serialization

};

/**
 * \brief Represents units.
 */
class people : public game_object, public boost::enable_shared_from_this<people>
{
public:
	people(people_type type, int tile_x, int tile_y, int surface_height, player owner);
	virtual ~people() {}
	std::vector<game_object*> draw(int screen_position_x, int screen_position_y);	///< Draws this. Returns empty vector.
	player show_owner() {return owner;}
	direction show_direction() {return movement_direction;}
	people_type show_type() {return type;}
	virtual void update() = 0;				///< Manages people. Should be called once each frame. Performs movement, attacks, ....
	void draw_interface();					///< Draws stats of this unit to panel. Called if one unit is selected.
	void draw_partial_interface(int button_number);		///< Draws image and health of this unit to panel. Called if more units are selected.
	virtual void damage(int damage, tile* attacker_position, bool ranged) = 0;	///< Handles attack on this unit.
	int die();						///< Makes unit dead. 
	bool is_death() {return bIs_death;}		
	void draw_life_bar(int screen_position_x, int screen_position_y);	///< Draws how much life unit has. Called if unit is selected or mouse points to it.
	virtual void check_death() = 0;				///< Removes pointers to dead objects from internal structures.
	static bool general_can_move(tile* from, tile* to);	///< Returns if it is possible to go from one tile to another. Doesn't take in account people.
	virtual bool can_go_here(tile* t) = 0;			///< Returns if it is possible to go on given tile.
	virtual bool can_move(tile* from, tile* to) = 0;	///< Returns if it is possible to go from one tile to another.
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
	virtual void specific_draw_interface() {}
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
	std::vector<tile*> path;	///< Where the unit plans to go. Last tile is goal tile.
	player owner;
	int max_life;
	int life;
	int armor;
	int attack;
	bool bIs_death;
	bool bMoving;
	bool bAttacking;
	bool hidden;		///< People are hidden if they are currently going to tower - they are inside tower.

	people() {}		//for boost::serialization	

};

/**
 * \brief Represents soldiers.
 */
class warrior : public people
{
public:
	warrior(people_type type, int tile_x, int tile_y, int surface_height, player owner);
	void update();	 
	
	void damage(int damage, tile* attacker_position, bool ranged);		///< Handles attack on this warrior.
	
	void add_target(boost::shared_ptr<target> t);			///< If new target has higher priority than old one, it will become active target.
	int show_max_target_priority();
	void check_death();				///< Removes invalid pointers and pointers to dead objects from internal warrior structures.
	
	bool can_go_here(tile* t);			///< Returns whether this warrior can go to given tile.
	bool can_move(tile* from, tile* to);		///< Returns whether this warrior can move from one tile to another in one step.
	static bool static_can_go_here(tile* t);	///< Returns whether some warrior can go to given tile. Doesn't take in account people.
	static bool static_can_move(tile* from, tile* to);	///< Returns whether some warrior can from one tile to another. Doesn't take in account people.
	bool can_attack_people(tile* from, tile* to);	///< Returns whether this warrior can attack people on "to" tile from "from" tile.
	bool can_attack_building(tile* from, tile* to);	///< Returns whether this warrior can attack building on "to" tile from "from" tile.

	void find_free_tile_near();			///< Sets last tile of path to be an accessible tile near warrior location.
	bool has_target() {return current_target != nullptr;}
	target_priority show_target_priority();		
	bool starving;					///< Indicates whether this warrior received food from it's barracks.
	
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
	bool find_path_to_target();
	void specific_draw_interface();
	bool try_attack_tile(tile* to);
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

/**
 * \brief Represents unit which carry resources between buildings.
 */
class carrier : public people
{
public:
	carrier(people_type type, int tile_x, int tile_y, int surface_height, player owner, boost::shared_ptr<building> home_building); 
	void update();
	void specific_draw_interface();
	void give_task(resources resource_type, int amount, transaction_type type, boost::shared_ptr<building> target, std::vector<tile*> path_to_target);
				///< Gives this carrier task. Should be called only on idle carriers.
	void damage(int damage, tile* attacker_position, bool ranged);	///< Handles attacks on this carrier.
	
	bool can_go_here(tile* t);				///< Returns whether carrier can go to given tile.		
	bool can_move(tile* from, tile* to);			///< Returns whether carrier can go from one tile to another in one step.
	static bool static_can_go_here(tile* t);		///< Returns whether carrier can go to given tile if it is empty.
	static bool static_can_move(tile* from, tile* to);	///< Returns whether carrier can go from one tile to another in one step if the goal tile is empty.
	
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
