#ifndef ai_h
#define ai_h

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
		ar & units;
		ar & main_target;
	}
		
protected:
	std::vector<tile*> weak_spots();
	std::vector<std::vector<tile*>> accessible_buildings(tile* start);
	
	std::vector<boost::weak_ptr<warrior>> units;

	boost::weak_ptr<building> main_target;
};

#endif

