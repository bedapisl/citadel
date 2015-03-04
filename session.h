#ifndef session_h
#define session_h

#include "core.h"

#define TIME_TO_UPDATE_HAPPINESS (2*game_info::fps)
//#define INVASION_TIME (200*game_info::fps)
//#define FIRST_INVASION (300*game_info::fps)
#define STARTING_WOOD 400
#define STARTING_STONE 300
#define STARTING_MARBLE 50
#define STARTING_BRICKS 0

class game_session{
public:
	game_session(std::vector<int> starting_resources, int starting_honour, int enemies, std::vector<int> natural_resources_amount, int mountains_amount);
	int update(game_mouse* mouse, bool& done);
	int update_happiness();
	int show_happiness() {return happiness;}
	int invasion();
	int show_frames_from_start() {return frames_from_start;}
	int frames_to_invasion() {return time_to_invasion;}
	void finish_serialization();
	
	std::vector<std::vector<boost::shared_ptr<tile>>> tile_list;
	std::vector<boost::shared_ptr<building>> building_list;
	std::vector<boost::shared_ptr<people>> people_list;
	std::vector<boost::shared_ptr<missile>> missile_list;
	std::vector<bool> unlocked_buildings;
	boost::shared_ptr<carrier_output> global_stock;
	graphical_texts hints;
	enemy_ai ai;
	int honour;
	bool game_started; 		//this will be set true after warehouse is built

	friend class boost::serialization::access;
	
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & tile_list;
		ar & building_list;
		ar & people_list;
		ar & missile_list;
		ar & unlocked_buildings;
		ar & global_stock;
		ar & ai;
		ar & honour;
		ar & game_started;
		ar & frames_from_start;
		ar & happiness;
		ar & starving;
		ar & invasion_number;
		ar & time_to_invasion;
		ar & invasion_interval;
	}

private:
	game_session() {}

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
