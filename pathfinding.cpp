#include "pathfinding.h"

std::vector<tile*> pathfinding::adjacent_tiles(tile* t, bool move_diagonally) 
{
	std::vector<tile*> adjacent;
	if(move_diagonally)
	{
		for(int y = std::max(t->show_tile_y() - 1, 0); y <= std::min(t->show_tile_y() + 1, game_info::map_height - 1); ++y)
		{
			for(int x = std::max(t->show_tile_x() - 1, 0); x <= std::min(t->show_tile_x() + 1, game_info::map_width - 1); ++x)
			{
				if((y != t->show_tile_y()) || (x != t->show_tile_x()))
				{
					adjacent.push_back(session->tile_list[y][x].get());
				}
			}
		}
	}
	else
	{
		int base_x = t->show_tile_x();
		int base_y = t->show_tile_y();
		std::vector<std::pair<int, int>> coordinates{std::pair<int, int>(base_x+1, base_y), std::pair<int, int>(base_x-1, base_y), 
							std::pair<int, int>(base_x, base_y+1), std::pair<int, int>(base_x, base_y-1)};
		for(int i=0; i<coordinates.size(); ++i)
		{
			int x = coordinates[i].first;
			int y = coordinates[i].second;
			
			if((x >= 0) && (x < game_info::map_width) && (y >= 0) && (y < game_info::map_height))
			{
				adjacent.push_back(session->tile_list[y][x].get());
			}
		}
	}
	return adjacent;
}

std::vector<tile*> pathfinding::near_accessible_tiles(tile* middle_tile, int number_of_tiles)
{
	int max_distance = sqrt(number_of_tiles)/2;
	int distance = 0;

	std::vector<tile*> tiles;
	tiles.push_back(middle_tile);
	int explored_index = 0;
	int distance_increase = 1;
	bool done = false;

	while(!done)
	{
		for(int i=0; i<tiles[explored_index]->accessible_neighbours.size(); ++i)
		{	
			tile* t = tiles[explored_index]->accessible_neighbours[i];
			if(!contains(tiles, t) && (t->building_on_tile.expired() || t->can_go_inside_building))
			{
				tiles.push_back(t);
				if(tiles.size() == number_of_tiles)
				{
					done = true;
					break;
				}
			}
		}
		explored_index++;
		if(explored_index == distance_increase)
		{
			if(distance_increase == max_distance)
			{
				done = true;
			}
			else
			{
				distance_increase = tiles.size();
				++distance;
				if(distance > max_distance)
					done = true;
			}
		}
		if(explored_index == tiles.size())
			done = true;
	}
	
	if(tiles.size() < number_of_tiles)
	{
		int founded = tiles.size();
		
		while(tiles.size() < number_of_tiles)
		{
			tiles.push_back(tiles[rand() % founded]);
		}
	}
	return tiles;
}
	



