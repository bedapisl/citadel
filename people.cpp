#include "core.h"

extern std::ofstream log_file;
extern ALLEGRO_BITMAP** image_list;
extern game_session* session;
extern int display_width;
extern int display_height;
extern ALLEGRO_FONT* font15;

people::people(people_type type, int tile_x, int tile_y, int surface_height, player owner) 
			: game_object(tile_x, tile_y, surface_height, true, people_info::show_people_info(type).image, 1, PEOPLE)
{		

	people_info info = people_info::show_people_info(type);

	this->type = type;
	this->moving_diagonally = false;
	this->movement_direction = NO_DIRECTION;
	this->movement_elevation = NO_ELEVATION;
	this->action_duration = 0;
	this->max_action_duration = 0;
	this->owner = owner;
	this->bIs_death = false;
	this->bAttacking = false;
	this->bMoving = false;
	this->hidden = false;

	this->previous_tile = NULL;
	this->next_tile = NULL;
	this->path = std::vector<tile*>();
	
	this->life = info.life;
	this->attack = info.attack;
	this->armor = info.armor;
	this->frames_to_move = info.frames_to_move;
	this->frames_to_attack = info.frames_to_attack;

	this->max_life = life;


	if(!image)
	{	
		LOG("error failed to load people image");
		throw std::exception();
	}
}

std::vector<game_object*> people::draw(int screen_position_x, int screen_position_y)
{
	if(bIs_death)
	{	
		LOG("error - drawing dead people");
		return std::vector<game_object*>();
	}

	if(hidden)
		return std::vector<game_object*>();

	int drawing_x = game_x - screen_position_x - 32;
	int drawing_y = game_y - screen_position_y - 16;

	if(bMoving)
	{
		drawing_y -= 32 + 32*(previous_tile->show_effective_height() + 
					((double)(max_action_duration - action_duration)/(double)max_action_duration)
						* (next_tile->show_effective_height() - previous_tile->show_effective_height()));
	}
	else
	{
		drawing_y -= 32 + 32*session->tile_list[tile_y][tile_x]->show_effective_height();
	}

	int start_x = 0;		//what part of image will be drawn
	int start_y = 0;

	if(type == CATAPULT)
		return draw_catapult(drawing_x, drawing_y);

	if(!bAttacking)
	{
		if((movement_direction == SOUTH) || (movement_direction == SOUTHEAST) || (movement_direction == SOUTHWEST))
			start_x += TILE_WIDTH;

		else if((movement_direction == NORTH) || (movement_direction == NORTHEAST) || (movement_direction == NORTHWEST))
			start_x += 3*TILE_WIDTH;

		else if(movement_direction == EAST)
			start_x += 5*TILE_WIDTH;
		
		else if(movement_direction == WEST)
			start_x += 7*TILE_WIDTH;

		if(action_duration % 16 >= 8)
			start_x += TILE_WIDTH;		//second leg forward
	}
	else if(bAttacking)
	{
		start_y += 128;

		if((movement_direction == NORTH) || (movement_direction == NORTHWEST) || (movement_direction == WEST) || (movement_direction == SOUTHWEST))
			start_x += 4*TILE_WIDTH;	//left or right
	
		if((type == AXEMAN) || (type == SWORDSMAN))
		{
			if(action_duration < frames_to_attack/2)		//axe goes up
			{
				start_x += TILE_WIDTH*(int)(action_duration/((int)frames_to_attack/8));
			}
			else						//axe goes down
				start_x += TILE_WIDTH*(int)(((int)frames_to_attack - action_duration)/((int)frames_to_attack/8));
		}
		else if((type == BOWMAN) || (type == SPEARMAN))
		{
			start_x += TILE_WIDTH*(action_duration/((int)frames_to_attack/4));
		}
	}
		
	if(owner == RED_PLAYER)
		start_y += 64;

	al_draw_bitmap_region(image_list[image], start_x, start_y, 64, 64, drawing_x, drawing_y, 0);

	return std::vector<game_object*>();
}

void people::draw_interface()
{
	specific_draw_interface();
	people_info info = people_info::show_people_info(type);
	al_draw_text(font25, WRITING_COLOR, display_width / 2, display_height - BUTTON_SIZE, ALLEGRO_ALIGN_CENTRE, info.name.c_str());	//draw name

	al_draw_textf(font15, WRITING_COLOR, 10, display_height - BUTTON_SIZE, ALLEGRO_ALIGN_LEFT, "Health: %i/%i", life, max_life);
	al_draw_textf(font15, WRITING_COLOR, 10, display_height - BUTTON_SIZE + BUTTON_SIZE / 3, ALLEGRO_ALIGN_LEFT, "Attack: %i", info.attack);
	al_draw_textf(font15, WRITING_COLOR, 10, display_height - BUTTON_SIZE + 2 * BUTTON_SIZE / 3, ALLEGRO_ALIGN_LEFT, "Armor: %i", info.armor);
}

void people::draw_partial_interface(int button_number)
{
	al_draw_filled_rectangle(button_number * BUTTON_SIZE + 8, display_height - BUTTON_SIZE + 8, (button_number + 1)*BUTTON_SIZE - 8, display_height - 8, BLACK_COLOR);
	al_draw_bitmap_region(image_list[image], 0, 0, 64, 64, button_number*BUTTON_SIZE + 8, display_height - BUTTON_SIZE + 8, 0);

	button::draw_progress_bar(button_number * BUTTON_SIZE + 8, display_height - 10, (100*life)/max_life, 64, 6);
}

int people::draw_life_bar(int screen_position_x, int screen_position_y)
{
	if(hidden)
		return 0;

	int drawing_x = game_x - screen_position_x - 32;
	int drawing_y = game_y - screen_position_y - session->tile_list[tile_y][tile_x]->show_effective_height()*32 - 32;
	float health = (float)life/(float)max_life;
	al_draw_rectangle(drawing_x, drawing_y - 2, drawing_x + TILE_WIDTH, drawing_y + 3, GREY_COLOR, 1);  
	al_draw_filled_rectangle(drawing_x, drawing_y - 2, drawing_x + TILE_WIDTH*(health), drawing_y + 2, LIGHT_GREEN_COLOR);
	return 0;
}

/*Checks heights and ramps, not people*/
bool people::general_can_move(tile* from, tile* to)		
{
	if((abs(from->show_tile_x() - to->show_tile_x()) > 1) || (abs(from->show_tile_y() - to->show_tile_y()) > 1))
		return false;

	if((to->is_water_tile()) || (from->is_water_tile()))
		return false;

	direction way = compute_direction(from, to);
	direction way_back = compute_direction(to, from);
	
	possible_borders from_border = tile_borders(from, way);
	possible_borders to_border = tile_borders(to, way_back);
	
	int height_difference = height_difference_for_moving(from, to);
	
	switch(height_difference)
	{
		case(-2):
			return ((from_border == DOUBLE_DOWN_BORDER) && (to_border == NORMAL_BORDER));
			break;
		case(-1):
			return ((from_border == DOWN_BORDER) && (to_border == NORMAL_BORDER)) || ((from_border == DOUBLE_DOWN_BORDER) && (to_border == DOWN_BORDER));
			break;
		case(0):
			return ((from_border == NORMAL_BORDER) && (to_border == NORMAL_BORDER)) || ((from_border == DOWN_BORDER) && (to_border == DOWN_BORDER));
			break;
		case(1):
			return ((from_border == NORMAL_BORDER) && (to_border == DOWN_BORDER)) || ((from_border == DOWN_BORDER) && (to_border == DOUBLE_DOWN_BORDER));
			break;
		case(2):
			return ((from_border == NORMAL_BORDER) && (to_border == DOUBLE_DOWN_BORDER));
			break;
	}

	return false;
}

void people::rotate(int tile_x, int tile_y, bool clockwise)
{
	game_object::rotate(tile_x, tile_y, clockwise);
	if(bMoving)
		movement_direction = compute_direction(previous_tile, next_tile);
}

/*Tries to move to next tile in path. If it is not possible, returns -1.*/
bool people::move_to_next_tile()
{
	if(path.size() == 0)
	{
		LOG("error");
		return false;
	}
	
	tile* first_path = path[0];

	if(!can_move(session->tile_list[tile_y][tile_x].get(), first_path))
		return false;

	int height_difference = height_difference_for_moving(session->tile_list[tile_y][tile_x].get(), first_path);

	if(abs(height_difference) > 2)
		return false;
	
	if(height_difference == -2)		//moving down from tower
	{
		hidden = true;
		set_surface_height(show_surface_height() - 2);
	}
	
	if(height_difference == 2)
	{
		hidden = true;
		set_surface_height(show_surface_height() + 2);
	}

	if(height_difference == -1)
	{
		if((session->tile_list[tile_y][tile_x]->is_ramp()) || (session->tile_list[tile_y][tile_x]->stairs_on_tile))
		{
			movement_elevation = DOWN_ELEVATION;
			set_surface_height(show_surface_height() - 1);
		}
		else
		{
			LOG("error - move_to_next_tile - wrong path");
			movement_direction = NO_DIRECTION;
			movement_elevation = NO_ELEVATION;
			return false;
		}
	}

	else if(height_difference == 0)
		movement_elevation = NO_ELEVATION;

	else if(height_difference == 1)
	{
		if((first_path->is_ramp()) || first_path->stairs_on_tile)
		{
			movement_elevation = UP_ELEVATION;
			set_surface_height(show_surface_height() + 1);
		}
		else
		{
			LOG("error -  move_to_next_tile - wrong path");
			movement_direction = NO_DIRECTION;
			movement_elevation = NO_ELEVATION;
			return false;
		}
	}

	movement_direction = compute_direction(session->tile_list[tile_y][tile_x].get(), first_path); 
	
	bMoving = true;

	if(movement_direction % 2 == 0)
	{
		action_duration  = frames_to_move;		//TILE_WIDTH/8;
		moving_diagonally = false;
	}
	else if(movement_direction % 2 == 1)
	{
		action_duration = frames_to_move*sqrt(2.0);		//TILE_WIDTH/6; 	//1.41 ~ sqrt(2)
		moving_diagonally = true;
	}

	if(type == CARRIER)
	{
		first_path->check_death_people_on_tile();
		int carriers_in_path = 0;
		
		for(int i=0; i<first_path->people_on_tile.size(); ++i)
		{
			if(first_path->people_on_tile[i].lock()->show_type() == CARRIER)
			{
				carriers_in_path++;
			}
		}
		action_duration =  action_duration * (1 + carriers_in_path);
	}

	max_action_duration = action_duration;
	
	previous_tile = session->tile_list[tile_y][tile_x].get();
	next_tile = first_path;
	
	previous_tile->people_leave(shared_from_this());
	next_tile->people_come(shared_from_this());
	
	tile_x = next_tile->show_tile_x();
	tile_y = next_tile->show_tile_y();

	path.erase(path.begin());
	
	if(type != CARRIER)
	{
		warrior* p = dynamic_cast<warrior*>(this);
		p->find_free_tile_near();
	}	

	return true;
}
	
//return to->height - from_height, negative return value means that people move down
int people::height_difference_for_moving(tile* from, tile* to)
{
	std::vector<tile*> tiles{from, to};
	std::vector<int> heights;
	
	for(int i=0; i<tiles.size(); ++i)
	{	
		heights.push_back(tiles[i]->show_surface_height());
		if(!tiles[i]->building_on_tile.expired())
		{
			if(tiles[i]->can_go_inside_building)		//gate
				continue;
			
			else if(tiles[i]->can_go_on_building)		//walls or stairs
			{
				switch(tiles[i]->building_on_tile.lock()->type)
				{
					case(WALL):
					case(PALISADE):
					case(NORTHWEST_STAIRS):
					case(NORTHEAST_STAIRS):
					case(SOUTHEAST_STAIRS):
					case(SOUTHWEST_STAIRS):
						heights[i]++;
						break;
					case(NORTHWEST_TOWER):
					case(NORTHEAST_TOWER):
					case(SOUTHEAST_TOWER):
					case(SOUTHWEST_TOWER):
						heights[i] += 2;
						break;

					default:
						throw std::exception();
				}
			}
		}				
	}
	return heights[1] - heights[0];
}

/*Moves people to their right position between tiles.*/
int people::small_move()				//moving one tile diagonally (NORTH, EAST...) is 1.25 slower than moving normally (NORTHEAST...)
{
	int previous_game_x = previous_tile->show_game_x();
	int previous_game_y = previous_tile->show_game_y();
	int next_game_x = next_tile->show_game_x();
	int next_game_y = next_tile->show_game_y();
	
	game_x = previous_game_x + ((double)(max_action_duration - action_duration)/(double)max_action_duration)*(next_game_x - previous_game_x);
	game_y = previous_game_y + ((double)(max_action_duration - action_duration)/(double)max_action_duration)*(next_game_y - previous_game_y);
	return 0;
}

std::vector<game_object*> people::draw_catapult(int drawing_x, int drawing_y)
{
	int start_x = 0;
	int start_y = static_cast<int>(movement_direction) * 64;
	
	if(bAttacking)
		start_x = 64 * ((max_action_duration - action_duration)/(max_action_duration / 11));
	
	al_draw_bitmap_region(image_list[image], start_x, start_y, 64, 64, drawing_x, drawing_y, 0);
	return std::vector<game_object*>();
}

int people::die()
{
	if(owner == RED_PLAYER)
		LOG("owner is RED_PLAYER");

	else if(owner == BLUE_PLAYER)
		LOG("owner is BLUE_PLAYER");


	bIs_death = true;

	session->tile_list[tile_y][tile_x]->people_die();

	return 0;
}

warrior::warrior(people_type type, int tile_x, int tile_y, int surface_height, player owner) 
			: people(type, tile_x, tile_y, surface_height, owner)
{
	next_tile_to_attack = nullptr;
	attacking_here = nullptr;
	time_to_search_for_enemies = 0;
	regeneration_time = max_regeneration_time;
	starving = false;
	target_number = 0;

	switch(type)
	{
		case(AXEMAN):
		case(SWORDSMAN):
			is_ranged = false;
			break;
		case(BOWMAN):
		{
			range = archers_range;
			is_ranged = true;
		}
		break;
		case(SPEARMAN):
		{
			range = spearman_range;
			is_ranged = true;
		}
		break;
		case(CATAPULT):
		{
			range = catapult_range;
			is_ranged = true;
		}
		break;
		default:
			throw std::exception();
	}
	
}

void warrior::update()
{
	assert((bAttacking && (attacking_here != nullptr)) || (!bAttacking));

	if(bIs_death)
		return;

	regenerate();
	
	if(action_duration > 0)			//is in the middle of action
	{
		if(!bAttacking)
		{
			small_move();
		}
		else if(action_duration == max_action_duration/2)
		{
			execute_attack();
		}
		action_duration--;
	}

	if(action_duration == 0)			//ready for new action
	{	
		movement_direction = NO_DIRECTION;
		bAttacking = false;
		attacking_here = nullptr;
		bMoving = false;
		hidden = false;
		
		check_targets();

		if(current_target == nullptr)
		{
			if(time_to_search_for_enemies == 0)
			{
				find_near_enemies();
				time_to_search_for_enemies = interval_between_enemies_search;
			}
			else
				time_to_search_for_enemies--;

			if(time_to_search_for_enemies < 0)
				throw std::exception();
		}

		if((current_target != nullptr) && (path.empty()) && (next_tile_to_attack == NULL))
		{
			if(!try_fullfill_target(current_target))
				clear_target();
		}
		if(next_tile_to_attack != NULL)
		{
			bool success = try_attack_tile(next_tile_to_attack);
			if(!success)
			{
				if(!try_fullfill_target(current_target))
					clear_target();

				if(next_tile_to_attack != NULL)
				{
					if(!try_attack_tile(next_tile_to_attack))
						throw std::exception();
				}

				else if(!path.empty())
				{
					if(!move_to_next_tile())
						throw std::exception();
				}
			}
		}
		else if(!path.empty())
		{
			bool success = move_to_next_tile();
			if(!success)
			{
				if(!try_fullfill_target(current_target))
					clear_target();

				if(next_tile_to_attack != NULL)
				{	
					if(!try_attack_tile(next_tile_to_attack))
						throw std::exception();
				}
				else if(!path.empty())
				{	
					if(!move_to_next_tile())
						throw std::exception();
				}
			}
		}
	}
}

void warrior::specific_draw_interface()
{
	if(starving)
	{
		al_draw_text(font15, WRITING_COLOR, 200, display_height - BUTTON_SIZE, ALLEGRO_ALIGN_LEFT, "Unit is starving");
	}
}

void warrior::damage(int damage, tile* attacker_position, bool is_ranged)
{
	int evasion_chance = 0;		//in percents
	boost::shared_ptr<tile> t = session->tile_list[tile_y][tile_x];
	if((is_ranged) && (t->can_go_on_building))
	{
		if(t->building_on_tile.expired())	
			throw std::exception();
		
		switch(t->building_on_tile.lock()->type)
		{
			case(PALISADE):
				evasion_chance = 50;
				break;
			case(WALL):
				evasion_chance = 75;
				break;
			case(SOUTHWEST_TOWER):
			case(NORTHWEST_TOWER):
			case(NORTHEAST_TOWER):
			case(SOUTHEAST_TOWER):
				evasion_chance = 90;
				break;
			default:
				throw std::exception();
		}
	}
	
	bool evade = false;
	if(rand() % 100 < evasion_chance)
	{
		evade = true;
	}

	damage -= armor;
	if(damage < 0)
		damage = 0;

	if(!evade)
		life -= damage;

	if(life < 0)
		die();
	
	attacker_position->check_death_people_on_tile();
	
	if(attacker_position->people_on_tile.size() > 0)
	{
		people* p = attacker_position->people_on_tile[0].lock().get();
		if(p->show_owner() != owner)
		{
			add_target(boost::shared_ptr<target>(new target(attacker_position->people_on_tile[0], target_priority::ATTACKED_BY_ENEMY)));
			if(owner == RED_PLAYER)
			{
				session->ai.unit_attacked(attacker_position);
			}
		}
	}
	else if((!attacker_position->building_on_tile.expired()) && (attacker_position->building_on_tile.lock()->show_owner() != owner))
		add_target(boost::shared_ptr<target>(new target(attacker_position->building_on_tile, target_priority::ATTACKED_BY_ENEMY)));
}

bool warrior::try_attack_tile(tile* to)
{
	tile* from = session->tile_list[tile_y][tile_x].get();
	
	direction attack_direction = compute_direction(from, to);
	
	if(action_duration == 0)
	{	
		if(to->are_people_on_tile())
		{
			boost::shared_ptr<people> people_on_tile_ptr = to->people_on_tile[0].lock();
			if(can_attack_people(from, to))
			{
				if(people_on_tile_ptr->show_owner() != owner)
				{	
					action_duration = frames_to_attack;
					max_action_duration = action_duration;
					movement_direction = attack_direction;
					bAttacking = true;
					assert(next_tile_to_attack != nullptr);
					attacking_here = next_tile_to_attack;
					return true;
				}
			}
		}
		
		boost::shared_ptr<building> building_on_tile_ptr = to->building_on_tile.lock();
		if((building_on_tile_ptr) && (!bAttacking))
		{
			if(can_attack_building(from, to))
			{
				if(building_on_tile_ptr->show_owner() != owner)
				{
					action_duration = frames_to_attack;
					max_action_duration = action_duration;
					movement_direction = attack_direction;
					bAttacking = true;
					assert(next_tile_to_attack != nullptr);
					attacking_here = next_tile_to_attack;
					return true;
				}
			}
		}
	}
	bAttacking = false;
	return false;
}

void warrior::add_target(boost::shared_ptr<target> t)
{
	if((current_target == nullptr) || (t->priority >= current_target->priority))
	{
		if(try_fullfill_target(t))
			current_target = t;
	}
}

void warrior::check_death()
{	
}

void warrior::check_targets()
{
	if(current_target == nullptr)
		return;
	
	bool erase = false;
	if((current_target->type == PEOPLE) && (current_target->people_target.expired()))
		erase = true;

	else if((current_target->type == BUILDING) && (current_target->building_target.expired()))
		erase = true;
	
	else if((current_target->type == TILE) && ((current_target->tile_target->show_tile_x() == tile_x) && (current_target->tile_target->show_tile_y() == tile_y)))
		erase = true;
	
	if(erase)
	{
		current_target = nullptr;
	}
	
	if(current_target != nullptr)
	{
		if((current_target->type == PEOPLE) && (!current_target->people_target.expired()))
		{
			if((next_tile_to_attack == nullptr) && (!path.empty()) && ((path.back()->show_tile_x() != current_target->people_target.lock()->show_tile_x())
								|| (path.back()->show_tile_y() != current_target->people_target.lock()->show_tile_y())))
			{
				if(!try_fullfill_target(current_target))
					clear_target();
			}
		}
	}
}

bool warrior::try_fullfill_target(boost::shared_ptr<target> target_to_fullfill)
{
	if(target_to_fullfill == nullptr)
		return false;

	bool target_ok = false;

	if(target_to_fullfill->type == TILE)
	{	
		tile* t = target_to_fullfill->tile_target;
		if(can_go_here(t))
		{
			std::vector<tile*> starting_tile(1, session->tile_list[tile_y][tile_x].get());
			std::vector<tile*> path_to_goal = pathfinding::a_star(starting_tile, [this](tile* from, tile* to) -> bool {return this->can_move(from, to);}, 						pathfinding::single_tile_goal_functor(t), pathfinding::single_tile_heuristic_functor(t), pathfinding::normal_real_distance_functor());
			
			if(path_to_goal.size() > 0)
			{
				next_tile_to_attack = NULL;
				path = path_to_goal;
				target_ok = true;
			}
		}
	}
	else if((target_to_fullfill->type == PEOPLE) || (target_to_fullfill->type == BUILDING))
	{
		tile* tile_under_target;
		tile* from = session->tile_list[tile_y][tile_x].get();

		if(target_to_fullfill->type == PEOPLE)
		{
			boost::shared_ptr<people> p = target_to_fullfill->people_target.lock();
			tile_under_target = session->tile_list[p->show_tile_y()][p->show_tile_x()].get();
			
			if(can_attack_people(from, tile_under_target))
			{
				next_tile_to_attack = tile_under_target;
				path.clear();
				target_ok = true;
			}
		}
		else
		{
			boost::shared_ptr<building> b = target_to_fullfill->building_target.lock();
			std::vector<tile*> tiles_under_target = tiles_under_building(b->show_tile_x(), b->show_tile_y(), b->show_size());
			tile_under_target = tiles_under_target[0];

			for(int i=0; i<tiles_under_target.size(); ++i)
			{
				if(can_attack_building(from, tiles_under_target[i]))
				{
					next_tile_to_attack = tiles_under_target[i];
					path.clear();
					target_ok = true;
				}
			}
		}

		if(!target_ok)
		{
			std::vector<tile*> tiles_near_target = tiles_in_circle(1.0, tile_under_target);
			std::vector<tile*> goal_tiles;
			for(int i=0; i<tiles_near_target.size(); ++i)
			{
				if(target_to_fullfill->type == PEOPLE)
				{
					if(can_attack_people(tiles_near_target[i], tile_under_target))
					{
						goal_tiles.push_back(tiles_near_target[i]);
					}
				}
				else if(target_to_fullfill->type == BUILDING)
				{
					if(can_attack_building(tiles_near_target[i], tile_under_target))
					{
						goal_tiles.push_back(tiles_near_target[i]);
					}
				}
			}
			if(goal_tiles.size() > 0)
			{
				std::vector<tile*> starting_tile(1, session->tile_list[tile_y][tile_x].get());
				std::vector<tile*> new_path = pathfinding::a_star(starting_tile, [this](tile* from, tile* to) {return this->can_move(from, to);}, pathfinding::multiple_tile_goal_functor(goal_tiles), pathfinding::single_tile_heuristic_functor(tile_under_target), pathfinding::normal_real_distance_functor());

				if(new_path.size() > 0)
				{
					next_tile_to_attack = NULL;
					path = new_path;
					target_ok = true;
				}
			}
		}
	}
	return target_ok;
}

void warrior::clear_target()
{
	current_target = nullptr;
	next_tile_to_attack = nullptr;
	path.clear();
}

void warrior::find_near_enemies()
{
	tile* t = session->tile_list[tile_y][tile_x].get();

	std::vector<tile*> adjacent = pathfinding::adjacent_tiles(t, true);
	for(int i=0; i<adjacent.size(); ++i)
	{	
		if(can_attack_people(t, adjacent[i]) || can_attack_building(t, adjacent[i]))
		{
			adjacent[i]->check_death_people_on_tile();
			if(adjacent[i]->people_on_tile.size() > 0)
			{
				if(adjacent[i]->people_on_tile[0].lock()->show_owner() != owner)
				{
					add_target(boost::shared_ptr<target>(new target(adjacent[i]->people_on_tile[0], target_priority::ENEMY_NEARBY)));
					next_tile_to_attack = t;
					return;
				}
			}
			if(!adjacent[i]->building_on_tile.expired())
			{
				if(adjacent[i]->building_on_tile.lock()->show_owner() != owner)
				{
					add_target(boost::shared_ptr<target>(new target(adjacent[i]->building_on_tile, target_priority::ENEMY_NEARBY)));
					next_tile_to_attack = t;
					return;
				}
			}
		}
	}

	std::vector<tile*> starting_tile(1, session->tile_list[tile_y][tile_x].get());
	std::vector<std::vector<tile*>> paths = pathfinding::breadth_first_search(starting_tile, [this](tile* from, tile* to) {return this->can_move(from, to);}, 
						pathfinding::any_enemy_goal_functor(boost::dynamic_pointer_cast<warrior>(shared_from_this())), true, true, 7);

	if(paths.size() > 0)
	{
		std::vector<tile*> adjacent = pathfinding::adjacent_tiles(paths[0].back(), true);
		for(int i=0; i<adjacent.size(); ++i)
		{	
			adjacent[i]->check_death_people_on_tile();
			if(adjacent[i]->people_on_tile.size() > 0)
			{
				if(can_attack_people(paths[0].back(), adjacent[i]))
				{
					if(adjacent[i]->people_on_tile[0].lock()->show_owner() != owner)
					{
						add_target(boost::shared_ptr<target>(new target(adjacent[i]->people_on_tile[0], target_priority::ENEMY_NEARBY)));
						path = paths[0];
						break;
					}
				}
			}
			if(!adjacent[i]->building_on_tile.expired())
			{
				if(can_attack_building(paths[0].back(), adjacent[i]))
				{
					if(adjacent[i]->building_on_tile.lock()->show_owner() != owner)
					{
						add_target(boost::shared_ptr<target>(new target(adjacent[i]->building_on_tile, target_priority::ENEMY_NEARBY)));
						path = paths[0];
						break;
					}
				}
			}
		}
	}
}

void warrior::execute_attack()
{
	tile* from = session->tile_list[tile_y][tile_x].get();
	tile* to = attacking_here;
	
	if(to->are_people_on_tile())
	{
		boost::shared_ptr<people> people_on_tile_ptr = to->people_on_tile[0].lock();
		if(can_attack_people(from, to))
		{
			if(people_on_tile_ptr->show_owner() != owner)
			{	
				switch(type)
				{
					case(AXEMAN):
					case(SWORDSMAN):
						people_on_tile_ptr->damage(attack, from, is_ranged);
						break;
					case(BOWMAN):
						session->missile_list.push_back(boost::shared_ptr<missile>(new missile(ARCHERS_ARROW, from, attack, to)));
						break;
					case(SPEARMAN):
						session->missile_list.push_back(boost::shared_ptr<missile>(new missile(ARCHERS_ARROW, from, attack, to)));
						break;
					case(CATAPULT):
						session->missile_list.push_back(boost::shared_ptr<missile>(new missile(CATAPULT_SHOT, from, attack, to)));
						break;
					default:
						throw std::exception();
				}
				return;
			}
		}
	}
	
	boost::shared_ptr<building> building_on_tile_ptr = to->building_on_tile.lock();
	if(building_on_tile_ptr)
	{
		if(can_attack_building(from, to))
		{
			if(building_on_tile_ptr->show_owner() != owner)
			{
				switch(type)
				{
					case(AXEMAN):
					case(SWORDSMAN):
						building_on_tile_ptr->damage(attack);
						break;
					case(BOWMAN):
						session->missile_list.push_back(boost::shared_ptr<missile>(new missile(ARCHERS_ARROW, from, attack, to)));
						break;
					case(SPEARMAN):
						session->missile_list.push_back(boost::shared_ptr<missile>(new missile(ARCHERS_ARROW, from, attack, to)));
						break;
					case(CATAPULT):
						session->missile_list.push_back(boost::shared_ptr<missile>(new missile(CATAPULT_SHOT, from, attack, to)));
						break;
					default:
						throw std::exception();
				}
				return;
			}
		}
	}
	return;
}


void warrior::find_free_tile_near()
{
	tile* t = session->tile_list[tile_y][tile_x].get();
	t->check_death_people_on_tile();
	if(t->people_on_tile.size() > 1)
	{
		std::vector<tile*> adjacent = pathfinding::adjacent_tiles(t, true);
		std::vector<tile*> free;
		std::vector<tile*> accessible;
	
		for(int i=0; i<adjacent.size(); ++i)
		{
			t->check_death_people_on_tile();
			if(can_move(t, adjacent[i]))
			{
				if(t->people_on_tile.size() == 0)
					free.push_back(t);

				else
					accessible.push_back(t);
			}
		}
		if(free.size() > 0)
		{
			path.push_back(free[rand() % free.size()]);
		}
		else if(accessible.size() > 0)
		{
			path.push_back(accessible[rand() % accessible.size()]);
		}
	}
}

target_priority warrior::show_target_priority()
{
	if(current_target == nullptr)
		return target_priority::NO_TARGET;

	return current_target->priority;
}

void warrior::regenerate()
{
	if(regeneration_time > 0)
	{
		regeneration_time--;
		return;
	}
	
	regeneration_time = max_regeneration_time;
	if(starving)
	{
		life--;
		if(life < 0)
			die();
	}
	else	
	{
		life++;
		if(life > max_life)
			life = max_life;
	}
}

bool warrior::can_go_here(tile* t)
{
	t->check_death_people_on_tile();
	if(t->people_on_tile.size() > 0)
	{
		if(t->people_on_tile[0].lock()->show_owner() != owner)
			return false;
	}
	return ((t->building_on_tile.expired() || t->can_go_inside_building || t->can_go_on_building) && (t->object == NOTHING) && (!t->is_water_tile()));
}

bool warrior::can_move(tile* from, tile* to)
{
	if(!can_go_here(to))
		return false;

	return people::general_can_move(from, to);
}

bool warrior::can_attack_people(tile* from, tile* enemy_tile)
{
	switch(type)
	{
		case(AXEMAN):
		case(SWORDSMAN):
			return general_can_move(from, enemy_tile);
		break;
		case(SPEARMAN):
		case(BOWMAN):
		case(CATAPULT):
			return can_attack_ranged(from, enemy_tile);
		break;
		default:
			throw std::exception();
	}
	throw std::exception();
	return 0;
}

bool warrior::can_attack_building(tile* from, tile* enemy_tile)
{
	if(enemy_tile->building_on_tile.expired())
		return false;

	switch(type)
	{	
		case(AXEMAN):
		case(SWORDSMAN):
		{
			if((abs(enemy_tile->show_tile_x() - from->show_tile_x()) > 1) | (abs(enemy_tile->show_tile_y() - from->show_tile_y()) > 1))
				return false;
			
			if(owner == enemy_tile->building_on_tile.lock()->show_owner())
				return false;

			if(from->show_effective_height() == enemy_tile->show_surface_height())
				return true;

			if(from->show_effective_height() == enemy_tile->show_surface_height() + 1)
			{
				if(tile_borders(from, compute_direction(from, enemy_tile)) == DOWN_BORDER)
					return true;
			}

			return false;
		}
		break;
		case(SPEARMAN):
		case(BOWMAN):
		case(CATAPULT):
		{
			return can_attack_ranged(from, enemy_tile);	
		}
		break;
		default:
			throw std::exception();
	}
	throw std::exception();
	return false;
}

bool warrior::can_attack_ranged(tile* from, tile* to)
{
	double distance = sqrt(pow(from->show_tile_x() - to->show_tile_x(), 2) + pow(from->show_tile_y() - to->show_tile_y(), 2));
	return distance <= range;
}

carrier::carrier(people_type type, int tile_x, int tile_y, int surface_height, player owner, boost::shared_ptr<building> home_building)
			: people(type, tile_x, tile_y, surface_height, owner), home_building(home_building), transaction_size(0)
{
	waiting = true;
}

void carrier::update()
{
	if((previous_tile != nullptr) && (next_tile != nullptr))
		if((abs(previous_tile->show_tile_x() - next_tile->show_tile_x()) > 1) || (abs(next_tile->show_tile_y() - previous_tile->show_tile_y()) > 1))
			throw std::exception();

	if(bIs_death)
		return;
	
	if(waiting)
		return;

	if(action_duration > 0)			//is in the middle of action
	{
		small_move();
		if(action_duration == 1)
		{
			movement_direction = NO_DIRECTION;
			bMoving = false;
		}
		action_duration--;
	}

	if(action_duration == 0)			//ready for new action
	{
		check_target();
		
		if(is_target_adjacent())
		{
			target_reached();
		}	
		else
		{
			bool valid_path = move_to_next_tile();	
		
			if((!valid_path) || (path.empty()))
			{
				bool target_reachable = find_path_to_target();
				if(!target_reachable)
				{
					if(target.lock() == home_building.lock())
						die();
					else
					{
						target = home_building;
						if(!find_path_to_target())
							die();
					}
				}
			}
		}
	}
	if(hidden)
		throw std::exception();
}

void carrier::specific_draw_interface()
{
	if(((type_of_transaction == IN_TRANSACTION) && (path.back()->building_on_tile.lock() == home_building.lock())) || 
		((type_of_transaction == OUT_TRANSACTION) && (path.back()->building_on_tile.lock() == target.lock())))
	{
		al_draw_bitmap_region(image_list[RESOURCES_IMAGE], 30 * static_cast<int>(resource_carried), 0, 30, 30, 100, display_height - BUTTON_SIZE + 5, 0);
		al_draw_textf(font15, WRITING_COLOR, 140, display_height - BUTTON_SIZE + 5, ALLEGRO_ALIGN_LEFT, "%i", transaction_size);
	}
}

void carrier::give_task(resources resource_type, int amount, transaction_type type, boost::shared_ptr<building> target, std::vector<tile*> path_to_target)
{
	if(!target->has_carrier_output())
		throw std::exception();

	resource_carried = resource_type;
	transaction_size = amount;
	type_of_transaction = type;
	this->target = target;
	path = path_to_target;
	bool in_building = waiting;
	waiting = false;
	
	transaction_type reversed_transaction;
	if(type == IN_TRANSACTION)
		reversed_transaction = OUT_TRANSACTION;
	else 
		reversed_transaction = IN_TRANSACTION;

	bool success_1 = home_building.lock()->show_carrier_output()->reserve_transaction(resource_type, amount, type);
	bool success_2 = target->show_carrier_output()->reserve_transaction(resource_type, amount, reversed_transaction);
	
	if((!success_1) || (!success_2))
		throw std::exception();

	if(type == OUT_TRANSACTION)
		home_building.lock()->show_carrier_output()->accomplish_transaction(resource_type, amount, type);	//subtract resources which should be carried away
	if(path_to_target.size() == 0)
		throw std::exception();

	if(in_building)
	{
		path_to_target[0]->people_come(shared_from_this());
		int new_height = path_to_target[0]->show_surface_height() + 1;
		if(path_to_target[0]->stairs_on_tile)
			new_height++;
		
		game_object::update(path_to_target[0]->show_tile_x(), path_to_target[0]->show_tile_y(), new_height);
			
	}
}

void carrier::damage(int damage, tile* attacker_position, bool is_ranged)
{
	damage -= armor;
	life -= damage;
	if(life < 0)
	{
		die();
	}
}

bool carrier::can_go_here(tile* t)
{
	t->check_death_people_on_tile();
	if((t->people_on_tile.size() > 0) && (t->people_on_tile[0].lock()->show_owner() != owner))
		return false;

	return static_can_go_here(t);
}

bool carrier::can_move(tile* from, tile* to)
{
	if(!can_go_here(to))
		return false;

	if((from->show_tile_x() != to->show_tile_x()) && (from->show_tile_y() != to->show_tile_y()))		//cant move diagonally
		return false;
	
	return people::general_can_move(from, to);
}

bool carrier::static_can_go_here(tile* t)
{
	if(!t->building_on_tile.expired())
	{
		if(t->can_go_on_building)
		{
			building_type b = t->building_on_tile.lock()->type;
			if((b == NORTHWEST_STAIRS) || (b == NORTHEAST_STAIRS) || (b == SOUTHEAST_STAIRS) || (b == SOUTHWEST_STAIRS))
				return true;

			else 
				return false;

		}
		else if(t->can_go_inside_building)
			return true;

		else
			return false;
	}

	return t->real_path_on_tile();
}

bool carrier::static_can_move(tile* from, tile* to)
{
	if(!static_can_go_here(to))
		return false;

	if((from->show_tile_x() != to->show_tile_x()) && (from->show_tile_y() != to->show_tile_y()))		//cant move diagonally
		return false;
	
	return people::general_can_move(from, to);
}

bool carrier::find_path_to_target()
{
	if(target.expired())
		throw std::exception();
	
	std::vector<tile*> starting_tiles(1, session->tile_list[tile_y][tile_x].get());
	tile* target_tile = session->tile_list[target.lock()->show_tile_y()][target.lock()->show_tile_x()].get();
	
	path = pathfinding::a_star(starting_tiles, [this](tile* from, tile* to){return this->can_move(from, to);}, pathfinding::single_building_goal_functor(target_tile, false), pathfinding::single_tile_heuristic_functor(target_tile), pathfinding::carrier_real_distance_functor(), false);
	
	return (path.size() > 0);
}

bool carrier::is_target_adjacent()
{
	boost::shared_ptr<building> target_building = target.lock();
	if(target_building == NULL)
		throw std::exception();

	std::vector<tile*> target_tiles = tiles_under_building(target_building->show_tile_x(), target_building->show_tile_y(), target_building->show_size());
	std::vector<tile*> adjacent_tiles = session->tile_list[tile_y][tile_x]->accessible_neighbours;

	for(int i=0; i<target_tiles.size(); ++i)
	{
		for(int j=0; j<adjacent_tiles.size(); ++j)
		{
			if(target_tiles[i] == adjacent_tiles[j])
			{
				return true;
			}
		}
	}
	return false;
}

void carrier::check_target()
{
	if(home_building.expired())
	{
		die();
		return;
	}
	if((target.expired()) && !waiting)
	{
		target = home_building;
		bool success = find_path_to_target();
		if(!success)
			die();
	}
}
void carrier::target_reached()
{
	bool home = (target.lock() == home_building.lock());
	boost::shared_ptr<carrier_output> output = target.lock()->show_carrier_output();
	
	if(home)
	{
		if(type_of_transaction == IN_TRANSACTION)
			output->accomplish_transaction(resource_carried, transaction_size, type_of_transaction);
		
		output->add_idle_carrier(boost::dynamic_pointer_cast<carrier>(shared_from_this()));
		
		session->tile_list[tile_y][tile_x]->people_leave(shared_from_this());
		waiting = true;
		transaction_size = 0;
	}
	else
	{
		transaction_type other_transaction = (type_of_transaction == IN_TRANSACTION) ? OUT_TRANSACTION : IN_TRANSACTION;
		output->accomplish_transaction(resource_carried, transaction_size, other_transaction);
		
		target = home_building;
		if(!is_target_adjacent())
		{
			bool success = find_path_to_target();
			if(!success)
				die();
		}
	}
}
	

