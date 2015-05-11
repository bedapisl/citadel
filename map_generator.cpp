#include "core.h"

map_generator::map_generator() : rand(0, std::numeric_limits<int>::max()), base_border(game_info::map_generator_base_tile), gauss_deviation(game_info::map_generator_gauss_deviation),
				base_ramp_probability(game_info::map_generator_base_ramp_probability), ramp_start(game_info::map_generator_ramp_start),
				ramp_end(game_info::map_generator_ramp_end)
{ }

/* Main generating function. In generated map are all tiles accessible from borders. Needs lots of time (normally about 0.5s but theoretically infinite amount).*/
std::vector<std::vector<boost::shared_ptr<tile>>> map_generator::generate(std::vector<int> resources, int mountains, int seed)
{
	rng = std::default_random_engine(seed);
	mountains_level = mountains;
	LOG("generating map");
	
	bool done = false;
	while(!done)
	{
		reset_map();				//clear map

		generate_surface(mountains * mountain_height_modifier);		//generate heights of tiles

		add_ramps();				//add some ramps
		
		for(int i = 0; i<game_info::map_height; i++)		
		{
			for(int j = 0; j<game_info::map_width; j++)
				map[i][j]->set_right_drawing(map);
		}

		make_accessible();			//can add ramps and change heights to make all tiles accessible
		
		done = check_accessible();
	}

	if(resource_amount_modifier.size() != resources.size())
		throw std::exception();
	
	std::vector<tile*> clumps_middles;
	for(int i=0; i<resources.size(); ++i)	//generate all resources
	{
		int resource_left = resource_amount_modifier[i] * resources[i];
		while(resource_left > 0)
		{
			std::normal_distribution<double> distribution(20.0, 5.0);

			int clump_size;
			if(resource_left > undivisible_clump_size)		//choose clump size
			{
				clump_size = std::max(0, std::min((int)(distribution(rng) + 0.5), resource_left));
			}
			else
				clump_size = resource_left;

			std::vector<tile*> clump = generate_clump(clump_size, clumps_middles);	//generate clump
			fill_clump(clump, static_cast<natural_resources>(i));	//fill clump with resource
			
			if(!check_accessible())					//if some tiles are inaccessible
			{
				clear_clump(clump);					//remove clump
				clumps_middles.pop_back();
			}
			else 
				resource_left -= clump.size();			
		}
	}
	
	for(int i = 0; i<game_info::map_height; i++)		
	{
		for(int j = 0; j<game_info::map_width; j++)
			map[i][j]->set_right_drawing(map);
	}

	for(int i = 0; i<game_info::map_height; i++)		
	{
		for(int j = 0; j<game_info::map_width; j++)
			map[i][j]->set_accessible_neighbours(map);
		
	}
	
	return map;
}

void map_generator::reset_map()
{
	map.clear();
	for(int i=0; i<game_info::map_height; ++i)
	{
		map.push_back(std::vector<boost::shared_ptr<tile>>());
		for(int j=0; j<game_info::map_width; ++j)
			map[i].push_back(boost::shared_ptr<tile>(new tile(GRASS, j, i, 0)));
	}
	tile::highest_surface = 0;
}

void map_generator::generate_surface(int height_of_middle_tile)
{
	int middle_x = game_info::map_width/2;
	int middle_y = game_info::map_height/2;
	int end_x = game_info::map_width - 1;
	int end_y = game_info::map_height - 1;
	
	set_base_square_height(middle_x, middle_y, height_of_middle_tile);

	surface_rectangle(0, 0, middle_x, middle_y);	
	surface_rectangle(middle_x, 0, end_x, middle_y);
	surface_rectangle(0, middle_y, middle_x, end_y);
	surface_rectangle(middle_x, middle_y, end_x, end_y);
}

///Corners of rectangle should be already set.
void map_generator::surface_rectangle(int start_x, int start_y, int end_x, int end_y)
{
	if((end_x - start_x <= (base_border * 2 + 1)) || (end_y - start_y <= (base_border * 2 + 1)))
	{	
		// image of situation: x and y axes, P1, P2, P3, P4 are already done. Their size is base_border.
		//	  -> x
		//   |	P1  B  P2 
		//   V	A   C  E 	
		//   y	P3  D  P4
		//
		//	We need to set height for A, B, C, D, E. For C new height is computed from P1, P2, P3, P4, 
		//	for others it is computed only from adjacent P

		int height = middle_tile_height(start_x, start_y, end_x, end_y);		//C
		set_rectangle_height(start_x + base_border, start_y + base_border, end_x - base_border, end_y - base_border, height);

		height = middle_tile_height(start_x, start_y, start_x, end_y);				//A
		set_rectangle_height(start_x, start_y + base_border, start_x + base_border, end_y - base_border, height);
		
		height = middle_tile_height(start_x, start_y, end_x, start_y);				//B
		set_rectangle_height(start_x + base_border, start_y, end_x - base_border, start_y + base_border, height);
	
		height = middle_tile_height(start_x, end_y, end_x, end_y);				//D
		set_rectangle_height(start_x + base_border, end_y - base_border, end_x - base_border, end_y, height);

		height = middle_tile_height(end_x, start_y, end_x, end_y);				//E
		set_rectangle_height(end_x - base_border, start_y + base_border, end_x, end_y - base_border, height);

		return;
	}

	int middle_x = (start_x + end_x)/2;
	int middle_y = (start_y + end_y)/2;
						//tiles between two other tiles
	set_base_square_height(start_x,middle_y, new_height(start_x, start_y, start_x, end_y));
	set_base_square_height(end_x, middle_y, new_height(end_x, start_y, end_x, end_y));
	set_base_square_height(middle_x, start_y, new_height(start_x, start_y, end_x, start_y));
	set_base_square_height(middle_x, end_y, new_height(start_x, end_y, end_x, end_y));
						
						//middle tile
	set_base_square_height(middle_x, middle_y, middle_tile_height(start_x, start_y, end_x, end_y));
						
						//recursion on subproblems
	surface_rectangle(start_x, start_y, middle_x, middle_y);
	surface_rectangle(start_x, middle_y, middle_x, end_y);
	surface_rectangle(middle_x, start_y, end_x, middle_y);
	surface_rectangle(middle_x, middle_y, end_x, end_y);
}

int map_generator::random_change(std::vector<tile*> tiles)
{
	static std::normal_distribution<double> gauss(0.0, gauss_deviation);

	double distance_modifier = std::abs(tiles[0]->show_tile_x() - tiles[1]->show_tile_x()) + std::abs(tiles[1]->show_tile_y() - tiles[1]->show_tile_y());
	distance_modifier = distance_modifier * distance_modifier * mountains_level / 100.0;

	double height = 0;
	for(int i=0; i<tiles.size(); ++i)
	{
		int h = tiles[i]->show_surface_height();
		height += h;
	}
	
	return std::max(height/(double)tiles.size() + gauss(rng)*distance_modifier + 0.5, 0.0);	//0.5 is because of rounding
}

int map_generator::new_height(int first_x, int first_y, int second_x, int second_y)
{
	std::vector<tile*> tiles{map[first_y][first_x].get(), map[second_y][second_x].get()};
	return random_change(tiles);
}

int map_generator::middle_tile_height(int first_x, int first_y, int second_x, int second_y)
{
	std::vector<tile*> tiles{
		map[first_y][first_x].get(),
		map[first_y][second_x].get(),
		map[second_y][first_x].get(),
		map[second_y][second_x].get()
	};

	return random_change(tiles);
}

void map_generator::set_base_square_height(int tile_x, int tile_y, int height)
{
	for(int x = tile_x - base_border; x <= tile_x + base_border; ++x)
	{
		for(int y = tile_y - base_border; y <= tile_y + base_border; ++y)
		{
			if((x >= 0) && (y >= 0) && (x < game_info::map_width) && (y < game_info::map_height))
			{
				map[y][x]->set_surface_height(height);
			}
		}
	}
}

void map_generator::set_rectangle_height(int start_x, int start_y, int end_x, int end_y, int height)
{
	for(int x = start_x; x < end_x; ++x)
	{
		for(int y = start_y; y < end_y; ++y)
		{
			if((x >= 0) && (y >= 0) && (x < game_info::map_width) && (y < game_info::map_height))
			{
				map[y][x]->set_surface_height(height);
			}
		}
	}
}

void map_generator::add_ramps_to_line(direction ramp_direction, int start_x, int start_y, int end_x, int end_y)
{
	std::vector<tile*> line;
	for(int x=start_x; x<=end_x; ++x)
	{
		for(int y=start_y; y<=end_y; ++y)
			line.push_back(map[y][x].get());
	}
	
	ramp_lines.push_back(line);
}

void map_generator::end_line(direction dir, int x, int y, int length, bool changing_x, bool longer_line)
{
	if(changing_x)
	{
		if(longer_line)
			add_ramps_to_line(dir, x - length, y, x, y);
		else
			add_ramps_to_line(dir, x - length, y, x - 1, y);
	}
	else
	{
		if(longer_line)
			add_ramps_to_line(dir, x, y - length, x, y);
		else
			add_ramps_to_line(dir, x, y - length, x, y - 1);
	}
}

void map_generator::check_line(int x, int y, int& length, int& height, bool& early_start, int& early_start_height, direction dir)
					//dir is ramp direction, line direction is different
{
	int tile_height = map[y][x]->show_surface_height();
	
	bool changing_x = ((dir == NORTHEAST) || (dir == SOUTHWEST));

	int under_tile_height;

	switch(dir)
	{
		case(NORTHEAST):
			under_tile_height = map[y-1][x]->show_surface_height();
			break;
		case(SOUTHEAST):
			under_tile_height = map[y][x+1]->show_surface_height();
			break;
		case(SOUTHWEST):
			under_tile_height = map[y+1][x]->show_surface_height();
			break;
		case(NORTHWEST):
			under_tile_height = map[y][x-1]->show_surface_height();
			break;
		default:
			throw std::exception();
	}

	bool same_heights = (tile_height == under_tile_height);
	bool longer_line = (same_heights && (tile_height == height));

	if(tile_height - 1 == under_tile_height)	//this tile makes valid ramp
	{
		if(height != tile_height)		//end of previous line
		{
			if(height != -1)
				end_line(dir, x, y, length, changing_x, longer_line);
			
			length = 1;
			height = tile_height;
		
			if((early_start) && (early_start_height == tile_height))
				length = 2;
		}
		else
			length++;
	}
	else
	{	
		if(height != -1)
			end_line(dir, x, y, length, changing_x, longer_line);
	
		height = -1;
	}

	early_start = same_heights;
	if(early_start)
		early_start_height = tile_height;
}

std::vector<std::vector<tile*>> map_generator::connect_lines()
{
	std::vector<tile*> line;
	std::vector<std::vector<tile*>> borders;

	while(!ramp_lines.empty())
	{
		line = ramp_lines.back();
		ramp_lines.pop_back();

		bool changing = true;
		while(changing)
		{
			changing = false;
			for(int i=0; i<ramp_lines.size(); ++i)
			{
				if(ramp_lines[i].front() == line.back())
				{
					changing = true;
					for(int j=1; j<ramp_lines[i].size(); ++j)
					{
						line.push_back(ramp_lines[i][j]);
					}
					ramp_lines.erase(ramp_lines.begin() + i);
				}
				else if(ramp_lines[i].back() == line.back())
				{
					changing = true;
					for(int j=ramp_lines[i].size() - 2; j >= 0; --j)
					{
						line.push_back(ramp_lines[i][j]);
					}
					ramp_lines.erase(ramp_lines.begin() + i);
				}
				else if(ramp_lines[i].front() == line.front())
				{
					changing = true;
					line.insert(line.begin(), ramp_lines[i].rbegin(), ramp_lines[i].rend() - 1);
					ramp_lines.erase(ramp_lines.begin() + i);
				}
				else if(ramp_lines[i].back() == line.front())
				{
					changing = true;
					line.insert(line.begin(), ramp_lines[i].begin(), ramp_lines[i].end() - 1);
					ramp_lines.erase(ramp_lines.begin() + i);
				}
			}
		}
		borders.push_back(line);
	}
	return borders;
}

void map_generator::generate_ramps(std::vector<std::vector<tile*>>& borders)
{
	static std::uniform_real_distribution<double> rand(0.0, 1.0);
	bool make_ramp = true;

	int min_time_to_change = 2;
	int time = min_time_to_change;
	for(std::vector<tile*> b : borders)
	{
		make_ramp = (rand(rng) <= base_ramp_probability);

		for(int i=0; i<b.size(); ++i)
		{
			if(time == 0)
			{
				if(make_ramp)
				{
					if(rand(rng) <= ramp_end)
					{
						make_ramp = false;
						time = min_time_to_change;
					}
				}
				else
				{
					if(rand(rng) <= ramp_start)
					{
						make_ramp = true;
						time = min_time_to_change;
					}
				}
			}
			else
				time--;
					
			if(make_ramp)
				b[i]->set_type(RAMP);
			else
				b[i]->set_type(GRASS);
		}
	}
}

void map_generator::add_ramps()
{
	for(int y=1; y<game_info::map_height - 1; ++y)
	{
		int northeast_length = 0;
		int northeast_height = -1;
		bool northeast_early_start = false;
		int northeast_early_height = -1;
		int southwest_length = 0;
		int southwest_height = -1;
		bool southwest_early_start = false;
		int southwest_early_height = -1;

		for(int x=0; x<game_info::map_width; ++x)
		{
			check_line(x, y, northeast_length, northeast_height, northeast_early_start, northeast_early_height, NORTHEAST);
			check_line(x, y, southwest_length, southwest_height, southwest_early_start, southwest_early_height, SOUTHWEST);
		}
	}

	for(int x=1; x<game_info::map_width - 1; ++x)
	{
		int northwest_length = 0;
		int northwest_height = -1;
		bool northwest_early_start = false;
		int northwest_early_height = -1;
		int southeast_length = 0;
		int southeast_height = -1;
		bool southeast_early_start = false;
		int southeast_early_height = -1;

		for(int y=0; y<game_info::map_height; ++y)
		{
			check_line(x, y, northwest_length, northwest_height, northwest_early_start, northwest_early_height, NORTHWEST);
			check_line(x, y, southeast_length, southeast_height, southeast_early_start, southeast_early_height, SOUTHEAST);
		}
	}

	std::vector<std::vector<tile*>> borders = connect_lines();
	generate_ramps(borders);
}	
						
std::vector<tile*> map_generator::generate_clump(int size, std::vector<tile*>& clumps_middles)
{
	static int clump_number = 0;
	++clump_number;

	LOG("clump: " << clump_number);

	std::vector<tile*> clump;
	std::vector<tile*> frontier;

	std::uniform_int_distribution<int> rand(0, std::numeric_limits<int>::max());
	
	tile* first_tile;

	bool ok = false;
						
	while(!ok)				//choose clump starting tile. we dont want clumps near borders of map or near each other
	{
		ok = true;
		int x = (rand(rng) % (game_info::map_height - 10)) + 5;
		int y = (rand(rng) % (game_info::map_width - 10)) + 5;
		
		for(int i=0; i<clumps_middles.size(); ++i)
		{
			int diff_x = x - clumps_middles[i]->show_tile_x();
			int diff_y = y - clumps_middles[i]->show_tile_y();
			if(std::sqrt(diff_x * diff_x + diff_y * diff_y) < 5.0)
			{
				ok = false;
			}
		}
		first_tile = map[y][x].get();
	}
	
	clumps_middles.push_back(first_tile);
	frontier.push_back(first_tile);	

	while((clump.size() < size) && (!frontier.empty()))
	{
		int index = rand(rng) % frontier.size();
		tile* t = frontier[index];
		frontier.erase(frontier.begin() + index);

		clump.push_back(t);
		std::vector<tile*> neighbours = pathfinding::accessible_neighbours(t, [] (tile* from, tile* to) 
					{return to->is_free() && !to->is_ramp() && !to->is_fertile();},
							false, map);
		
		for(tile* n : neighbours)
		{
			if((!contains(frontier, n)) && (!contains(clump, n)))
				frontier.push_back(n);
		}
	}
	return clump;
}

void map_generator::fill_clump(std::vector<tile*>& clump, natural_resources resource)
{
	for(tile* t : clump)
	{
		switch(resource)
		{
			case(natural_resources::WATER):
				t->add_object(WATER_TILE);
				break;
			case(natural_resources::WOOD):
				t->add_object(TREE_TILE);
				break;
			case(natural_resources::FERTILE_SOIL):
				t->add_fertile();
				break;
			case(natural_resources::IRON):
				t->add_object(IRON_TILE);
				break;
			case(natural_resources::COAL):
				t->add_object(COAL_TILE);
				break;
			case(natural_resources::MARBLE):
				t->add_object(MARBLE_TILE);
				break;
			case(natural_resources::GOLD):
				t->add_object(GOLD_TILE);
				break;
		}
	}
}

void map_generator::clear_clump(std::vector<tile*>& clump)
{
	for(tile* t : clump)
	{
		t->add_object(NOTHING);
	}
}

bool map_generator::check_accessible()
{	
	static std::vector<std::vector<bool>> explored(game_info::map_height, std::vector<bool>(game_info::map_width, false));
	
	bool return_value = true;

	std::vector<tile*> tiles = pathfinding::near_accessible_tiles(map[0][0].get(), 1000000000, 1000000000, warrior::static_can_move, true, map);
	for(tile* t : tiles)
	{
		explored[t->show_tile_y()][t->show_tile_x()] = true;
	}
	
	for(int x=0; x<game_info::map_width; ++x)
	{
		for(int y=0; y<game_info::map_height; ++y)
		{
			if(!explored[y][x] && (map[y][x]->object == NOTHING))
			{
				return_value = false;
				break;
			}
		}
		if(!return_value)
			break;
	}

	for(tile* t : tiles)		//reset static variable
	{
		explored[t->show_tile_y()][t->show_tile_x()] = false;
	}

	return return_value;
}


void map_generator::make_accessible()
{
	bool done = false;
	int last_x = -1, last_y = -1;

	while(!done)
	{
		done = true;
		std::vector<std::vector<bool>> explored(game_info::map_height, std::vector<bool>(game_info::map_width, false));
		
		std::vector<tile*> accessible = pathfinding::near_accessible_tiles(map[0][0].get(), std::numeric_limits<int>::max(), std::numeric_limits<int>::max(), 
							warrior::static_can_move, true, map);

		for(tile* t : accessible)
		{
			explored[t->show_tile_y()][t->show_tile_x()] = true;
		}

		for(int y=0; y<game_info::map_height; y++)
		{
			for(int x=0; x<game_info::map_width; x++)
			{
				if(!explored[y][x])
					//if tile is not explored I will change it's height such that it will be accessible from previous tile
				{	
					done = false;
					if((last_x == x) && (last_y == y))
					{
						LOG("bad map, generating new one");
						return;
					}
					
					last_x = x;
					last_y = y;

					tile* previous_tile;
					direction direction_from_previous;

					if(x == 0)
					{
						previous_tile = map[y-1][x].get();
						direction_from_previous = SOUTHWEST;
					}
					else			//x > 0 because algorithm started with [0][0], which must be explored 
					{
						previous_tile = map[y][x-1].get();
						direction_from_previous = SOUTHEAST;
					}

					tile* t = map[y][x].get();
					
					possible_borders borders = tile_borders(previous_tile, direction_from_previous);

					if(borders == NORMAL_BORDER)
					{
						if(t->show_surface_height() > previous_tile->show_surface_height())
						{
							t->set_surface_height(previous_tile->show_surface_height() + 1);
							t->set_type(RAMP);
						}
						else
						{
							t->set_surface_height(previous_tile->show_surface_height());
							t->set_type(GRASS);
						}
					}
					else if(borders == DOWN_BORDER)
					{	
						t->set_surface_height(previous_tile->show_surface_height() - 1);
						t->set_type(GRASS);
					}

					else if(borders == RAMP_BORDER)
					{
						t->set_surface_height(previous_tile->show_surface_height());
						t->set_type(RAMP);
					}
					
					for(int i=y-1; i<=y+1; i++)
					{
						for(int j=x-1; j<=x+1; j++)
						{
							if(((i >= 0) && (i < game_info::map_height)) && ((j >= 0) && (j < game_info::map_height)))
								map[i][j]->set_right_drawing(map);
						}
					}

					for(int i=y-1; i<=y+1; i++)
					{
						for(int j=x-1; j<=x+1; j++)
						{
							if(((i >= 0) && (i < game_info::map_height)) && ((j >= 0) && (j < game_info::map_height)))
								map[i][j]->set_accessible_neighbours(map);
						}
					}
					break;
				}
			}
			if(!done)
				break;
		}
	}
}

void rectangle_map_generator::reset_map()
{
	map.clear();
	for(int i=0; i<game_info::map_height; ++i)
	{
		map.push_back(std::vector<boost::shared_ptr<tile>>());
		for(int j=0; j<game_info::map_width; ++j)
			map[i].push_back(boost::shared_ptr<tile>(new tile(GRASS, j, i, 0)));
	}
}

/* Main generating function. In generated map are all tiles accessible from borders. Needs lots of time (normally about 0.5s but theoretically infinite amount).*/
std::vector<std::vector<boost::shared_ptr<tile>>> rectangle_map_generator::generate(std::vector<int> resources, int mountains)
{
	bool done = false;
	while(!done)
	{
		LOG("generating map");
		
		reset_map();

		generate_surface(mountains);

		generate_water(resources[static_cast<int>(natural_resources::WATER)]);
		generate_resource(TREE_TILE, 3*resources[static_cast<int>(natural_resources::WOOD)]);	//we need more forests
		generate_resource(IRON_TILE, resources[static_cast<int>(natural_resources::IRON)]);
		generate_resource(COAL_TILE, resources[static_cast<int>(natural_resources::COAL)]);
		generate_resource(MARBLE_TILE, resources[static_cast<int>(natural_resources::MARBLE)]);
		generate_resource(GOLD_TILE, resources[static_cast<int>(natural_resources::GOLD)]);
		generate_fertile(resources[static_cast<int>(natural_resources::FERTILE_SOIL)]);

		for(int i = 0; i<game_info::map_height; i++)		
		{
			for(int j = 0; j<game_info::map_width; j++)
				map[i][j]->set_right_drawing(map);
		}

		for(int i = 0; i<game_info::map_height; i++)		
		{
			for(int j = 0; j<game_info::map_width; j++)
				map[i][j]->set_accessible_neighbours(map);
			
		}
		done = check_map();
	}
	return map;
}

/*Generated tiles height of tiles and ramps.*/
void rectangle_map_generator::generate_surface(int mountains)
{

	int clenitost = 50*mountains;
	int max_height = 5*mountains;
	int hornatost = 2;

	for(int i = 0; i<clenitost; i++)
	{
		int tile_x1 = 1 + (rand() % (game_info::map_width - 2));
		int tile_y1 = 1 + (rand() % (game_info::map_height - 2));
		int tile_x2 = 1 + (rand() % (game_info::map_width - 2));
		int tile_y2 = 1 + (rand() % (game_info::map_height - 2));

		if(tile_y2 - tile_y1 < 3)		//negative or very small size
			continue;
		if(tile_x2 - tile_x1 < 3)
			continue;
	
		if(tile_x2 - tile_x1 > game_info::map_width/2)
		{
			tile_x1 += game_info::map_width/4;
			tile_x2 -= game_info::map_width/4;
		}
		
		if(tile_y2 - tile_y1 > game_info::map_height/2)
		{
			tile_y1 += game_info::map_height/4;
			tile_y2 -= game_info::map_height/4;
		}
		
		int height = max_height;
		for(int j = 0; j < hornatost; j++)
		{
			int random_number = rand() % max_height;
			if(random_number < height)
				height = random_number;
		}
		
		int max_height_under = 0;
		for(int y = tile_y1; y <= tile_y2; y++)
		{
			for(int x = tile_x1; x <= tile_x2; x++)
			{
				if(max_height_under < map[y][x]->show_surface_height())
				{
					max_height_under = map[y][x]->show_surface_height();
				}
			}
		}
		if(height > max_height_under)
			height = max_height_under + 1;
				
		for(int y = tile_y1; y <= tile_y2; y++)
		{
			for(int x = tile_x1; x <= tile_x2; x++)
			{
				if((((x == tile_x1) || (x == tile_x2)) || ((y == tile_y1) || (y == tile_y2))) && (height > 0))
				{
					map[y][x]->set_type(RAMP);
					map[y][x]->set_surface_height(height);
				}
				else
				{
					map[y][x]->set_type(GRASS);
					map[y][x]->set_surface_height(height);
				} 
			}
		}
	}
}

/* Adds one 5*5 block of tiles with stone, one with iron, one with trees. There is a chnace that blocks will be at same location thus one will overwrite another. Doesnt add anything if map is very small.*/ 
void rectangle_map_generator::generate_resource(object_on_tile ob, int amount)
{
	if((game_info::map_width < 10) || (game_info::map_height < 10))
	{
		LOG("cannot generate resources - map is too small");
		return;
	}

	for(int k=0; k<amount; ++k)
	{
		int resource_x = rand() % (game_info::map_width - 5);
		int resource_y = rand() % (game_info::map_height - 5);

		if((MAP_WIDTH > 10) & (MAP_HEIGHT > 10))		//add some trees
		{
			for(int i=resource_x; i<resource_x + 4; i++)
			{
				for(int j = resource_y; j<resource_y + 4; j++)
				{
					if(map[i][j]->show_type() == GRASS)
						map[i][j]->add_object(ob);
				}
			}
		}
	}
}

void rectangle_map_generator::generate_fertile(int amount)
{	
	for(int i=0; i<4*amount; ++i)
	{
		int x = (rand() % (game_info::map_width - 6));
		int y = (rand() % (game_info::map_width - 6));
		for(int i=0; i<25; ++i)
		{
			int x2 = rand() % 7 + x;
			int y2 = rand() % 7 + y;
			if((x2 >= 0) && (x2 < game_info::map_width) && (y2 >= 0) && (y2 < game_info::map_height))
			{
				if(map[y2][x2]->show_type() == GRASS)
					map[y2][x2]->add_fertile();
			}
		}
	}
}

void rectangle_map_generator::generate_water(int amount)
{
	for(int i=0; i<3*amount; ++i)
	{
		int x = rand() % game_info::map_width;
		int y = rand() % game_info::map_height;
		int new_x = x, new_y = y;
	
		int probability = 120;

		while(rand() % 100 <= probability)
		{
			map[y][x]->add_object(WATER_TILE);
			if(rand() % 2 == 1)
				new_x = x + (rand() % 3) - 1;
			
			else
				new_y = y + (rand() % 3) - 1;
			
			if((new_x >= 0) && (new_x < game_info::map_width))
				x = new_x;

			if((new_y >= 0) && (new_y < game_info::map_height))
				y = new_y;

			probability--;
		}
	}
}

//	Check if every tile is accessible from borders. If not, then makes them accessible. If it is too hard to make them accessible return false.  
bool rectangle_map_generator::check_map()
{	
	for(int y=0; y<game_info::map_height; y++)
	{
		for(int x=0; x<game_info::map_width; x++)
		{
			if(((y >= 0) && (y < game_info::map_height)) && ((x >= 0) && (x < game_info::map_width)))
				map[y][x]->set_right_drawing(map);
		}
	}
	
	for(int y=0; y<game_info::map_height; y++)
	{
		for(int x=0; x<game_info::map_width; x++)
		{
			if(((y >= 0) && (y < game_info::map_height)) && ((x >= 0) && (x < game_info::map_width)))
				map[y][x]->set_accessible_neighbours(map);
		}
	}

	std::vector<std::vector<bool>> explored(game_info::map_height, std::vector<bool>(game_info::map_width, false));
	
	explored = find_accessible(explored, map[0][0].get());

	for(int y=0; y<game_info::map_height; y++)
	{
		for(int x=0; x<game_info::map_width; x++)
		{
			if((!explored[y][x]) && (!map[y][x]->is_water_tile()))
				//if tile is not explored I will change it's height such that it will be accessible from previous tile
			{			
				tile* previous_tile;
				direction direction_from_previous;

				if(x == 0)
				{
					previous_tile = map[y-1][x].get();
					direction_from_previous = SOUTHWEST;
				}
				else			//x > 0 because algorithm started with [0][0], which must be explored 
				{
					previous_tile = map[y][x-1].get();
					direction_from_previous = SOUTHEAST;
				}

				tile* t = map[y][x].get();
				
				possible_borders borders = tile_borders(previous_tile, direction_from_previous);

				if(borders == NORMAL_BORDER)
				{
					if(t->show_surface_height() > previous_tile->show_surface_height())
						t->set_surface_height(previous_tile->show_surface_height() + 1);
					else
						t->set_surface_height(previous_tile->show_surface_height());
				}
				else if(borders == DOWN_BORDER)
					t->set_surface_height(previous_tile->show_surface_height() - 1);

				else if(borders == RAMP_BORDER)
					t->set_surface_height(previous_tile->show_surface_height());
				
				t->set_type(RAMP);

				for(int i=y-1; i<=y+1; i++)
				{
					for(int j=x-1; j<=x+1; j++)
					{
						if(((i >= 0) && (i < game_info::map_height)) && ((j >= 0) && (j < game_info::map_height)))
							map[i][j]->set_right_drawing(map);
					}
				}

				for(int i=y-1; i<=y+1; i++)
				{
					for(int j=x-1; j<=x+1; j++)
					{
						if(((i >= 0) && (i < game_info::map_height)) && ((j >= 0) && (j < game_info::map_height)))
							map[i][j]->set_accessible_neighbours(map);
					}
				}
				
				if(explored[previous_tile->show_tile_y()][previous_tile->show_tile_x()])
				{
					if(!previous_tile->is_water_tile())
						explored = find_accessible(explored, previous_tile);		//try if it is accessible from previous tile
				}
			}
		}
	}

	for(int i=0; i<game_info::map_height; ++i)
	{
		for(int j=0; j<game_info::map_width; ++j)
		{
			if((!explored[i][j]) && (!map[i][j]->is_water_tile()))	
				return false;
		}
	}
	return true;
}

/* Used only by check map function. Starts or continues breadth first search from start and remembers explored tiles. If cannot explore anything else stops and returns array with explored tiles.*/
std::vector<std::vector<bool>>& rectangle_map_generator::find_accessible(std::vector<std::vector<bool>> & explored, tile* start)
{
	std::vector<tile*> queue;

	int start_index = 0;

	queue.push_back(start);

	explored[start->show_tile_y()][start->show_tile_x()] = true;
	
	while(start_index < queue.size())		//breadth first search
	{
		for(int i=0; i<queue[start_index]->accessible_neighbours.size(); i++)
		{
			tile* next_tile = queue[start_index]->accessible_neighbours[i];
			
			if(!next_tile)
				LOG("WARNING - wrong accessible_neighbours");	

			if(!explored[next_tile->show_tile_y()][next_tile->show_tile_x()])
			{
				queue.push_back(next_tile);
				explored[next_tile->show_tile_y()][next_tile->show_tile_x()] = true;
			}
		}
		start_index++;
	}
	
	return explored;
}

