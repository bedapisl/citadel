#ifndef ai_h_bedas_guard
#define ai_h_bedas_guard

#include "core.h"

class enemy_ai
{
public:
	void update(); 
	void register_units();		//finds RED_PLAYER's warriors
	void check_death_units();
	void unit_attacked(tile* from);

	friend class boost::serialization::access;

	template <class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		using boost::serialization::make_nvp;

		ar & make_nvp("units", units);
		ar & make_nvp("main_target", main_target);
	}
		
protected:
	std::vector<tile*> weak_spots();
	std::vector<std::vector<tile*>> accessible_buildings(tile* start);
	
	std::vector<boost::weak_ptr<warrior>> units;

	boost::weak_ptr<building> main_target;
};

#endif

