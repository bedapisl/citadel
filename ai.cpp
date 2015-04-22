#include "core.h"

extern std::ofstream log_file;
extern game_session* session;

void enemy_ai::update()
{
	std::vector<boost::shared_ptr<warrior>> waiting_units;
	
	check_death_units();
	for(size_t i=0; i<units.size(); ++i)
	{
		if(!units[i].lock()->has_target())
		{
			waiting_units.push_back(units[i].lock());
		}
	}
	if(waiting_units.empty())
		return;


	if(main_target.expired())
	{
		choose_main_target();
	}

	boost::shared_ptr<target> t(new target(main_target, target_priority::AI_BUILDING_TARGET));
	
	for(size_t i=0; i<waiting_units.size(); ++i)
		waiting_units[i]->add_target(t);
}

void enemy_ai::register_units()
{
	units.clear();

	for(size_t i=0; i<session->people_list.size(); ++i)
	{
		if(session->people_list[i]->show_owner() == RED_PLAYER)
			units.push_back(boost::dynamic_pointer_cast<warrior>(session->people_list[i]));
	}
}

void enemy_ai::check_death_units()
{
	auto it = remove_if(units.begin(), units.end(), [](boost::weak_ptr<warrior> ptr) {return ptr.expired();});
	units.erase(it, units.end());
}

void enemy_ai::unit_attacked(tile* from)
{
	check_death_units();
	if(!from->are_people_on_tile())
		return;

	boost::weak_ptr<people> aggressor = from->people_on_tile[0];
	for(size_t i=0; i<units.size(); ++i)
	{	
		if(abs(units[i].lock()->show_tile_x() - from->show_tile_x()) + abs(units[i].lock()->show_tile_y() - from->show_tile_y()) < 10)
		{
			if(units[i].lock()->show_target_priority() < target_priority::FRIENDLY_UNIT_ATTACKED)
			{
				units[i].lock()->add_target(boost::shared_ptr<target>(new target(aggressor, target_priority::FRIENDLY_UNIT_ATTACKED)));
			}
		}
	}
}

void enemy_ai::choose_main_target()
{
	if(!main_target.expired())
		return;

	check_death_units();
	if(units.empty())
		return;

	std::vector<tile*> starting_tile(1, session->tile_list[units[0].lock()->show_tile_y()][units[0].lock()->show_tile_x()].get());
		
	std::vector<std::vector<tile*>> paths_to_buildings = pathfinding::breadth_first_search(starting_tile, warrior::static_can_move, pathfinding::any_building_goal_functor(), true, false);
						
	if(paths_to_buildings.size() == 0)	//first compute reacheble buildings
		throw std::exception();

	std::vector<std::vector<INFLUENCE_T>> map = generate_influence_map();
	INFLUENCE_T best = -100000;
	boost::weak_ptr<building> best_target;

	for(int i=0; i<paths_to_buildings.size(); ++i)
	{
		tile* t = paths_to_buildings[i].back();
		INFLUENCE_T inf = map[t->show_tile_y()][t->show_tile_x()];

		if(inf > best)
		{
			best = inf;
			best_target = t->building_on_tile;
		}
	}
	
	main_target = best_target;
}

std::vector<std::vector<enemy_ai::INFLUENCE_T>> enemy_ai::generate_influence_map()
{
	std::vector<std::vector<INFLUENCE_T>> positive_map(game_info::map_height, std::vector<INFLUENCE_T>(game_info::map_width, 0));
	std::vector<std::vector<INFLUENCE_T>> negative_map(game_info::map_height, std::vector<INFLUENCE_T>(game_info::map_width, 0));

	for(int i=0; i<session->building_list.size(); ++i)
	{
		boost::shared_ptr<building> b = session->building_list[i];
		INFLUENCE_T influence = building_info::show_building_info(b->show_type()).influence;

		std::vector<tile*> building_tiles = tiles_under_building(b->show_tile_x(), b->show_tile_y(), b->show_size());
		
		if(influence > 0)
			add_influence_source(building_tiles, influence, positive_map);
		else	
			add_influence_source(building_tiles, influence, negative_map);
	}

	for(int i=0; i<session->people_list.size(); ++i)
	{
		boost::shared_ptr<people> p = session->people_list[i];
		INFLUENCE_T influence = people_info::show_people_info(p->show_type()).influence;
		if(p->show_owner() == RED_PLAYER)
			influence = -influence;
	
		if(influence > 0)
			add_influence_source(std::vector<tile*>(1, session->tile_list[p->show_tile_y()][p->show_tile_x()].get()), influence, positive_map);
		else
			add_influence_source(std::vector<tile*>(1, session->tile_list[p->show_tile_y()][p->show_tile_x()].get()), influence, negative_map);
	}

	for(int i=0; i<positive_map.size(); ++i)
	{
		for(int j=0; j<positive_map[i].size(); ++j)
		{
			positive_map[i][j] += negative_map[i][j];
		}
	}

	return positive_map;
}

void enemy_ai::add_influence_source(std::vector<tile*> starting_tiles, INFLUENCE_T influence, std::vector<std::vector<INFLUENCE_T>>& map)
{
	bool positive = true;

	if(influence < 0)
	{
		positive = false;
		influence = -influence;
	}

	std::vector<std::vector<bool>> computed(map.size(), std::vector<bool>(map[0].size(), false));
	for(tile* t : starting_tiles)
	{
		computed[t->show_tile_y()][t->show_tile_x()] = true;
	}
		
	std::vector<tile*> frontier = starting_tiles;
	std::vector<tile*> next_frontier;

	INFLUENCE_T influence_change = 2;

	while(influence > 0)
	{
		for(tile* t : frontier)
		{
			if(positive)
				map[t->show_tile_y()][t->show_tile_x()] = std::max(influence, map[t->show_tile_y()][t->show_tile_x()]);
			else	
				map[t->show_tile_y()][t->show_tile_x()] = std::min( - influence, map[t->show_tile_y()][t->show_tile_x()]);

			std::vector<tile*> neighbours = t->accessible_neighbours;
			
			for(tile* n : neighbours)
			{
				if(warrior::static_can_move(t, n) && (!computed[n->show_tile_y()][n->show_tile_x()]))
				{
					computed[n->show_tile_y()][n->show_tile_x()] = true;
					next_frontier.push_back(n);
				}
			}
		}
		influence -= influence_change;
	}
}
	
	
		



