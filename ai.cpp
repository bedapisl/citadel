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
		std::vector<tile*> starting_tile(1, session->tile_list[waiting_units[0]->show_tile_y()][waiting_units[0]->show_tile_x()].get());
		
		std::vector<std::vector<tile*>> paths_to_buildings = pathfinding::breadth_first_search(starting_tile, [waiting_units](tile* from, tile* to){return waiting_units[0]->can_move(from, to);}, pathfinding::any_building_goal_functor(), true, false);
			
		if(paths_to_buildings.size() == 0)
			throw std::exception();

		int random_index = rand() % paths_to_buildings.size();
		
		main_target = paths_to_buildings[random_index].back()->building_on_tile;
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


/*
std::vector<tile*> enemy_ai::weak_spots()
{
	//najit accessible buildings tak, aby byla nebezpecnost cest co nejnizsi, abych spocital nebezpecnost cest, budu muset uvazovat i mozny pohyb nepratel
	//spocitat nebezpecnost utoku na kazdou budovu = nebezpecnost cesty + nebezpecnost okolnich budov
	//uvazit skody zpusobene znicenim budovy a pripadne uspesnost utoku na okolni policka



}
*/

/*
void enemy_ai::wait_tactis()				//wait for another invasion
{

}

void enemy_ai::directed_attack_tactics()			//one spot is chosen which all units try to destroy
{							//

}

void enemy_ai::frontal_attack_tactics()			//if RED_PLAYER has much more units		
{							//everyone attacking nearest player units

}

*/


