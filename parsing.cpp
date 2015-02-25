
#include "core.h"

void object_info::load(const std::string & value, const std::string & variable)
{
	if(variable == "name")
		name = value;

	else if(variable == "text")
		text = value;

	else if(variable == "life")
		life = std::stoi(value);

	else if(variable == "armor")
		armor = std::stoi(value);
}

void object_info::parse_file(std::ifstream & file)
{
	while(!file.eof())
	{
		std::string line;
		getline(file, line);
		std::string variable, value;

		size_t equality_index = line.find('=');
		if(equality_index == std::string::npos)
			continue;

		variable = line.substr(0, equality_index);
		value = line.substr(equality_index + 1);

		remove_spaces_and_apostrophs(variable);
		remove_spaces_and_apostrophs(value);

		this->load(value, variable);
	}
}

template <typename INFO_TYPE, typename DATABASE_TYPE, typename ENUM_TYPE>
std::vector<INFO_TYPE> object_info::load_info(int number_of_enums, std::string folder_name)
{
	std::vector<INFO_TYPE> info_vector;
	INFO_TYPE info;
	std::string file_name;
	for(int i=0; i<number_of_enums; ++i)
	{
		try
		{
			std::string name_from_database = std::get<1>(find<DATABASE_TYPE, 0>(static_cast<ENUM_TYPE>(i)));
			file_name = folder_name + name_from_database + ".conf";
		}
		catch (std::exception ex)
		{
			LOG("");
			throw ex;
		}
	
		std::ifstream file(file_name, std::ios::in);
		if(!file)
		{
			LOG("error - cant open file " + file_name);
			throw new std::exception;
		}
		
		INFO_TYPE info;
		
		info.image = std::get<2>(find<DATABASE_TYPE, 0>(static_cast<ENUM_TYPE>(i)));

		info.parse_file(file);
		info_vector.push_back(info);
	}
	return info_vector;
}

void building_info::load(const std::string & value, const std::string & variable)
{
	object_info::load(value, variable);

	if(variable == "building_wood")
		building_price[WOOD] = std::stoi(value);

	else if(variable == "building_stone")
		building_price[STONE] = std::stoi(value);

	else if(variable == "building_marble")
		building_price[MARBLE] = std::stoi(value);

	else if(variable == "building_bricks")
		building_price[BRICKS] = std::stoi(value);
	
	
	if(variable == "first_wood")				//first upgrade
		first_upgrade_price[WOOD] = std::stoi(value);

	else if(variable == "first_stone")
		first_upgrade_price[STONE] = std::stoi(value);

	else if(variable == "first_marble")
		first_upgrade_price[MARBLE] = std::stoi(value);

	else if(variable == "first_bricks")
		first_upgrade_price[BRICKS] = std::stoi(value);
	
	
	if(variable == "second_wood")				//second upgrade
		second_upgrade_price[WOOD] = std::stoi(value);

	else if(variable == "second_stone")
		second_upgrade_price[STONE] = std::stoi(value);

	else if(variable == "second_marble")
		second_upgrade_price[MARBLE] = std::stoi(value);

	else if(variable == "second_bricks")
		second_upgrade_price[BRICKS] = std::stoi(value);


	if(variable == "third_wood")				//third_upgrade
		third_upgrade_price[WOOD] = std::stoi(value);

	else if(variable == "third_stone")
		third_upgrade_price[STONE] = std::stoi(value);

	else if(variable == "third_marble")
		third_upgrade_price[MARBLE] = std::stoi(value);

	else if(variable == "third_bricks")
		third_upgrade_price[BRICKS] = std::stoi(value);

	else if(variable == "height_of_life_bar")
		height_of_life_bar = std::stoi(value);

	else if(variable == "number_of_floors")
		number_of_floors = std::stoi(value);

	else if(variable == "honour_price")
		honour_price = std::stoi(value);

	else if(variable == "upgrade_info")
		upgrade_info = value;

	else if(variable == "number_of_carriers")
		number_of_carriers = std::stoi(value);

	else if(variable == "number_of_workers")
		number_of_workers = std::stoi(value);

	else if(variable == "capacity")
	{
		if(value == "infinity")
			capacity = std::numeric_limits<int>::max();
		
		else
			capacity = std::stoi(value);
	}

	else if(variable == "can_be_upgraded")
	{
		if((value == "true") || (value == "TRUE") || (value == "True") || (value == "1"))
		{
			can_be_upgraded = true;
		}
		else
		{
			can_be_upgraded = false;
		}
	}

	else if(variable == "size")
	{
		if(value == "ONE_TILE_BUILDING")
			size = ONE_TILE_BUILDING;

		else if(value == "FOUR_TILE_BUILDING")
			size = FOUR_TILE_BUILDING;

		else if(value == "NINE_TILE_BUILDING")
			size = NINE_TILE_BUILDING;

		else if(value == "LEFT_GATE_BUILDING")
			size = LEFT_GATE_BUILDING;
		
		else
		{
			LOG("error in parsing - wrong size");
			throw new std::exception;
		}
	}
}

building_info& building_info::show_building_info(building_type type)
{
	static std::vector<building_info> info = load_info<building_info, building_names, building_type>(NUMBER_OF_BUILDINGS, "data/buildings/");
	return info[static_cast<int>(type)];
}

void people_info::load(const std::string & value, const std::string & variable)
{
	object_info::load(value, variable);

	if(variable == "attack")
		attack = std::stoi(value);

	else if(variable == "weapons")
		price[WEAPONS] = std::stoi(value);

	else if(variable == "bows")
		price[BOWS] = std::stoi(value);

	else if(variable == "plate_armour")
		price[PLATE_ARMOR] = std::stoi(value);

	else if(variable == "frames_to_move")
		frames_to_move = std::stoi(value);
	
	else if(variable == "frames_to_attack")
		frames_to_attack = std::stoi(value);

	else if(variable == "honour_price")
		honour_price = std::stoi(value);
}

people_info& people_info::show_people_info(people_type type)
{
	static std::vector<people_info> info = load_info<people_info, people_names, people_type>(NUMBER_OF_PEOPLE, "data/people/");
	return info[static_cast<int>(type)];
}

