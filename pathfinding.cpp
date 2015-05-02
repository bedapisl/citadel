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






