/**
 * \file session.h
 * \brief Contains \ref game_session class which represents one game.
 */

#ifndef session_h
#define session_h

#include "core.h"

#define TIME_TO_UPDATE_HAPPINESS (2*game_info::fps)

/**
 * \brief Contains all data about one game session and manages global ingame events
 * The active instance of this class can be accessed through "session" global variable. 
 */

class game_session{
public:
	game_session(std::vector<int> starting_resources, int starting_honour, int enemies, std::vector<int> natural_resources_amount, int mountains_amount);
	void update(game_mouse* mouse, bool& done);		///< Updates and manages this class. Should be called once each frame.
	int show_happiness() {return happiness;}
	int show_frames_from_start() {return frames_from_start;}	///< Frames the moment when first warehouse was built.
	int frames_to_invasion() {return time_to_invasion;}		///< How much time lefts before enemies will attack.
	void finish_serialization();	
	int show_invasion_number() {return invasion_number;}

	std::vector<std::vector<boost::shared_ptr<tile>>> tile_list;	///< This is the game map. First index to tile_list is tile_y coordinate, second is tile_x coordinate.
	std::vector<boost::shared_ptr<building>> building_list;		///< List of all real buildings in the game.
	std::vector<boost::shared_ptr<people>> people_list;		///< List of all people in the game.
	std::vector<boost::shared_ptr<missile>> missile_list;		///< List of all missiles.
	std::vector<bool> unlocked_buildings;				///< Indicates whether given building is unlocked.
	boost::shared_ptr<carrier_output> global_stock;			///< Global resources storage - all warehouses are using this.
	graphical_texts hints;						///< Texts which appers on the map.
	enemy_ai ai;							
	int honour;	
	bool game_started; 		///< Indicates whether warehouse was built.

	friend class boost::serialization::access;
	
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		using boost::serialization::make_nvp;
		ar & make_nvp("tile_list", tile_list);
		ar & make_nvp("building_list", building_list);
		ar & make_nvp("people_list", people_list);
		ar & make_nvp("missile_list", missile_list);
		ar & make_nvp("unlocked_buildings", unlocked_buildings);
		ar & make_nvp("global_stock", global_stock);
		ar & make_nvp("ai", ai);
		ar & make_nvp("honour", honour);
		ar & make_nvp("game_started", game_started);
		ar & make_nvp("frames_from_start", frames_from_start);
		ar & make_nvp("happiness", happiness);
		ar & make_nvp("starving", starving);
		ar & make_nvp("invasion_number", invasion_number);
		ar & make_nvp("time_to_invasion", time_to_invasion);
		ar & make_nvp("invasion_interval", invasion_interval);
	}

private:
	game_session() {}
	int update_happiness();
	int invasion();

	static std::vector<bool> set_unlocked_buildings();
	static std::vector<bool> set_unlocked_warehouse();
	
	int frames_from_start;
	int happiness;		//0 - 100
	bool starving; 
	int invasion_number;
	int time_to_invasion;
	int invasion_interval;
};

#endif
