#ifndef ai_h_bedas_guard
#define ai_h_bedas_guard

#include "core.h"


/**
 * \brief Class for managing enemies high level behaviour.
 *
 */
class enemy_ai
{
public:
	void update();				///< Should be called once per frame, manages whole class.
	void register_units();			///< Registers all currently existing enemy units to be controled by this class. 
	void unit_attacked(tile* from);		///< Tells AI that AI's unit was attacked.

	friend class boost::serialization::access;

	template <class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		using boost::serialization::make_nvp;

		ar & make_nvp("units", units);
		ar & make_nvp("main_target", main_target);
	}
		
protected:
	void check_death_units();
	std::vector<tile*> weak_spots();
	std::vector<std::vector<tile*>> accessible_buildings(tile* start);
	
	std::vector<boost::weak_ptr<warrior>> units;

	boost::weak_ptr<building> main_target;
};

#endif

