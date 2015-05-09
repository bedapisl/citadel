
/**
 * \file constant_database.h
 * \brief Contains templates for general constant database definition and all constant database instances.
 */

#ifndef CONSTANT_DATABASE_BEDAS
#define CONSTANT_DATABASE_BEDAS

#include <vector>
#include <map>
#include <tuple>
#include <utility>
#include <stdexcept>

class constant_database_exception : public std::runtime_error
{
public:
	constant_database_exception(const std::string& message) : std::runtime_error(message) {};
};
/**
 * \brief Base class for constant databases.
 * Shouldn't be used directly. Examples of constant databases are lower in this file.
 */
template <typename derived_database, typename Tuple>
class base_database
{
private:
	
	/**
	 * Returns map of pairs with "idx"-th element of tuple and index to vector of tuples. 
	 * This map is the used to search for tuples by "idx"-th element.
	 */
	template <int idx>
	static const std::multimap<typename std::tuple_element<idx, Tuple>::type, std::size_t> & get_map()
	{
		static std::multimap<typename std::tuple_element<idx, Tuple>::type, std::size_t> m;	
			//key in map is idx-th element in tuple, value is index in "std::vector<Tuple> base_database::tuples"

		static bool initialized = false;
		if(initialized)
			return m;

		initialized = true;
		init_vector();

		for(std::size_t i=0; i<tuples.size(); ++i)	//initialization of map
		{
			std::pair<typename std::tuple_element<idx, Tuple>::type, std::size_t> tuple_with_key(std::get<idx>(tuples[i]), i);
			m.insert(tuple_with_key);
		}
		return m;
	}

	static std::vector<Tuple> tuples;		///< Data in database.
	
	/// Initialize data (std::vector<Tuples> base_database::tuples) in database.
	static void init_vector()	
	{
		static bool initialized = false;
		if(initialized)
			return;

		initialized = true;
		derived_database::init();
		return;
	}
	
	/// \brief Returns tuple from database "db" which has "key" as "idx"-th element.
	/// Example: Given database "colors" with following tuples: t1("blue", "sky"), t2("black", "night")
	/// find<colors, 0>("blue") returns t1, find<colors, 1>("sky") also returns t1, but find<colors, 1>("blue") throws "no key found".
	template <typename db, int idx>
	friend const typename db::value_type &find(const typename std::tuple_element<idx, typename db::value_type>::type & key);

protected:
	/// Function used by derived databases to load data in database.
	template <typename ... TList>	
	static void insert(TList ... parametres)
	{
		tuples.push_back(std::make_tuple(parametres ...));
	}
};

template <typename db, typename Tuple>
std::vector<Tuple> base_database<db, Tuple>::tuples;

/**
 * \brief Function to find data in some constant database.
 * \tparam db Name of database instance, which should be searched.
 * \tparam idx Index of element of tuples in database which will be the key that is searched for.
 * 
 * Returns tuples from database "db" with "key" as "idx"-th element of the tuple.
 * Throws exception if no tuple is found or more than one tuple is found.
 *
 * Example: 
 * We have database called building_names with building_types (enum) and building names and we want to get the name given the type.
 * \code
 *		tuple<building_type, std::string> t = find<building_names, 0>(HOUSE);
 * 		std::string name = get<1>(t);
 * \endcode
 */
template <class db, int idx>
const typename db::value_type &find(const typename std::tuple_element<idx, typename db::value_type>::type & key)
{
	const std::multimap<typename std::tuple_element<idx, typename db::value_type>::type, std::size_t> & map_of_tuples 
		= base_database<db, typename db::value_type>::template get_map<idx>();

	auto it_lower = map_of_tuples.lower_bound(key);
	auto it_upper = map_of_tuples.upper_bound(key);
	if(it_lower == it_upper)
		throw constant_database_exception("no key found");

	it_upper--;
	if(it_lower != it_upper)
		throw constant_database_exception("multiple keys found");	

	return base_database<db,typename db::value_type>::tuples[(*it_lower).second];
}

/**
 * \brief Constant database with buildings names as strings, enums and names of their images.
 */
struct building_names: public base_database<building_names, std::tuple<building_type, std::string, picture>>
{
	typedef std::tuple<building_type, std::string, picture> value_type;

	static void init()
	{
		insert(HOUSE, "house", HOUSE_IMAGE);
		insert(MARKET, "market", MARKET_IMAGE);
		insert(WAREHOUSE, "warehouse", WAREHOUSE_IMAGE);
		insert(GREAT_HALL, "great_hall", GREAT_HALL_IMAGE);
		insert(CHURCH, "church", CHURCH_IMAGE);
		insert(STORE, "store", STORE_IMAGE);
		insert(BARRACKS, "barracks", BARRACKS_IMAGE);
		insert(SCOUT, "scout", SCOUT_IMAGE);
		insert(QUARRY, "quarry", QUARRY_IMAGE);
		insert(WOODCUTTER, "woodcutter", WOODCUTTER_IMAGE);
		insert(HUNTER, "hunter", HUNTER_IMAGE);
		insert(FISHERMAN, "fisherman", FISHERMAN_IMAGE);
		insert(APPLE_FARM, "apple_farm", APPLE_FARM_IMAGE);
		insert(DAIRY_FARM, "dairy_farm", DAIRY_FARM_IMAGE);
		insert(WHEAT_FARM, "wheat_farm", WHEAT_FARM_IMAGE);
		insert(WINDMILL, "windmill", WINDMILL_IMAGE);
		insert(CLAY_PIT, "clay_pit", CLAY_PIT_IMAGE);
		insert(POTTERY_WORKSHOP, "pottery_workshop", POTTERY_WORKSHOP_IMAGE);
		insert(BRICKMAKER, "brickmaker", BRICKMAKER_IMAGE);
		insert(MARBLE_QUARRY, "marble_quarry", MARBLE_QUARRY_IMAGE);
		insert(GOLD_MINE, "gold_mine", GOLD_MINE_IMAGE);
		insert(COAL_MINE, "coal_mine", COAL_MINE_IMAGE);
		insert(IRON_MINE, "iron_mine", IRON_MINE_IMAGE);
		insert(SMITH, "smith", SMITH_IMAGE);
		insert(ARMOURER, "armourer", ARMOURER_IMAGE);
		insert(FLETCHER, "fletcher", FLETCHER_IMAGE);
		insert(PALISADE, "palisade", PALISADE_IMAGE);
		insert(WALL, "wall", WALL_IMAGE);
		insert(NORTHWEST_TOWER, "tower", TOWER_IMAGE);
		insert(NORTHEAST_TOWER, "tower", TOWER_IMAGE);
		insert(SOUTHEAST_TOWER, "tower", TOWER_IMAGE);
		insert(SOUTHWEST_TOWER, "tower", TOWER_IMAGE);
		insert(NORTHWEST_STAIRS, "stairs", STAIRS_IMAGE);
		insert(NORTHEAST_STAIRS, "stairs", STAIRS_IMAGE);
		insert(SOUTHEAST_STAIRS, "stairs", STAIRS_IMAGE);
		insert(SOUTHWEST_STAIRS, "stairs", STAIRS_IMAGE);
		insert(LEFT_GATE, "gate", GATE_IMAGE);
		insert(RIGHT_GATE, "gate", GATE_IMAGE);
	}
};

/**
 * \brief Constant database for people names as strings, enums and names of their images.
 */
struct people_names : public base_database<people_names, std::tuple<people_type, std::string, picture>>
{
	typedef std::tuple<people_type, std::string, picture> value_type;

	static void init()
	{
		insert(AXEMAN, "axeman", AXEMAN_IMAGE);
		insert(BOWMAN, "bowman", BOWMAN_IMAGE);
		insert(SPEARMAN, "spearman", SPEARMAN_IMAGE);
		insert(SWORDSMAN, "swordsman", SWORDSMAN_IMAGE);
		insert(CATAPULT, "catapult", CATAPULT_IMAGE);
		insert(CARRIER, "carrier", CARRIER_IMAGE);
	}
};

/**
 * \brief Constant database which matches resources names as enums to strings.
 */

struct resources_names : public base_database<resources_names, std::tuple<resources, std::string>>
{
	typedef std::tuple<resources, std::string> value_type;

	static void init()
	{
		insert(WOOD, "wood");
		insert(STONE, "stone");
		insert(MARBLE, "marble");
		insert(BRICKS, "bricks");
		insert(MILK, "milk");
		insert(FISH, "fish");
		insert(MEAT, "meat");
		insert(FRUIT, "fruit");
		insert(CLAY, "clay");
		insert(POTTERY, "pottery");
		insert(IRON, "iron");
		insert(WEAPONS, "weapons");
		insert(BOWS, "bows");
		insert(COAL, "coal");
		insert(PLATE_ARMOR, "plate armor");
		insert(WHEAT, "wheat ");
		insert(BREAD, "bread");
		insert(GOLD, "gold");
	}
};


/**
 * \brief Constant database which matches output of building::can_build_here() function to texts which will be shown to player.
 */
struct hint_database : public base_database<hint_database, std::tuple<can_build_output, std::string>>
{
	typedef std::tuple<can_build_output, std::string> value_type;

	static void init()
	{
		insert(can_build_output::CAN_BUILD, "");
		insert(can_build_output::MISSING_WOOD, "Not enough wood");
		insert(can_build_output::MISSING_STONE, "Not enough stone");
		insert(can_build_output::MISSING_BRICKS, "Not enough bricks");
		insert(can_build_output::MISSING_MARBLE, "Not enough marble");
		insert(can_build_output::NO_TREES, "No trees nearby");
		insert(can_build_output::NO_WATER, "No water nearby");
		insert(can_build_output::NO_FERTILE_LAND, "No fertile land nearby");
		insert(can_build_output::NO_IRON, "No iron nearby");
		insert(can_build_output::NO_COAL, "No coal nearby");
		insert(can_build_output::NO_MARBLE, "No marble nearby");
		insert(can_build_output::NO_GOLD, "No gold nearby");
		insert(can_build_output::NO_ROCKS, "No rocks nearby");
		insert(can_build_output::NO_SPACE, "Not enough space");
		insert(can_build_output::ENEMIES_NEARBY, "Enemies nearby");
	}
};
	
#endif
