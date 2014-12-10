#ifndef parsing_h
#define parsing_h

#include "core.h"

extern ALLEGRO_BITMAP** image_list;

struct building_names;
struct people_names;

class object_info
{
public:
	std::string name;
	std::string text;
	picture image;
	int life;
	int armor;
	
	object_info() : name("Unknown"), text("No description") {}
	virtual void load(const std::string & value, const std::string & variable);
	void parse_file(std::ifstream & file);

protected:	
	
	template <typename INFO_TYPE, typename DATABASE_TYPE, typename ENUM_TYPE>
	static std::vector<INFO_TYPE> load_info(int number_of_enums, std::string folder);
};

class building_info : public object_info
{
public:
	building_info() : building_price(NUMBER_OF_RESOURCES, 0), first_upgrade_price(NUMBER_OF_RESOURCES, 0), second_upgrade_price(NUMBER_OF_RESOURCES, 0), 
						third_upgrade_price(NUMBER_OF_RESOURCES, 0), honour_price(0), number_of_floors(1), height_of_life_bar(128)  {}

	void load(const std::string & value, const std::string & variable);

	building_size size;
	std::vector<int> building_price;
	std::vector<int> first_upgrade_price;
	std::vector<int> second_upgrade_price;
	std::vector<int> third_upgrade_price;
	int honour_price;
	bool can_be_upgraded;
	std::string upgrade_info;
	int number_of_floors;
	int height_of_life_bar;
	int number_of_carriers;
	int number_of_workers;
	int capacity;

	static building_info& show_building_info(building_type type);

private:
	static std::vector<building_info> load_building_info();
};

class people_info : public object_info
{
public:
	people_info() : price(NUMBER_OF_RESOURCES, 0), honour_price(0) {}
	void load(const std::string & value, const std::string & variable);
	
	int attack;
	std::vector<int> price;
	int honour_price;
	int frames_to_move;
	int frames_to_attack;

	static people_info& show_people_info(people_type type);

private:
	static std::vector<people_info> load_people_info();
};

#endif


