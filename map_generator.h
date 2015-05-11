
/**
 * \file map_generator.h
 * \brief New and old map generator.
 */

#ifndef map_generator_bedas_guard
#define map_generator_bedas_guard

#include "core.h"

/**
 * \brief Class for generating maps with "fractals" and clumps.
 * Some parameters of map generation are stored in data/config.txt.
 * They are loaded in class game_info and from there this class uses them.
 * Other parameters are chosen by player and are passed to the constructor.
 * With the same parametres and seed, generated maps should the same.
 * Generation take some time, mainly because all tiles of generated map should be accessible from the borders.
 * If generation take too much time try lowering number of mountains or resources, use bigger tiles (config.txt: map_generator_base_borders), 
 * or lower randomness (config.txt: map_generator_gauss_deviation)
 */
class map_generator
{
public:
	map_generator();
	std::vector<std::vector<boost::shared_ptr<tile>>> generate(std::vector<int> resources, int mountains, int seed);	///< Generates new random map.

protected:
	void reset_map();
		
		//Surface generation
	void generate_surface(int mountains);		///< Sets tile height of tiles in map. Expects all heights to be 0.
	void surface_rectangle(int start_x, int start_y, int end_x, int end_y);		///< Sets tile heights in rectangle.
	int random_change(std::vector<tile*> tiles); 					///< Returns mean of heights of tiles plus/minus some random change.
	int new_height(int first_x, int first_y, int second_x, int second_y);		///< Returns new height of tile between first and second tile.
	int middle_tile_height(int first_x, int first_y, int second_x, int second_y);	///< Returns new height of tile in middle of rectangle.
	void set_base_square_height(int tile_x, int tile_y, int height);		///< sets height of all tiles in square which size is 2*base_border + 1
	void set_rectangle_height(int start_x, int start_y, int end_x, int end_y, int height);	///< Sets heights of tiles in rectangle.
	
		//Ramps generation
		//Ramp lines are straight lines of tiles where it is possible to add ramp.
	void add_ramps_to_line(direction ramp_direction, int start_x, int start_y, int end_x, int end_y);///< Sets tiles on line to be ramps with some probability.
	void end_line(direction dir, int x, int y, int length, bool changing_x, bool longer_line);	///< Handles founded end of line of ramps.
	void check_line(int x, int y, int& length, int& height, bool& early_start, int& early_start_height, direction dir);	///< Checks tile if it belongs to some line of ramps.
	std::vector<std::vector<tile*>> connect_lines();			///< Connect adjacent ramp lines.
	void generate_ramps(std::vector<std::vector<tile*>>& borders);		///< Add ramps to some parts of borders. 
	void add_ramps();							///< Main ramp generating function. Add ramps to random places in map.
		
		//Resources generation
	std::vector<tile*> generate_clump(int size, std::vector<tile*>& clumps_middles);			///< Generates clump of given size. 
	void fill_clump(std::vector<tile*>& clump, natural_resources resource); ///< Fill tiles in clump with given resource.
	void clear_clump(std::vector<tile*>& clump);				///< Sets object on all tiles of clump as nothing.
	
	bool check_accessible();			///< Returns whether all tiles of map are accessible from map[0][0]
	void make_accessible();				///< Makes accessible all tiles. Expect map without resources.

	std::vector<std::vector<boost::shared_ptr<tile>>> map;	///< Map generated so far.
	std::vector<std::vector<tile*>> ramp_lines;

	std::default_random_engine rng;
	std::uniform_int_distribution<int> rand;

	const int base_border;			///0 is only one tile, 1 means 3x3, 2 means 5x5
	const double gauss_deviation;		///< How irregular terrain will be. Normal values are about 0.005 - 0.05
	
	const double base_ramp_probability;	///< Probability first tile of border is ramp
	const double ramp_start;		///< Probability of ramp given previous tile is not ramp
	const double ramp_end;			///< Probability of cliff given previous tile is ramp
	
	const int  mountain_height_modifier = 1;	//Base mountain height is set up by player's option "mountains" in random game. 
							//This is it's modifier. Higher means higer mountains.
	int mountains_level;				///< Value of "mountains" option in random_game_setting
				
				//for WATER, WOOD, FERTILE_SOIL, IRON, COAL, MARBLE, GOLD in this order
	std::vector<int> resource_amount_modifier{30, 50, 50, 15, 5, 15, 5};	///< Amount of tiles with each resource multiplier.
	const int undivisible_clump_size = 10;		///< Soft rule for smallest clump size. Majority of clumps will be bigger.
};

/**
 * \brief Deprecated map generator.
 */
class rectangle_map_generator
{
public:
	std::vector<std::vector<boost::shared_ptr<tile>>> generate(std::vector<int> resources, int mountains);

protected:
	void reset_map();
	void generate_surface(int mountains);
	void generate_resource(object_on_tile ob, int amount);
	void generate_fertile(int amount);
	void generate_water(int amount);
	bool check_map();
	std::vector<std::vector<bool>>& find_accessible(std::vector<std::vector<bool>> & explored, tile* start);

	std::vector<std::vector<boost::shared_ptr<tile>>> map;
};

#endif




