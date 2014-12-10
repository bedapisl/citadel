#ifndef pathfinding_h
#define pathfinding_h

#include "core.h"

class tile;
class game_session;

extern game_session* session;

class pathfinding
{
private:
	template <typename TILE_TYPE>
	static std::vector<tile*> find_path_back(const std::vector<std::vector<TILE_TYPE>>& map, tile* goal_tile);

	struct search_tile;
	struct a_star_tile;
	class comparator_functor;

public:
	class single_tile_goal_functor;
	class single_tile_heuristic_functor;
	class multiple_tile_goal_functor;
	class unknown_heuristic_functor;
	class single_building_goal_functor;
	class any_building_goal_functor;
	class any_enemy_goal_functor;
	class normal_real_distance_functor;
	class carrier_real_distance_functor;

	static std::vector<tile*> adjacent_tiles(tile* t, bool move_diagonally);
	
	template <typename ACCESSIBLE, typename GOAL>
	static std::vector<std::vector<tile*>> breadth_first_search(std::vector<tile*> starting_tiles, ACCESSIBLE accessible_function, GOAL goal_function,
						bool move_diagonally = true, bool check_goal_accessible = true, int max_distance = std::numeric_limits<int>::max());

	template <typename ACCESSIBLE, typename GOAL, typename HEURISTIC, typename REAL_DISTANCE>
	static std::vector<tile*> a_star(std::vector<tile*> starting_tiles, ACCESSIBLE accessible_function, GOAL goal_function, HEURISTIC heuristic_function, REAL_DISTANCE real_distance_function, bool move_diagonally = true);

	static std::vector<tile*> near_accessible_tiles(tile* middle_tile, int number_of_tiles);
};

struct pathfinding::search_tile
{
public:
	search_tile() : explored(false), previous_tile(nullptr) {}
	bool explored;
	tile* previous_tile;
};

template <typename TILE_TYPE>
std::vector<tile*> pathfinding::find_path_back(const std::vector<std::vector<TILE_TYPE>>& map, tile* goal_tile)
{
	TILE_TYPE t = map[goal_tile->show_tile_y()][goal_tile->show_tile_x()];
	std::vector<tile*> result;
	tile* current_tile = goal_tile;

	while(t.previous_tile != NULL)
	{
		result.push_back(current_tile);
		current_tile = t.previous_tile;
		t = map[current_tile->show_tile_y()][current_tile->show_tile_x()];
	}
	
	reverse(result.begin(), result.end());
	return result;
}

template <typename ACCESSIBLE, typename GOAL>
std::vector<std::vector<tile*>> pathfinding::breadth_first_search(std::vector<tile*> starting_tiles, ACCESSIBLE accessible_function, GOAL goal_function, 
						bool move_diagonally, bool check_goal_accessible, int max_distance)
{
	std::vector<std::vector<tile*>> result;

	std::vector<std::vector<search_tile>> map(game_info::map_height, std::vector<search_tile>(game_info::map_width));

	std::vector<tile*> frontier = starting_tiles;
	std::vector<tile*> new_frontier;
	for(size_t i=0; i<frontier.size(); ++i)
	{
		map[frontier[i]->show_tile_y()][frontier[i]->show_tile_x()].explored = true;
	}
	
	int distance = 0;

	while(distance < max_distance)
	{
		distance++;
		for(size_t i=0; i<frontier.size(); ++i)
		{
			std::vector<tile*> adjacent = adjacent_tiles(frontier[i], move_diagonally);
			for(size_t j=0; j<adjacent.size(); ++j)
			{
				search_tile& adjacent_tile = map[adjacent[j]->show_tile_y()][adjacent[j]->show_tile_x()];
				if(!adjacent_tile.explored)
				{
					if(accessible_function(frontier[i], adjacent[j]))
					{
						adjacent_tile.explored = true;
						adjacent_tile.previous_tile = frontier[i];
						new_frontier.push_back(adjacent[j]);
					}

					if(goal_function(adjacent[j]))
					{
						if((check_goal_accessible && people::general_can_move(frontier[i], adjacent[j])) || (!check_goal_accessible))
						{
							adjacent_tile.previous_tile = frontier[i];
							result.push_back(find_path_back(map, adjacent[j]));
						}
					}
				}
			}
		}
		if(new_frontier.size() == 0)
			break;

		frontier.clear();
		frontier.swap(new_frontier);
	}

	return result;
}


struct pathfinding::a_star_tile
{
public:
	a_star_tile() : explored(false), previous_tile(nullptr), distance(0.0) {}
	bool explored;
	tile* previous_tile;
	double distance;
};

class pathfinding::comparator_functor
{
public:
	bool operator()(std::pair<double, tile*> a, std::pair<double, tile*> b)
	{
		return a.first > b.first;
	}
};

class pathfinding::single_tile_goal_functor
{
public:
	single_tile_goal_functor(tile* goal_tile) : goal_tile(goal_tile) {}
	bool operator()(tile* t) {return t == goal_tile;}

private:
	tile* goal_tile;
};

class pathfinding::single_tile_heuristic_functor
{
public:
	single_tile_heuristic_functor(tile* goal_tile) : goal_tile(goal_tile) {}
	double operator() (tile* t) {return sqrt(pow(goal_tile->show_tile_x() - t->show_tile_x(), 2) + pow(goal_tile->show_tile_y() - t->show_tile_y(), 2));}

private:
	tile* goal_tile;
};

class pathfinding::multiple_tile_goal_functor
{
public:
	multiple_tile_goal_functor(std::vector<tile*> goal_tiles) : goal_tiles(goal_tiles) {}
	bool operator() (tile* t) {return contains(goal_tiles, t);}
private:
	std::vector<tile*> goal_tiles;
};

class pathfinding::unknown_heuristic_functor
{
public:	
	double operator() (tile* t) {return 0.0;}
};

class pathfinding::single_building_goal_functor	
{
public:
	single_building_goal_functor(tile* goal_tile, bool move_diagonally)
	{
		if(goal_tile->building_on_tile.expired())
			throw new std::exception;

		boost::shared_ptr<building> goal_building = goal_tile->building_on_tile.lock();
		std::vector<tile*> building_tiles = tiles_under_building(goal_building->show_tile_x(), goal_building->show_tile_y(), goal_building->show_size());
		for(size_t i=0; i<building_tiles.size(); ++i)
		{
			std::vector<tile*> adjacent = adjacent_tiles(building_tiles[i], move_diagonally);
			for(size_t j=0; j<adjacent.size(); ++j)
			{
				if((!contains(building_tiles, adjacent[j])) && (!contains(adjacent_to_goal, adjacent[j])) 
						&& people::general_can_move(adjacent[j], building_tiles[i]))
				{
					adjacent_to_goal.push_back(adjacent[j]);
				}
			}
		}
	}
	bool operator()(tile* t) {return contains(adjacent_to_goal, t);}

private:
	std::vector<tile*> adjacent_to_goal;
};

class pathfinding::any_building_goal_functor
{
public: 
	bool operator() (tile* t)
	{
		if(!t->building_on_tile.expired())
		{
			int id = t->building_on_tile.lock()->id;
			if(!contains(founded_buildings_ids, id))
			{
				founded_buildings_ids.push_back(id);
				return true;
			}
		}
		return false;
	}

private:
	std::vector<int> founded_buildings_ids;
};

class pathfinding::any_enemy_goal_functor
{
public:
	any_enemy_goal_functor(boost::shared_ptr<warrior> warrior_ptr) : warrior_ptr(warrior_ptr) {}
	bool operator() (tile* t)
	{
		std::vector<tile*> adjacent = adjacent_tiles(t, true);
		for(size_t i=0; i<adjacent.size(); ++i)
		{
			adjacent[i]->check_death_people_on_tile();
			if(adjacent[i]->people_on_tile.size() > 0)
			{
				if(warrior_ptr->can_attack_people(t, adjacent[i]))
				{
					if(adjacent[i]->people_on_tile[0].lock()->show_owner() != warrior_ptr->show_owner())
						return true;
				}
			}
			if(!adjacent[i]->building_on_tile.expired())
			{	
				if(warrior_ptr->can_attack_building(t, adjacent[i]))
				{
					if(adjacent[i]->building_on_tile.lock()->show_owner() != warrior_ptr->show_owner())
					{
						return true;
					}
				}
			}
		}
		return false;
	}
private:
	boost::shared_ptr<warrior> warrior_ptr;
};

class pathfinding::normal_real_distance_functor
{
public:
	double operator() (tile* from, tile* to)
	{
		return sqrt(abs(from->show_tile_x() - to->show_tile_x()) + abs(from->show_tile_y() - to->show_tile_y()));
	}
};

class pathfinding::carrier_real_distance_functor
{
public:
	double operator() (tile* from, tile* to)
	{
		to->check_death_people_on_tile();
		int carriers_in_path = 0;
		for(size_t i=0; i<to->people_on_tile.size(); ++i)
		{
			if(to->people_on_tile[i].lock()->show_type() == CARRIER)
				carriers_in_path++;
		}
		return (1 + carriers_in_path);		//carriers cant move diagonally, so real distance is always 1
	}
};

template <typename ACCESSIBLE, typename GOAL, typename HEURISTIC, typename REAL_DISTANCE>
std::vector<tile*> pathfinding::a_star(std::vector<tile*> starting_tiles, ACCESSIBLE accessible_function, GOAL goal_function, HEURISTIC heuristic_function, REAL_DISTANCE real_distance_function, bool move_diagonally)
{
	std::vector<std::vector<a_star_tile>> map(game_info::map_height, std::vector<a_star_tile>(game_info::map_width));
	for(int i=0; i<starting_tiles.size(); ++i)
	{
		map[starting_tiles[i]->show_tile_y()][starting_tiles[i]->show_tile_x()].explored = true;
	}

	std::priority_queue<std::pair<double, tile*>, std::vector<std::pair<double, tile*>>, comparator_functor> frontier;
	for(int i=0; i<starting_tiles.size(); ++i)
	{
		frontier.push(std::pair<double, tile*>(heuristic_function(starting_tiles[i]), starting_tiles[i]));
	}

	while(frontier.size() > 0)
	{
		tile* best = frontier.top().second;
		std::vector<tile*> tiles_to_explore = adjacent_tiles(best, move_diagonally);
		for(size_t i=0; i<tiles_to_explore.size(); ++i)
		{
			a_star_tile& adjacent = map[tiles_to_explore[i]->show_tile_y()][tiles_to_explore[i]->show_tile_x()];
			if((!adjacent.explored) && (accessible_function(best, tiles_to_explore[i])))
			{
				adjacent.explored = true;
				adjacent.previous_tile = frontier.top().second;
				adjacent.distance = map[best->show_tile_y()][best->show_tile_x()].distance + real_distance_function(best, tiles_to_explore[i]);
				frontier.push(std::pair<double, tile*>(adjacent.distance + heuristic_function(tiles_to_explore[i]), tiles_to_explore[i]));

				if(goal_function(tiles_to_explore[i]))
				{
					return find_path_back(map, tiles_to_explore[i]);
				}
			}
		}
		frontier.pop();
	}
	return std::vector<tile*>();
}
	

#endif
