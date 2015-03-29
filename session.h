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
