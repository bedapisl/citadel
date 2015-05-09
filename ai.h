
/**
 * \file ai.h
 * \brief High level AI.
 * 
 * This file contains high level AI player behaviour which uses influence maps.
 * Low level AI is in people.h. 
 */

#ifndef ai_h_bedas_guard
#define ai_h_bedas_guard

#include "core.h"


/**
 * \brief Class for managing high level enemy behaviour.
 * Gives orders to idle enemy units, and chooses building which the enemies will try attack by influence maps.
 */
class enemy_ai
{
public:
	typedef int INFLUENCE_T;

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
	void choose_main_target();
	std::vector<std::vector<INFLUENCE_T>> generate_influence_map();	///< Higher influence means enemy wants to go there
	void add_influence_source(std::vector<tile*> starting_tiles, INFLUENCE_T influence, std::vector<std::vector<INFLUENCE_T>>& map); ///< Adds one source of influence (unit, building) to map
	void check_death_units();
	
	std::vector<boost::weak_ptr<warrior>> units;
	boost::weak_ptr<building> main_target;
};

#endif

