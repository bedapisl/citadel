
/**
 * \file parsing.h
 * \brief Classess for parsing files describing buildings and peoples attributes.
 * Parsed files are in data/buildings and data/people
 */

#ifndef parsing_h
#define parsing_h

#include "core.h"

extern ALLEGRO_BITMAP** image_list;

struct building_names;
struct people_names;

/**
 * \brief Represents general informations from data files about some object type.
 */
class object_info
{
public:
	std::string name;	///< Name of the object.	
	std::string text;	///< Description of object.
	picture image;
	int life;
	int armor;
	double influence;	///< How much enemies want to be near given unit/building. Higher number means they want to be there.
	
	object_info() : name("Unknown"), text("No description") {}

protected:	
	
	virtual void load(const std::string & value, const std::string & variable);
	void parse_file(std::ifstream & file);
	template <typename INFO_TYPE, typename DATABASE_TYPE, typename ENUM_TYPE>
	static std::vector<INFO_TYPE> load_info(int number_of_enums, std::string folder);
	void remove_spaces_and_apostrophs(std::string& word);
};
/**
 * \brief Represents informations from data files about type of building.
 */
class building_info : public object_info
{
public:
	building_info() : building_price(NUMBER_OF_RESOURCES, 0), first_upgrade_price(NUMBER_OF_RESOURCES, 0), second_upgrade_price(NUMBER_OF_RESOURCES, 0), 
						third_upgrade_price(NUMBER_OF_RESOURCES, 0), honour_price(0), number_of_floors(1), height_of_life_bar(128)  {}


	building_size size;
	std::vector<int> building_price;
	std::vector<int> first_upgrade_price;
	std::vector<int> second_upgrade_price;
	std::vector<int> third_upgrade_price;
	int honour_price;
	bool can_be_upgraded;
	std::string upgrade_info;
	int number_of_floors;
	int height_of_life_bar;			///< How high the life bar should be drawn.
	int number_of_carriers;
	int number_of_workers;			
	int capacity;				///< Capacity of ingame resources (wood, stone, ...)

	static const building_info& show_building_info(building_type type);	///< Returns informations about given type of building.

private:
	void load(const std::string & value, const std::string & variable);
	static std::vector<building_info> load_building_info();
};

/**
 * \brief Represents informations from data files about some kind of people.
 */
class people_info : public object_info
{
public:
	people_info() : price(NUMBER_OF_RESOURCES, 0), honour_price(0) {}
	
	int attack;
	std::vector<int> price;		///< Price to train the unit. Indexed by resources enum.
	int honour_price;
	int frames_to_move;
	int frames_to_attack;

	static const people_info& show_people_info(people_type type);		///< Returns informations about given type of unit.

private:
	void load(const std::string & value, const std::string & variable);
	static std::vector<people_info> load_people_info();
};

#endif


