#include "core.h"

extern std::ofstream log_file;
extern int display_width;
extern int display_height;
extern ALLEGRO_BITMAP** image_list;
extern ALLEGRO_FONT* font25;
extern ALLEGRO_FONT* font15;
extern game_session* session;

int building::next_id = 0;

boost::shared_ptr<building> building::create_building(building_type type, int tile_x, int tile_y, int surface_height, player owner, bool is_real)
{
	boost::shared_ptr<building> new_building;

	switch(type)
	{
	case(NORTHWEST_TOWER):
	case(NORTHEAST_TOWER):
	case(SOUTHEAST_TOWER):
	case(SOUTHWEST_TOWER):
		new_building = boost::shared_ptr<building>(new tower(type, tile_x, tile_y, surface_height, BLUE_PLAYER, is_real));
		break;
	case(BARRACKS):
		new_building = boost::shared_ptr<building>(new barracks(type, tile_x, tile_y, surface_height, BLUE_PLAYER, is_real));
		break;
	case(WAREHOUSE):
		new_building = boost::shared_ptr<building>(new warehouse(type, tile_x, tile_y, surface_height, BLUE_PLAYER, is_real));
		break;
	case(WALL):
		new_building = boost::shared_ptr<building>(new wall(type, tile_x, tile_y, surface_height, BLUE_PLAYER, is_real));
		break;
	case(PALISADE):
		new_building = boost::shared_ptr<building>(new wall(type, tile_x, tile_y, surface_height, BLUE_PLAYER, is_real));
		break;
	case(NORTHWEST_STAIRS):
	case(NORTHEAST_STAIRS):
	case(SOUTHEAST_STAIRS):
	case(SOUTHWEST_STAIRS):
		new_building = boost::shared_ptr<building>(new stairs(type, tile_x, tile_y, surface_height, BLUE_PLAYER, is_real));
		break;
	case(CHURCH):
		new_building = boost::shared_ptr<building>(new church(type, tile_x, tile_y, surface_height, BLUE_PLAYER, is_real));
		break;
	case(STORE):
		new_building = boost::shared_ptr<building>(new store(type, tile_x, tile_y, surface_height, BLUE_PLAYER, is_real));
		break;
	case(SCOUT):
		new_building = boost::shared_ptr<building>(new scout(type, tile_x, tile_y, surface_height, BLUE_PLAYER, is_real));
		break;
	case(LEFT_GATE):
	case(RIGHT_GATE):
		new_building = boost::shared_ptr<building>(new gate(type, tile_x, tile_y, surface_height, BLUE_PLAYER, is_real));
		break;
	case(QUARRY):
	case(WOODCUTTER):
	case(HUNTER):
	case(FISHERMAN):
	case(APPLE_FARM):
	case(DAIRY_FARM):
	case(WHEAT_FARM):
	case(WINDMILL):
	case(CLAY_PIT):
	case(POTTERY_WORKSHOP):
	case(BRICKMAKER):
	case(MARBLE_QUARRY):
	case(GOLD_MINE):
	case(COAL_MINE):
	case(IRON_MINE):
	case(SMITH):
	case(ARMOURER):
	case(FLETCHER):
		new_building = boost::shared_ptr<building>(new production_building(type, tile_x, tile_y, surface_height, BLUE_PLAYER, is_real));
		break;
	case(HOUSE):
		new_building = boost::shared_ptr<building>(new house(type, tile_x, tile_y, surface_height, BLUE_PLAYER, is_real));
		break;
	case(MARKET):
		new_building = boost::shared_ptr<building>(new market(type, tile_x, tile_y, surface_height, BLUE_PLAYER, is_real));
		break;
	case(GREAT_HALL):
		new_building = boost::shared_ptr<building>(new great_hall(type, tile_x, tile_y, surface_height, BLUE_PLAYER, is_real));
		break;


	default:
		{
			LOG("button_build::init - error not defined building class " << type);
			throw std::exception();
		}
	}
	if((new_building->has_carrier_output()) && (is_real))
		new_building->show_carrier_output()->init(new_building);

	return new_building;
}

/*This function should be called once per frame for every buidling. Updates building.*/
void building::update()
{
	if(bIs_death)
		return;

	if((required_workers == actual_workers) && (!stopped))
		specific_update();
	
	if((has_carrier_output()) && (!stopped))
		show_carrier_output()->update();
}

/*Draws one floor of building. If the floor isn't last, returns this. For some buildings it is overwrited.*/ 
std::vector<game_object*> building::draw(int screen_position_x, int screen_position_y)
{
	int drawing_x = game_x - screen_position_x - 96;
	int drawing_y = game_y - screen_position_y - show_surface_height()*32 - 80 - drawing_floor*32;

	if(draw_selection)
	{
		draw_selection = false;
		al_draw_bitmap_region(image_list[BUILDING_SELECTION_IMAGE], size * 192, 0, 192, 128, drawing_x, drawing_y, 0);
	}

	if(is_real)
	{
		al_draw_bitmap_region(image_list[image], 64 + 192*drawing_floor, 0, 192, 128, drawing_x, drawing_y, 0);

		for(int i=0; i<upgrade_level; ++i)				//draw stars for upgrade
		{
			al_draw_bitmap(image_list[STAR_IMAGE], drawing_x + 25*i + 64, drawing_y + 92, 0);
		}
	}

	else 
	{
		if(draw_green)
			al_draw_tinted_bitmap_region(image_list[image], LIGHT_GREEN_COLOR, 64 + 192*drawing_floor, 0, 192, 128, drawing_x, drawing_y, 0);
		else 
			al_draw_tinted_bitmap_region(image_list[image], LIGHT_RED_COLOR, 64 + 192*drawing_floor, 0, 192, 128, drawing_x, drawing_y, 0);
	}

	drawing_floor++;

	if(drawing_floor < number_of_floors)			//this is necessary for right perspective when drawing high buildings  
	{
		return std::vector<game_object*>(1, this);
	}
	
	if(is_real)
	{
		if((actual_workers < required_workers) || (stopped))
		{
			if(size == FOUR_TILE_BUILDING)
				drawing_y += 32;

			if(size == ONE_TILE_BUILDING)
				drawing_y += 64;

			al_draw_bitmap(image_list[NO_WORKERS_IMAGE], drawing_x + 64, drawing_y - 32, 0);
		}
	}

	drawing_floor = 0;
	return std::vector<game_object*>();
}

/* If building is chosen by player, it draws it's own panel with buttons (not class button, just button). This function draws two buttons that every function have - destroy building and stop working.*/ 
void building::draw_interface()
{	
	int start_x = display_width - 3*BUTTON_SIZE + 8;

	al_draw_bitmap(image_list[REMOVE_BUILDING_IMAGE], start_x, display_height - BUTTON_SIZE + 8, 0);
	start_x -= BUTTON_SIZE;
	
	if(can_be_stopped())
	{
		al_draw_bitmap(image_list[STOP_WORKING_IMAGE], start_x, display_height - BUTTON_SIZE + 8, 0);
		start_x -= BUTTON_SIZE;
	}

	if(can_be_upgraded())
	{
		al_draw_bitmap(image_list[UPGRADE_IMAGE], start_x, display_height - BUTTON_SIZE + 8, 0);
		start_x -= BUTTON_SIZE;
	}
	
	std::string name = building_info::show_building_info(type).name;
	al_draw_text(font25, WRITING_COLOR, display_width/2, display_height - BUTTON_SIZE, ALLEGRO_ALIGN_CENTRE, name.c_str());

	draw_specific_interface();
}

void building::function_click(int mouse_x, int mouse_y)		//should be called only if player clicked  in panel area with building interface
{
	int x = display_width - 2*BUTTON_SIZE;

	if((mouse_x < x) && (mouse_x > x - BUTTON_SIZE))	//destroy building click
	{
		destroy_building();
	}
	x -= BUTTON_SIZE;
	
	if(can_be_stopped())
	{
		if((mouse_x < x) && (mouse_x > x - BUTTON_SIZE))
		{
			change_working();
		}
		x -= BUTTON_SIZE;
	}

	if(can_be_upgraded())
	{
		if((mouse_x < x) && (mouse_x > x - BUTTON_SIZE))
		{
			general_upgrade();
		}
		x -= BUTTON_SIZE;
	}
}

void building::draw_function_info(int mouse_x, int mouse_y)
{	
	specific_draw_function_info(mouse_x, mouse_y);

	if(mouse_y < display_height - BUTTON_SIZE)
		return;

	int x = display_width - 2*BUTTON_SIZE;

	if((mouse_x < x) && (mouse_x > x - BUTTON_SIZE))	//destroy building click
	{
		button::draw_button_info("Destroy building", "No resources will be returned.", std::vector<int>(NUMBER_OF_RESOURCES, 0), 0, 0, x - BUTTON_SIZE);
	}
	x -= BUTTON_SIZE;
	if(can_be_stopped())
	{
		if((mouse_x < x) && (mouse_x > x - BUTTON_SIZE))
		{
			std::string name;
			if(stopped)
				name = "Start working";
			else
				name = "Stop working";

			button::draw_button_info(name, "", std::vector<int>(NUMBER_OF_RESOURCES, 0), 0, 0, x - BUTTON_SIZE);
		}
		x -= BUTTON_SIZE;
	}

	if(can_be_upgraded())
	{
		if((mouse_x < x) && (mouse_x > x - BUTTON_SIZE))
		{
			if(can_be_upgraded())
			{
				building_info info = building_info::show_building_info(type);
				std::vector<int> price;
				switch(upgrade_level)
				{
					case(0):
						price = info.first_upgrade_price;
						break;
					case(1):
						price = info.second_upgrade_price;
						break;
					case(2):
						price = info.third_upgrade_price;
						break;
					default:
						throw std::exception();
				}
				button::draw_button_info("Upgrade", info.upgrade_info, price, 0, 0, x - BUTTON_SIZE);
			}
		}
		x -= BUTTON_SIZE;
	}
}

void building::draw_life_bar(int screen_position_x, int screen_position_y)
{
	int drawing_x = game_x - screen_position_x - 96;
	int drawing_y = game_y - screen_position_y - show_surface_height()*32 - 80 - number_of_floors*32;
	
	if(size == FOUR_TILE_BUILDING)
		drawing_y += 32;
	
	else if(size == ONE_TILE_BUILDING)
		drawing_y += 64;

	float health = (float)life/(float)max_life;
	al_draw_rectangle(drawing_x + 64, drawing_y - 2 + 160 - height_of_life_bar, drawing_x + 64 + TILE_WIDTH, drawing_y + 3 + 160 - height_of_life_bar, GREY_COLOR, 1);  
	al_draw_filled_rectangle(drawing_x + 64, drawing_y - 2 + 160 - height_of_life_bar, drawing_x + 64 + TILE_WIDTH*(health), drawing_y + 2 + 160 - height_of_life_bar, LIGHT_GREEN_COLOR);
}

/* Decrease life of building by damage.*/
void building::damage(int damage)
{	
	damage -= armor;
	if(damage < 0)
		return;

	life -= damage;
	if((life < 0) && (!bIs_death))
	{
		this->destroy_building();
	}
	return;
}

void building::rotate(int new_tile_x, int new_tile_y, bool clockwise)
{
	int coordinates_change = 0;
	if(size == FOUR_TILE_BUILDING)
		coordinates_change = 1;
	
	else if(size == NINE_TILE_BUILDING)
		coordinates_change = 2;

	if(clockwise)
	{
		new_tile_x += coordinates_change;
	}
	else
	{
		new_tile_y += coordinates_change;
	}
	
	game_object::update(new_tile_x, new_tile_y, show_surface_height());

}

void building::set_drawing_tile()
{
	std::vector<tile*> under_building = tiles_under_building(tile_x, tile_y, size);

	for(size_t i=0; i<under_building.size(); ++i)
		under_building[i]->set_draw_building(false);

	int drawing_tile_x = tile_x;
	int drawing_tile_y = tile_y;
	
	if(size == FOUR_TILE_BUILDING)
		drawing_tile_x--;
	
	else if(size == NINE_TILE_BUILDING)
		drawing_tile_x -= 2;
	
	session->tile_list[drawing_tile_y][drawing_tile_x]->set_draw_building(true);
	
	if(session->tile_list[drawing_tile_y][drawing_tile_x]->building_on_tile.expired())
	{
		LOG("error");
		throw std::exception();
	}
}

/*Returns true if building can be build on tile defined by parametr here. If not, returns false.*/
can_build_output building::can_build_here(tile* here, building_type type)
{
	if(session->game_started)
	{
		for(int i=0; i<NUMBER_OF_RESOURCES; ++i)		//check if player has enough resources
		{
			resources resource_type = static_cast<resources>(i);
			if(show_building_price(type, resource_type, NO_UPGRADE) > session->global_stock->show_amount(resource_type))
			{
				switch(resource_type)
				{
					case(WOOD):
						return can_build_output::MISSING_WOOD;
						break;
					case(STONE):
						return can_build_output::MISSING_STONE;
						break;
					case(BRICKS):
						return can_build_output::MISSING_BRICKS;
						break;
					case(MARBLE):
						return can_build_output::MISSING_MARBLE;
						break;
					default:
						throw std::exception();
				}
			}
		}
	}
	
	int end_x = here->show_tile_x();
	int end_y = here->show_tile_y();
	int start_x;
	int start_y;

	std::vector<tile*> near_tiles = tiles_in_circle(minimal_distance_from_enemies_for_building, here);
	for(size_t i=0; i<near_tiles.size(); ++i)		//buildings shouldt be built when enemies are nearby
	{
		near_tiles[i]->check_death_people_on_tile();
		if((!near_tiles[i]->people_on_tile.empty()) && (near_tiles[i]->people_on_tile[0].lock()->show_owner() == RED_PLAYER))
		{
			return can_build_output::ENEMIES_NEARBY;
		}
	}

	building_size size = building_info::show_building_info(type).size;
	
	switch(size)
	{
		case(ONE_TILE_BUILDING):
		{
			start_x = end_x;
			start_y = end_y;
		}
		break;
		case(FOUR_TILE_BUILDING):
		{
			start_x = end_x - 1;
			start_y = end_y - 1;
		}
		break;
		case(NINE_TILE_BUILDING):
		{
			start_x = end_x - 2;
			start_y = end_y - 2;
		}
		break;
		case(LEFT_GATE_BUILDING):
		{
			start_x = end_x - 2;
			start_y = end_y;
		}
		break;
		case(RIGHT_GATE_BUILDING):
		{
			start_x = end_x;
			start_y = end_y -2;
		}
		break;
		default:
			throw std::exception();
	}
	
	if(((start_x < 0) || (end_x > game_info::map_width - 1)) || ((start_y < 0) || (end_y > game_info::map_height - 1)))
		return can_build_output::NO_SPACE;
	
	for(int y=start_y; y <= end_y; y++)
	{
		for(int x=start_x; x <= end_x; x++)
		{
			boost::shared_ptr<tile> t = session->tile_list[y][x];
			if((t->show_surface_height() != session->tile_list[start_y][start_x]->show_surface_height()) 
					|| (t->show_type() != GRASS))
			{
				return can_build_output::NO_SPACE;
			}
			if(t->real_path_on_tile() && (((type != LEFT_GATE) && (type != RIGHT_GATE)) 
						|| (((type == LEFT_GATE) && (x != start_x + 1)) || ((type == RIGHT_GATE) && (y != start_y + 1)))))
			{
				return can_build_output::NO_SPACE;
			}

			else if(!t->is_free())
			{
				if(((size == LEFT_GATE_BUILDING) || (size == RIGHT_GATE_BUILDING)) && (t->building_on_tile.expired()) && (t->object == NOTHING) && ((y == start_y + 1) || (x == start_x + 1)))
				{
					//everything ok - building is gate and there are people on gate_tile
				}
				else	//everything else is bad
				{
					return can_build_output::NO_SPACE;
				}
			}
		}
	}
	
	return building::enough_resources(type, start_x, start_y, end_x, end_y);
}

building::building(building_type type, int tile_x, int tile_y, int surface_height, player owner, bool is_real) 
			: game_object(tile_x, tile_y, surface_height, is_real, building_info::show_building_info(type).image, 
						building_info::show_building_info(type).number_of_floors, BUILDING), 
				id(next_id++)
{
	building_info info = building_info::show_building_info(type);
	life = info.life;
	armor = info.armor;
	height_of_life_bar = info.height_of_life_bar;
	size = info.size;
	if(type == RIGHT_GATE)
		size = RIGHT_GATE_BUILDING;

	required_workers = info.number_of_workers;

	max_life = life;
	building::type = type;
	building::owner = owner;
	building::action_duration = 0;
	actual_workers = 0;
	bIs_death = false;
	stopped = false;
	draw_selection = false;
	upgrade_level = 0;
}

/*Removes building from game, but not deletes it. Some pointer may still points to it (people->building_target). Memory is deleted by check_death function.*/ 
void building::destroy_building()
{
	LOG("");
	bIs_death = true;

	stop_working();

	std::vector<tile*> tiles = tiles_under_building(tile_x, tile_y, size); 

	for(unsigned int i=0; i<tiles.size(); i++)
	{
		tiles[i]->set_draw_building(false);
		tile::minimap_updates.push_back(tiles[i]);
	}
}

bool building::can_be_upgraded()
{
	building_info info = building_info::show_building_info(type);
	if(info.can_be_upgraded && (upgrade_level < 3))
	{
		return true;
	}
	return false;
}

void building::general_upgrade()
{
	building_info info = building_info::show_building_info(type);
	std::vector<int> price;

	switch(upgrade_level)
	{
		case(0):
			price = info.first_upgrade_price;
			break;

		case(1):
			price = info.second_upgrade_price;
			break;

		case(2):
			price = info.third_upgrade_price;
			break;

		default:
			throw std::exception();

	}
	bool success = session->global_stock->try_subtract_list(price);
	if(success)
	{
		upgrade_level++;
		this->upgrade();
	}
}

int building::compute_button_number(int mouse_x, int mouse_y)
{
	if(mouse_y > display_height - BUTTON_SIZE)
	{
		for(int i=0; i<display_width*BUTTON_SIZE; ++i)
		{
			if((mouse_x > i*BUTTON_SIZE) && (mouse_x < (i+1)*BUTTON_SIZE))
			{
				return i;
			}
		}
	}
	return 100;			//this should indicate, that button number cannot be computed
}

can_build_output building::enough_resources(building_type type, int start_tile_x, int start_tile_y, int end_tile_x, int end_tile_y)
{
	if((type == WOODCUTTER) || (type == IRON_MINE) || (type == COAL_MINE) || (type == GOLD_MINE) || (type == QUARRY) || (type == MARBLE_QUARRY) 
				|| (type == FISHERMAN) || (type == HUNTER) || (type == CLAY_PIT))
	{
		int start_x = std::max(0, start_tile_x - 1);
		int start_y = std::max(0, start_tile_y - 1);
		int end_x = std::min(game_info::map_width - 1, end_tile_x + 1);
		int end_y = std::min(game_info::map_height - 1, end_tile_y + 1);

		int number_of_resources = 0;

		for(int y = start_y; y<= end_y; y++)
		{
			for(int x = start_x; x <= end_x; x++)
			{
				if(session->tile_list[start_tile_y][start_tile_x]->show_surface_height() != session->tile_list[y][x]->show_surface_height())
				{
					continue;
				}
				object_on_tile ob = session->tile_list[y][x]->object;
				if((ob == TREE_TILE) && ((type == WOODCUTTER) || (type == HUNTER)))
					number_of_resources++;
				
				else if(((ob == MARBLE_TILE) || (ob == IRON_TILE) || (ob == COAL_TILE) || (ob == GOLD_TILE)) && (type == QUARRY))
					number_of_resources++;

				else if((ob == MARBLE_TILE) && (type == MARBLE_QUARRY))
					number_of_resources++;
	
				else if((ob == IRON_TILE) && (type == IRON_MINE))
					number_of_resources++;
				
				else if((ob == COAL_TILE) && (type == COAL_MINE))
					number_of_resources++;
				
				else if((ob == GOLD_TILE) && (type == GOLD_MINE))
					number_of_resources++;

				else if(session->tile_list[y][x]->is_water_tile() && ((type == FISHERMAN) || (type == CLAY_PIT)))
					number_of_resources++;
			}
		}
		if(number_of_resources < 1)
		{
			switch(type)
			{
				case(WOODCUTTER):
				case(HUNTER):
					return can_build_output::NO_TREES;
					break;
				case(IRON_MINE):
					return can_build_output::NO_IRON;
					break;
				case(COAL_MINE):
					return can_build_output::NO_COAL;
					break;
				case(GOLD_MINE):
					return can_build_output::NO_GOLD;
					break;
				case(QUARRY):
					return can_build_output::NO_ROCKS;
					break;
				case(MARBLE_QUARRY):
					return can_build_output::NO_MARBLE;
					break;
				case(FISHERMAN):
				case(CLAY_PIT):
					return can_build_output::NO_WATER;
					break;
				default:
					throw std::exception();
			}
		}
		else
			return can_build_output::CAN_BUILD;
	}
	if((type == APPLE_FARM) || (type == WHEAT_FARM))
	{
		for(int x = start_tile_x; x <= end_tile_x; ++x)
		{
			for(int y = start_tile_y; y <= end_tile_y; ++y)
			{
				if(session->tile_list[y][x]->is_fertile())
				{
					return can_build_output::CAN_BUILD;
				}
			}
		}
		return can_build_output::NO_FERTILE_LAND;
	}
	return can_build_output::CAN_BUILD;
}

struct vertex_info;

void execute_order(std::vector<vertex_info> & vertices, int from, int to, int amount);

/**
 * \brief Describes vertex of graph of workers assignement. Works with vector<vertex_info> vertices, where are all vertices of the graph.
 */
struct vertex_info
{
public:
	vertex_info(boost::shared_ptr<building> b, bool is_house, int number_of_workers, int index_in_vertices) : b(b), is_house(is_house), being_asked(false), index_in_vertices(index_in_vertices), asked_neighbours(0), workers_in_house(0), workers_in_house_computed(false) {}
	boost::shared_ptr<building> b;		///< Each vertex represents building.
	bool is_house;
	std::vector<int> connected_indeces;	///< Indeces to "vertices" vector to connected buildings.
	std::vector<int> size_of_edge;		///< Similar structure as connected indeces: connected_indeces[i] = index of connected building B, size_of_edge[i] = size_of_edge between this and building B
	bool being_asked;			///< To stop infinite recursion.
	int index_in_vertices;			///< Index of this in vertices vector.
	int asked_neighbours;			///< Number of neighbours which were already asked for workers.
	int workers_in_house;			///< Idle workers in house
	bool workers_in_house_computed;	

	static void reset_being_asked(std::vector<vertex_info> & vertices);

	int ask_for_workers(std::vector<vertex_info> & vertices, int workers_needed);
};

void vertex_info::reset_being_asked(std::vector<vertex_info> & vertices)
{
	for(size_t i=0; i<vertices.size(); ++i)
	{
		vertices[i].being_asked = false;
		vertices[i].asked_neighbours = 0;
		vertices[i].workers_in_house_computed = false;
	}
}

/** 
 * Returns how many workers this vertex can give to the one which asked. 
 * Also generates orders to change sizes of edges such that it can give as much workers as possible.
 * Returns no more than workers needed.
 */
int vertex_info::ask_for_workers(std::vector<vertex_info> & vertices, int workers_needed)
{
	if(being_asked)		//stop infinite recursion - every vertex can be asked just once
		return 0;

	being_asked = true;
	int idle_workers = 0;
	if(is_house)
	{
		if(!workers_in_house_computed)
		{
			workers_in_house = boost::dynamic_pointer_cast<house>(b)->number_of_workers();	//Capacity of house
			for(size_t i=0; i<size_of_edge.size(); ++i)					
			{
				workers_in_house -= size_of_edge[i];			//subtract already used workers
			}
			
			workers_in_house_computed = true;

			if(workers_in_house < 0)
				throw std::exception();
		}
		
		if(workers_in_house < 0)
			throw std::exception();

		idle_workers = std::min(workers_in_house, workers_needed);
		workers_in_house = workers_in_house - idle_workers;

		if(idle_workers < 0)
			throw std::exception();
		
		while((idle_workers < workers_needed) && (asked_neighbours < size_of_edge.size()))	
					//try finding edge where I can send less workers then I am sending there already		 
		{
			if(size_of_edge[asked_neighbours] > 0)
			{
				int max_workers_transfer = std::min(workers_needed - idle_workers, size_of_edge[asked_neighbours]);
				int received_workers = vertices[connected_indeces[asked_neighbours]].ask_for_workers(vertices, max_workers_transfer);
				if(received_workers > 0)
				{	
					execute_order(vertices, index_in_vertices, connected_indeces[asked_neighbours], size_of_edge[asked_neighbours] - received_workers);
					idle_workers += received_workers;
				}
			}
			if(idle_workers < workers_needed)
				++asked_neighbours;
		}
	}
	else
	{	
		while((idle_workers < workers_needed) && (asked_neighbours < connected_indeces.size()))	
			//for each connected house, while I dont have enough workers, ask for workers
		{
			int received_workers = vertices[connected_indeces[asked_neighbours]].ask_for_workers(vertices, workers_needed - idle_workers);
			if(received_workers > 0)
			{
				execute_order(vertices, index_in_vertices, connected_indeces[asked_neighbours], size_of_edge[asked_neighbours] + received_workers);
				idle_workers += received_workers;
			}
			if(idle_workers < workers_needed)
				++asked_neighbours;
		}
	}

	if(idle_workers > workers_needed)
		throw std::exception();

	if(idle_workers < 0)
		throw std::exception();

	being_asked = false;
	return idle_workers;
}

void execute_order(std::vector<vertex_info> & vertices, int from, int to, int amount)
{
	for(size_t i=0; i<vertices[from].connected_indeces.size(); ++i)
	{
		if(vertices[from].connected_indeces[i] == to)
		{
			vertices[from].size_of_edge[i] = amount;
			break;
		}
	}

	for(size_t i=0; i<vertices[to].connected_indeces.size(); ++i)
	{
		if(vertices[to].connected_indeces[i] == from)
		{
			vertices[to].size_of_edge[i] = amount;
			break;
		}
	}
}

//Add "b" and all buildings connected by path to "vertices" if they are not there and returns index of "b" in "vertices". Returns -1 if building "b" dont need workers and it is not house.
//Stopped building cannot be added.
int add_building_to_graph(std::vector<vertex_info> & vertices, boost::shared_ptr<building> b, bool is_house)	
{
	if(is_house)
	{
		if(b->show_type() != HOUSE)
			return -1;
	}
	else if(!is_house)
	{ 
		if(b->show_required_workers() == 0)
			return -1;

		if(b->show_stopped())
			return -1;
	}

	for(size_t i=0; i<vertices.size(); ++i)		//if building has already been added, return it's index
	{
		if(vertices[i].b->id == b->id)
		{
			return i;
		}
	}

	int vertices_index = vertices.size();
	vertices.push_back(vertex_info(b, is_house, b->show_required_workers(), vertices_index));

	std::vector<tile*> building_tiles = tiles_under_building(b->show_tile_x(), b->show_tile_y(), b->show_size());

	std::vector<tile*> starting_tiles;	//starting tiles are tiles with path adjacent to building, because if it would be tiles_under_building, then adjacent buildings without path connecting them can share workers

	for(size_t i=0; i<building_tiles.size(); ++i)
	{
		std::vector<tile*> adjacent = pathfinding::adjacent_tiles(building_tiles[i], false);
		
		for(size_t j=0; j<adjacent.size(); ++j)
		{
			if((carrier::static_can_move(building_tiles[i], adjacent[j])) && (!contains(starting_tiles, adjacent[j])))
			{
				starting_tiles.push_back(adjacent[j]);
			}
		}
	}

	std::vector<std::vector<tile*>> connected = pathfinding::breadth_first_search(starting_tiles, carrier::static_can_move,
					[] (tile* from, tile* to) {return (!(to->building_on_tile.expired()) && (people::general_can_move(from, to)));}, 
					false, MAX_WORKERS_DISTANCE_FROM_HOUSE);
	
	for(size_t i=0; i<connected.size(); ++i)
	{
		boost::shared_ptr<building> connected_building = connected[i].back()->building_on_tile.lock();
		if(b == connected_building)
			continue;

		int connected_building_index = add_building_to_graph(vertices, connected_building, !is_house);
		
		if(connected_building_index != -1)
		{
			vertices[vertices_index].connected_indeces.push_back(connected_building_index);
			vertices[vertices_index].size_of_edge.push_back(0);				//all edges are 0 at the beginning
		}
	}
	return vertices_index;
}
	
void building::assign_workers()
{
	std::vector<vertex_info> vertices;
	for(size_t i=0; i<session->building_list.size(); ++i)		//initialization
	{
		add_building_to_graph(vertices, session->building_list[i], false);	//vertices will be biparted graph

		if(session->building_list[i]->show_type() == HOUSE)
		{
			boost::shared_ptr<house> house_ptr = boost::dynamic_pointer_cast<house>(session->building_list[i]);
			house_ptr->set_idle_workers(house_ptr->number_of_workers());	//reset number of idle workers
		}
	}

	for(size_t i=0; i<vertices.size(); ++i)				//try find some free workers
	{
		if(!vertices[i].is_house)
		{
			std::vector<vertex_info> new_graph = vertices;
			int founded_workers = new_graph[i].ask_for_workers(new_graph, new_graph[i].b->show_required_workers());
			vertex_info::reset_being_asked(new_graph);

			if(founded_workers == new_graph[i].b->show_required_workers())	//enough workers founded -> commit changes
			{
				vertices = new_graph;
				vertices[i].b->actual_workers = founded_workers;
			}
			else								//not enough workers -> forget new graph
				vertices[i].b->actual_workers = 0;
		}
	}
	for(size_t i=0; i<vertices.size(); ++i)			//set right number of idle workers in each house
	{
		if(vertices[i].is_house)
		{
			boost::shared_ptr<house> house_ptr = boost::dynamic_pointer_cast<house>(vertices[i].b);
			int idle_workers = house_ptr->number_of_workers();

			for(size_t j=0; j<vertices[i].size_of_edge.size(); ++j)
			{
				idle_workers -= vertices[i].size_of_edge[j];
			}
			if(idle_workers < 0)
				throw std::exception();

			house_ptr->set_idle_workers(idle_workers);
		}
	}
}

tower::tower(building_type type, int tile_x, int tile_y, int surface_height, player owner, bool is_real) 
			: building(type, tile_x, tile_y, surface_height, owner, is_real)
{
	if(is_real)
	{
		std::vector<tile*> tiles = tiles_under_building(tile_x, tile_y, FOUR_TILE_BUILDING);
		for(size_t i=0; i<tiles.size(); ++i)
		{
			tiles[i]->can_go_on_building = true;
		}
		
		switch(type)
		{
			case(SOUTHWEST_TOWER):
			{
				doors_tile = session->tile_list[tile_y][tile_x].get();
			}
			break;
			case(NORTHWEST_TOWER):
			{
				doors_tile = session->tile_list[tile_y][tile_x-1].get();
			}
			break;
			case(NORTHEAST_TOWER):
			{
				doors_tile = session->tile_list[tile_y-1][tile_x-1].get();
			}
			break;
			case(SOUTHEAST_TOWER):
			{
				doors_tile = session->tile_list[tile_y-1][tile_x].get();
			}
			break;
			default:
				throw std::exception();
		}
	}
}

tower::~tower()
{
	if(is_real)
	{
		std::vector<tile*> tiles = tiles_under_building(tile_x, tile_y, FOUR_TILE_BUILDING);
		for(size_t i=0; i<tiles.size(); ++i)
			tiles[i]->can_go_on_building = false;
	}
}

std::vector<game_object*> tower::draw(int screen_position_x, int screen_position_y)
{
	int drawing_x = game_x - screen_position_x - 96;
	int drawing_y = game_y - screen_position_y - show_surface_height()*32 - 80 - drawing_floor*32;
	int region_y;

	if(draw_selection)
	{
		draw_selection = false;
		al_draw_bitmap_region(image_list[BUILDING_SELECTION_IMAGE], size * 192, 0, 192, 128, drawing_x, drawing_y, 0);
	}

	switch(type)
	{
		case(NORTHWEST_TOWER):
			region_y = 128;
			break;
		case(NORTHEAST_TOWER):
			region_y = 256;
			break;
		case(SOUTHEAST_TOWER):
			region_y = 384;
			break;
		case(SOUTHWEST_TOWER):
			region_y = 0;
			break;
		default:
			throw std::exception();
	}
	
	ALLEGRO_COLOR color;
	if(is_real)
		color = WHITE_COLOR;

	else if(draw_green)
		color = LIGHT_GREEN_COLOR;
	
	else
		color = LIGHT_RED_COLOR;

	
	if(drawing_floor + 1 < number_of_floors)
	{
		al_draw_tinted_bitmap_region(image_list[image], color, 64 + 192*drawing_floor, region_y, 192, 128, drawing_x, drawing_y, 0);
	}
	else if(drawing_floor + 1 == number_of_floors)
	{
		al_draw_tinted_bitmap_region(image_list[image], color, 64 + 192*drawing_floor, region_y, 192, 128, drawing_x, drawing_y, 0);
		auto it = remove_if(people_to_draw.begin(), people_to_draw.end(), [](boost::weak_ptr<people> p){return p.expired();});
		people_to_draw.erase(it, people_to_draw.end());
		
		for(size_t i=0; i<people_to_draw.size(); ++i)
		{
			people_to_draw[i].lock()->draw(screen_position_x, screen_position_y);
		}
		people_to_draw.clear();

		al_draw_tinted_bitmap_region(image_list[image], color, 256 + 192*drawing_floor, region_y, 192, 128, drawing_x, drawing_y, 0);
	}
	
	drawing_floor++;

	if(drawing_floor < number_of_floors)			//this is necessary for right perspective when drawing high buildings  
	{
		return std::vector<game_object*>(1, this);
	}

	drawing_floor = 0;
	return std::vector<game_object*>();
}

void tower::rotate(int tile_x, int tile_y, bool clockwise)
{
	building::rotate(tile_x, tile_y, clockwise);
	if(clockwise)
	{
		if(type == SOUTHWEST_TOWER)
			type = NORTHWEST_TOWER;

		else if(type == NORTHWEST_TOWER)
			type = NORTHEAST_TOWER;

		else if(type == NORTHEAST_TOWER)
			type = SOUTHEAST_TOWER;

		else if(type == NORTHWEST_TOWER)
			type = SOUTHWEST_TOWER;
	}
	else
	{
		if(type == SOUTHWEST_TOWER)
			type = SOUTHEAST_TOWER;

		else if(type == SOUTHEAST_TOWER)
			type = NORTHEAST_TOWER;

		else if(type == NORTHEAST_TOWER)
			type = NORTHWEST_TOWER;

		else if(type == NORTHWEST_TOWER)
			type = SOUTHWEST_TOWER;
	}
}

barracks::barracks(building_type type, int tile_x, int tile_y, int surface_height, player owner, bool is_real) 
			: building(type, tile_x, tile_y, surface_height, owner, is_real), 
				output(new carrier_output(building_info::show_building_info(type).capacity, std::vector<resources>{MILK, FISH, MEAT, FRUIT, BREAD, WEAPONS, BOWS, PLATE_ARMOR}, 
					std::vector<resources>(0), building_info::show_building_info(type).number_of_carriers))
{
	number_of_functions = 1;
}

/*Checks time to create warrior ot archer and eventually creates them.*/
void barracks::specific_update()
{
	if(stopped)
		return;

	update_supported_units();

	if(action_duration > 0)
		action_duration--;
	
	if((action_duration == 0) && (actions.size() > 0) && (supported_units.size() < max_units[upgrade_level]))
	{
		boost::weak_ptr<warrior> new_warrior = warrior_born_near(actions[0], tile_x, tile_y, owner);
		if(!new_warrior.expired())
		{
			std::pair<boost::weak_ptr<warrior>, int> p(new_warrior, 0);
			supported_units.push_back(p);
		}

		actions.erase(actions.begin());

		if(actions.size() > 0)
			action_duration = time_to_make_unit;
	}
}

/*Is called if player clicked on button on this building. It handles the click, for example starts creating warrior.*/ 
void barracks::function_click(int mouse_x, int mouse_y)
{
	building::function_click(mouse_x, mouse_y);
	int number_of_button = compute_button_number(mouse_x, mouse_y);

	if(number_of_button >= number_of_functions)		
	{
		if(actions.size() > 0)
		{
			int start_x = display_width - 7*BUTTON_SIZE;
			int erase_index = -1;

			if((mouse_x > start_x) && (mouse_x < start_x + BUTTON_SIZE/2) && (mouse_y > display_height - BUTTON_SIZE) && (mouse_y < display_height - 40))
				erase_index = 0;				

			for(int i = 1; i<actions.size(); i++)
			{
				if(i > max_actions)
					break;

				if((mouse_x > start_x + (i-1)*40) && (mouse_x < start_x + i*40) && (mouse_y > display_height - BUTTON_SIZE/2) && (mouse_y < display_height))
					erase_index = i;
			}	
			
			if(erase_index != -1)
			{
				people_info erased_info = people_info::show_people_info(actions[erase_index]);
				output->save_list(erased_info.price);
				session->honour += erased_info.honour_price;
	
				actions.erase(actions.begin() + erase_index);
				if(actions.empty())
					action_duration = 0;
				
				else if(erase_index == 0)
					action_duration = time_to_make_unit;
			}
		}
	}
	else if(number_of_button < number_of_functions)
	{
		people_type people_created;
		switch(number_of_button)
		{
			case(0):
				people_created = SPEARMAN;
				break;
			case(1):
				people_created = AXEMAN;
				break;
			case(2):
				people_created = BOWMAN;
				break;
			case(3):
				people_created = SWORDSMAN;
				break;
		}

		people_info info = people_info::show_people_info(people_created);

		bool success = false;

		if((actions.size() < max_actions) && (actions.size() + supported_units.size() < max_units[upgrade_level]))
		{
			if(session->honour >= info.honour_price)
			{
				if(output->try_subtract_list(info.price))
				{
					session->honour -= info.honour_price;
					actions.push_back(people_created);
					success = true;
				}
			}
		}

		if((success) && (actions.size() == 1))
			action_duration = time_to_make_unit;
	}
	return;
}

void barracks::specific_draw_function_info(int mouse_x, int mouse_y)
{
	int number_of_button = compute_button_number(mouse_x, mouse_y);
	
	if(number_of_button >= number_of_functions)
		return;

	people_type type;

	switch(number_of_button)
	{
		case(0):
			type = SPEARMAN;
		break;
		case(1):
			type = AXEMAN;
		break;
		case(2):
			type = BOWMAN;
		break;
		case(3):
			type = SWORDSMAN;
		break;
		default:
		{
			throw std::exception();
			return;
		}
	}

	people_info info = people_info::show_people_info(type);
	
	button::draw_button_info(info.name, info.text, info.price, info.honour_price, 0, number_of_button*BUTTON_SIZE);

	return;
}

/*Draws buttons to create warrior and archer. Also draws time bar if something is created now and queue of action which it will work on later. */
void barracks::draw_specific_interface()
{
	std::vector<people_type> functions{SPEARMAN, AXEMAN, BOWMAN, SWORDSMAN};

	for(int i = 0; i<number_of_functions; i++)
	{
		al_draw_filled_rectangle(i*BUTTON_SIZE + 5, display_height - BUTTON_SIZE + 5, (i + 1)*BUTTON_SIZE -5 , display_height - 5, BLACK_COLOR);
		ALLEGRO_BITMAP* b = image_list[people_info::show_people_info(functions[i]).image];
		al_draw_bitmap_region(b, 0, 0, 64, 64, i*BUTTON_SIZE + 8, display_height - BUTTON_SIZE + 8, 0);
		al_draw_textf(font15, WRITING_COLOR, i*BUTTON_SIZE + 72, display_height - 23, ALLEGRO_ALIGN_RIGHT, "%i", i + 1);
	}

	al_draw_textf(font15, WRITING_COLOR, number_of_functions * BUTTON_SIZE, display_height - BUTTON_SIZE + 50, ALLEGRO_ALIGN_LEFT, "Space for %lu soldiers.", max_units[upgrade_level] - supported_units.size() - actions.size());

	output->draw_nonzero_resources(number_of_functions * BUTTON_SIZE, true);

	if(actions.size() > 0)
	{
		int start_x = display_width - 7*BUTTON_SIZE;
		al_draw_filled_rectangle(start_x + 5, display_height - BUTTON_SIZE + 5, start_x + BUTTON_SIZE/2 - 5, display_height - 45, BLACK_COLOR);
		
		ALLEGRO_BITMAP* function_picture = image_list[people_info::show_people_info(actions[0]).image];
		al_draw_scaled_bitmap(function_picture, 0, 0, 64, 64, start_x + 4, display_height - BUTTON_SIZE, 32, 32, 0);
		
		button::draw_progress_bar(start_x + 42, display_height - BUTTON_SIZE + 10, 100 - (100*action_duration)/time_to_make_unit);

		for(int i = 1; i<actions.size(); i++)
		{
			if(i > max_actions)
				break;
			al_draw_filled_rectangle(start_x + (i-1)*40 + 5, display_height - BUTTON_SIZE/2 + 5, start_x + i*40 - 5, display_height - 5, BLACK_COLOR);
			function_picture = image_list[people_info::show_people_info(actions[i]).image];
			al_draw_scaled_bitmap(function_picture, 0, 0, 64, 64, start_x + 4 + 40*(i-1), display_height - BUTTON_SIZE/2, 32, 32, 0);
		}
	}
}

void barracks::upgrade()
{	
	number_of_functions++;
}

void barracks::check_death_supported_units()
{
	auto it = remove_if(supported_units.begin(), supported_units.end(), [](std::pair<boost::weak_ptr<warrior>, int> w) {return w.first.expired();});
	supported_units.erase(it, supported_units.end());
}

void barracks::update_supported_units()
{
	check_death_supported_units();

	std::vector<resources> food_types{MILK, FISH, MEAT, FRUIT, BREAD};
	int food_type_index = 0;

	for(size_t i=0; i<supported_units.size(); ++i)
	{
		if(supported_units[i].second == 0)
		{
			bool done = false;
			while((!done) && (food_type_index < food_types.size()))
			{
				done = output->try_subtract(food_types[food_type_index], 1);
				if(!done)
					food_type_index++;

			}
			if(done)
			{
				supported_units[i].first.lock()->starving = false;
				supported_units[i].second = time_to_support_unit;
			}
			else	
				supported_units[i].first.lock()->starving = true;
		}
		else
			supported_units[i].second--;
	}
}

/*Initialize warehouse. Number of starting resources is defined at building.h file.*/
warehouse::warehouse(building_type type, int tile_x, int tile_y, int surface_height, player owner, bool is_real) 
		: building(type, tile_x, tile_y, surface_height, owner, is_real)
{
	if(is_real)
	{
		output = session->global_stock;
	}

}

/* Draw number of resources that are stored in warehouse. */
void warehouse::draw_specific_interface()
{
	output->draw_nonzero_resources(15, false);		//15 is number of pixels from left border of display
}

/*Initialize production_buidling. Production building has 4 subtypes: DAIRY_FARM, WOODCUTTER, QUARRY and IRON_MINE.*/ 
production_building::production_building(building_type type, int tile_x, int tile_y, int surface_height, player owner, bool is_real) 
			: building(type, tile_x, tile_y, surface_height, owner, is_real), work_done(0), status(production_building_status::NO_PATH)
				
{
	//ALLEGRO_BITMAP* image;

	switch(type)
	{
	case(QUARRY):
		{
			resource_produced = STONE;
		}
		break;
	case(WOODCUTTER):
		{
			resource_produced = WOOD;
		}
		break;
	case(HUNTER):
		{
			resource_produced = MEAT;
		}
		break;
	case(FISHERMAN):
		{
			resource_produced = FISH;
		}
		break;	
	case(APPLE_FARM):
		{
			resource_produced = FRUIT;
		}
		break;	
	case(DAIRY_FARM):
		{
			resource_produced = MILK;
		}
		break;
	case(WHEAT_FARM):
		{
			resource_produced = WHEAT;
		}
		break;
	case(WINDMILL):
		{
			resource_produced = BREAD;
			needed_resources.push_back(WHEAT);
		}
		break;
	case(CLAY_PIT):
		{
			resource_produced = CLAY;
		}
		break;
	case(POTTERY_WORKSHOP):
		{
			resource_produced = POTTERY;
			needed_resources.push_back(CLAY);
		}
		break;
	case(BRICKMAKER):
		{
			resource_produced = BRICKS;
			needed_resources.push_back(CLAY);
		}
		break;
	case(MARBLE_QUARRY):
		{
			resource_produced = MARBLE;
		}
		break;
	case(GOLD_MINE):
		{
			resource_produced = GOLD;
		}
		break;
	case(COAL_MINE):
		{
			resource_produced = COAL;
		}
		break;
	case(IRON_MINE):
		{
			resource_produced = IRON;
		}	
		break;
	case(SMITH):
		{
			resource_produced = WEAPONS;
			needed_resources.push_back(IRON);
		}	
		break;
	case(ARMOURER):
		{
			resource_produced = PLATE_ARMOR;
			needed_resources.push_back(IRON);
			needed_resources.push_back(COAL);
		}	
		break;
	case(FLETCHER):
		{
			resource_produced = BOWS;
		}	
		break;
	default:
	{
		LOG("production_building::production_building - unknown type of building");
		throw std::exception();
	}
	}
	
	int capacity = building_info::show_building_info(type).capacity;
	int number_of_carriers = building_info::show_building_info(type).number_of_carriers;
	output = boost::shared_ptr<carrier_output>(new carrier_output(capacity, needed_resources, std::vector<resources>(1, resource_produced), number_of_carriers));

	//current_time_to_produce = time_to_produce;
	amount_produced = 1;
}

/*This function should be called once per frame for every buidling. Updates building.*/
void production_building::update()
{
	if(bIs_death)
		return;
	
	if(stopped)
		status = production_building_status::STOPPED;
	
	if(required_workers != actual_workers)
	{
		bool adjacent_path = false;
		std::vector<tile*> building_tiles = tiles_under_building(shared_from_this());
		for(tile* t : building_tiles)
		{
			for(tile* n : t->accessible_neighbours)
			{
				if(n->real_path_on_tile())
				{
					adjacent_path = true;
					break;
				}
			}
			if(adjacent_path)
				break;
		}

		if(adjacent_path)
			status = production_building_status::NO_WORKERS_NEARBY;

		else
			status = production_building_status::NO_PATH;
	}

	if((required_workers == actual_workers) && (!stopped))
		specific_update();
	
	if(!stopped)
		show_carrier_output()->update();
}

/*Decrease time left to produce resources and if they are finished sends them to granary or warehouse*/
void production_building::specific_update()
{
	if(work_done < time_to_produce)
	{
		work_done += ((double)(upgrade_level + 1)) * (std::max(1.0, (double)session->show_happiness()) / 100.0);
		status = production_building_status::WORKING;
	}

	else if(output->show_amount(resource_produced) < output->show_capacity())
	{
		bool enough_resources = true;
		for(size_t i=0; i<needed_resources.size(); ++i)
		{
			if(output->show_amount(needed_resources[i]) < amount_produced)
			{
				enough_resources = false;
				missing_resource = needed_resources[i];
			}
		}
		
		if(enough_resources)
		{
			for(size_t i=0; i<needed_resources.size(); ++i)
				output->try_subtract(needed_resources[i], amount_produced);

			output->save(resource_produced, amount_produced);
			work_done = 0;
			status = production_building_status::WORKING;
		}
		else
		{
			status = production_building_status::NO_BASIC_RESOURCES;
		}
	}
	else
	{
		status = production_building_status::NO_CAPACITY;
	}
}

/* Draws how much time is left to create resource. */
void production_building::draw_specific_interface()
{
	al_draw_bitmap_region(image_list[RESOURCES_IMAGE], resource_produced*30, 0, 30, 30, 50, display_height - BUTTON_SIZE + 10, 0);

	int start_x = 100;
	
	button::draw_progress_bar(start_x, display_height - BUTTON_SIZE + 10, 100* (work_done / ((double)time_to_produce)));
	
		//this draws stocks
	output->draw_nonzero_resources(300, true);

	std::string text;

	switch(status)
	{
	case(production_building_status::WORKING):
	{
		text = "Working on " + std::to_string(session->show_happiness()) + "% (based on happiness).";
	}
		break;
	case(production_building_status::NO_CAPACITY):
		text = "Storage in this building is full.";
		break;
	case(production_building_status::NO_BASIC_RESOURCES):
		text = "Missing " + std::get<1>(find<resources_names, 0>(missing_resource));
		break;
	case(production_building_status::STOPPED):
		text = "Stopped";
		break;
	case(production_building_status::NO_WORKERS_NEARBY):
		text = "Not enough workers live nearby. Try building houses.";
		break;
	case(production_building_status::NO_PATH):
		text = "Building is not connected to path network.";
		break;
	default:
		throw std::exception();
	}
				//draws building status
	al_draw_textf(font15, WRITING_COLOR, 20, display_height - 30, ALLEGRO_ALIGN_LEFT, "%s", text.c_str());
}
	
void production_building::upgrade()
{
	output->increase_capacity(building_info::show_building_info(type).capacity * (upgrade_level + 1));
}

wall::wall(building_type type, int tile_x, int tile_y, int surface_height, player owner, bool is_real) 
			: building(type, tile_x, tile_y, surface_height, owner, is_real)
{
	//required_workers = 0;
	
	for(size_t i=0; i<4; ++i)
		borders.push_back(false);

	if(is_real)
		session->tile_list[tile_y][tile_x]->can_go_on_building = true;
}

wall::~wall()
{
	if(is_real)
		session->tile_list[tile_y][tile_x]->can_go_on_building = false;
}

/* If there are walls or gates in adjacent tiles, then this function changes drawing of wall.*/
void wall::specific_update()
{
	std::vector<std::pair<int, int>> coordinates_change;
	coordinates_change.push_back(std::pair<int, int>(-1,0));
	coordinates_change.push_back(std::pair<int, int>(0,1));
	coordinates_change.push_back(std::pair<int, int>(1,0));
	coordinates_change.push_back(std::pair<int, int>(0,-1));

	for(size_t i=0; i<4; ++i)
	{
		int changed_tile_y = tile_y + coordinates_change[i].first;
		int changed_tile_x = tile_x + coordinates_change[i].second;
		borders[i] = false;

		if(((changed_tile_y >= 0) && (changed_tile_y < game_info::map_height)) && ((changed_tile_x >= 0) && (changed_tile_x < game_info::map_width)))
		{	
			boost::shared_ptr<building> adjacent_building = session->tile_list[changed_tile_y][changed_tile_x]->building_on_tile.lock();
			if(adjacent_building)
			{
				building_type adjacent = adjacent_building->show_type();
				if((adjacent == WALL) || (adjacent == LEFT_GATE) || (adjacent == RIGHT_GATE) || (adjacent == PALISADE) ||
			(adjacent == NORTHWEST_STAIRS) || (adjacent == NORTHEAST_STAIRS) || (adjacent == SOUTHEAST_STAIRS) || (adjacent == SOUTHWEST_STAIRS))
					borders[i] = true;
			}
		}
	}
}

std::vector<game_object*> wall::draw(int screen_position_x, int screen_position_y)
{
	int drawing_x = game_x - screen_position_x - 32;
	int drawing_y = game_y - screen_position_y - show_surface_height()*32 - 16 - drawing_floor*32;
	
	if(draw_selection)
	{
		draw_selection = false;
		al_draw_bitmap_region(image_list[BUILDING_SELECTION_IMAGE], size * 192, 0, 192, 128, drawing_x - 64, drawing_y - 64, 0);
	}

	ALLEGRO_COLOR color;

	if(is_real)
		color = WHITE_COLOR;
	else if(draw_green)
		color = LIGHT_GREEN_COLOR;
	else
		color = LIGHT_RED_COLOR;

	if(drawing_floor == 0)
	{
		al_draw_tinted_bitmap_region(image_list[image], color, 0, 0, 64, 64, drawing_x, drawing_y, 0);
	}
	else if(drawing_floor == 1)
	{
		for(int i : {0, 3, 2, 1})
		{
			if(!borders[i])
				al_draw_tinted_bitmap_region(image_list[image], color, 64*(i+1), 0, 64, 64, drawing_x, drawing_y, 0);

			if(i == 3)		//draw people between walls
			{
				auto it = remove_if(people_to_draw.begin(), people_to_draw.end(), [](boost::weak_ptr<people> p){return p.expired();});
				people_to_draw.erase(it, people_to_draw.end());

				for(size_t j=0; j<people_to_draw.size(); ++j)
				{
					people_to_draw[j].lock()->draw(screen_position_x, screen_position_y);
				}
				people_to_draw.clear();
			}
		}
	}
	
	drawing_floor++;

	if(drawing_floor < number_of_floors)			//this is necessary for right perspective when drawing high buildings  
	{
		return std::vector<game_object*>(1, this);
	}
	
	drawing_floor = 0;
	return std::vector<game_object*>(0);
}

gate::gate(building_type type, int tile_x, int tile_y, int surface_height, player owner, bool is_real) 
			: building(type, tile_x, tile_y, surface_height, owner, is_real)
{
	if(type == LEFT_GATE)
		size = LEFT_GATE_BUILDING;
	else
		size = RIGHT_GATE_BUILDING;
	
	open = true;
	image_to_draw = 0;
	
	if(!is_real)
		return;

	set_gate_tile();	
}

gate::~gate()
{
	if(is_real)
	{
		gate_tile->can_go_inside_building = false;
	}
}

//It just control right drawing of gate. For opening and closing gate see gate::function_click().
void gate::specific_update()
{
	if(gate_tile->building_on_tile.expired())
		throw std::exception();

	if(action_duration > 0)				
		action_duration--;
	if(action_duration == 1)
	{
		if((open) & (image_to_draw > 0))
		{
			image_to_draw--;
			action_duration = 10;
		}
		else if((!open) & (image_to_draw < 3))
		{
			image_to_draw++;
			action_duration = 10;
		}
	}
}

void gate::draw_specific_interface()
{
	al_draw_bitmap_region(image_list[OPEN_GATE_IMAGE], 0, 0, 64, 64, 8, display_height - BUTTON_SIZE + 8, 0);

	al_draw_bitmap_region(image_list[OPEN_GATE_IMAGE], 64, 0, 64, 64, BUTTON_SIZE + 8, display_height - BUTTON_SIZE + 8, 0);
	
	for(int i=0; i<2; i++)	//draws shortcut numbers
	{
		al_draw_textf(font15, WRITING_COLOR, i*BUTTON_SIZE + 72, display_height - 23, ALLEGRO_ALIGN_RIGHT, "%i", i + 1);
	}
}

/*Opens or closes gate.*/
void gate::function_click(int mouse_x, int mouse_y)
{
	building::function_click(mouse_x, mouse_y);
	int number_of_function = compute_button_number(mouse_x, mouse_y);

	if(number_of_function == 0)
	{
		open = true;
		action_duration = 10;
		
		gate_tile->can_go_inside_building = true;
	}
	else if(number_of_function == 1)
	{
		open = false;
		action_duration = 10;
		
		gate_tile->can_go_inside_building = false;
	}
	return;
}

std::vector<game_object*> gate::draw(int screen_position_x, int screen_position_y)
{
	int drawing_x = game_x - screen_position_x - 96;
	int drawing_y = game_y - screen_position_y - show_surface_height()*32 - 80;
	
	if(draw_selection)
	{
		draw_selection = false;
		al_draw_bitmap_region(image_list[BUILDING_SELECTION_IMAGE], size * 192, 0, 192, 128, drawing_x, drawing_y, 0);
	}

	if(type == RIGHT_GATE)
	{
		drawing_x += 64;
		drawing_y += 0;
	}
	
	if(is_real)
	{
		if(type == RIGHT_GATE)	//entrance is to right
		{
			image_to_draw += 4;
		}

		if(drawing_floor == 0)
		{
			al_draw_bitmap_region(image_list[image], image_to_draw*128 + 64, 128, 128, 128, drawing_x, drawing_y, 0);

			al_draw_bitmap_region(image_list[image], image_to_draw*128 + 64, 0, 128, 128, drawing_x, drawing_y, 0);
		}
		else
		{
			al_draw_bitmap_region(image_list[image], image_to_draw*128 + 64, 256, 128, 128, drawing_x, drawing_y, 0);
		}
			
		if(type == RIGHT_GATE)
			image_to_draw -= 4;

	}
	else 
	{
		if(type == RIGHT_GATE)
		{
			image_to_draw = 4;
		}
		else if(type == LEFT_GATE)
			image_to_draw = 0;
		
		ALLEGRO_COLOR color;
		if(draw_green)
			color = LIGHT_GREEN_COLOR;
		else
			color = LIGHT_RED_COLOR;
		
		if(drawing_floor == 0)
		{
			al_draw_tinted_bitmap_region(image_list[image], color, image_to_draw*128 + 64, 128, 128, 128, drawing_x, drawing_y, 0);
			al_draw_tinted_bitmap_region(image_list[image], color, image_to_draw*128 + 64, 0, 128, 128, drawing_x, drawing_y, 0);
		}
		else
			al_draw_tinted_bitmap_region(image_list[image], color, image_to_draw*128 + 64, 256, 128, 128, drawing_x, drawing_y, 0);
	}

	drawing_floor++;
	if(drawing_floor == 1)
	{
		return std::vector<game_object*>{this};
	}

	drawing_floor = 0;
	
	return std::vector<game_object*>();
}

void gate::rotate(int new_tile_x, int new_tile_y, bool clockwise)
{
	if(type == RIGHT_GATE)
	{
		type = LEFT_GATE;
		if(clockwise)
			new_tile_x += 2;
	}

	else if(type == LEFT_GATE)
	{
		type = RIGHT_GATE;
		if(!clockwise)
			new_tile_y += 2;
	}
	
	game_object::update(new_tile_x, new_tile_y, show_surface_height());		//this will also change game_x and game_y
}

void gate::set_gate_tile()
{
	if(!is_real)
		return;

	if(type == LEFT_GATE)
		gate_tile = session->tile_list[tile_y][tile_x - 1].get();
	
	else
		gate_tile = session->tile_list[tile_y - 1][tile_x].get();

	gate_tile->add_path(is_real);
	gate_tile->can_go_inside_building = true;
}
		
house::house(building_type type, int tile_x, int tile_y, int surface_height, player owner, bool is_real) 
				: building(type, tile_x, tile_y, surface_height, owner, is_real)
{	idle_workers = workers_by_house_level[0];
	starving = false;
}

void house::specific_update()
{
	if(action_duration > 1)
	{
		action_duration--;
		return;
	}

	action_duration = TIME_TO_EAT;

	bool needs_pottery = false;
	int types_of_food = upgrade_level + 1;
	if(upgrade_level == 3)
	{
		needs_pottery = true;
		types_of_food--;
	}
	std::vector<resources> food_types{MILK, MEAT, FRUIT, BREAD, FISH};
	std::vector<resources> received_food_types;

	std::vector<tile*> house_tiles = tiles_under_building(tile_x, tile_y, size);
	std::vector<std::pair<boost::shared_ptr<building>, int>> possible_markets = buildings_connected_by_path(house_tiles, HOUSE_MARKET_DISTANCE);

	has_market = false;
	
	for(size_t i=0; i<possible_markets.size(); ++i)
	{
		if(possible_markets[i].first->type == MARKET)
		{
			has_market = true;
			market* near_market = dynamic_cast<market*>(possible_markets[i].first.get());

			if(possible_markets[i].second <= near_market->show_max_distance_to_house())
			{
				for(size_t j=0; j<food_types.size(); ++j)
				{
					if((!contains(received_food_types, food_types[j])) 
							&& (near_market->show_carrier_output()->try_subtract(food_types[j], 1)) && (types_of_food > 0))
					{			
						types_of_food--;
						received_food_types.push_back(food_types[j]);
					}
				}
				if((needs_pottery) && (near_market->show_carrier_output()->try_subtract(POTTERY, 1)))
					needs_pottery = false;

				if((!needs_pottery) && (types_of_food == 0))		//house have everytihng it needs
					break;
			}
		}
	}
	
	missing_resources = types_of_food;
	if(needs_pottery)
		missing_resources++;
}

void house::draw_specific_interface()
{
	al_draw_textf(font15, WRITING_COLOR, 20, display_height - BUTTON_SIZE + 10, ALLEGRO_ALIGN_LEFT, "Base happiness modifier: %i", base_happiness[upgrade_level]);

	if(!has_market)
		al_draw_textf(font15, WRITING_COLOR, 20, display_height - BUTTON_SIZE  + 50, ALLEGRO_ALIGN_LEFT, "No market nearby - happiness: -%i", 5*missing_resources);

	else if(missing_resources > 0)
		al_draw_textf(font15, WRITING_COLOR, 20, display_height - BUTTON_SIZE  + 50, ALLEGRO_ALIGN_LEFT, "Not enough resources in near market - happiness: -%i", 5*missing_resources);

	button::draw_progress_bar(350, display_height - BUTTON_SIZE + 10, 100 - (action_duration*100) / TIME_TO_EAT);
	
	al_draw_textf(font15, WRITING_COLOR, 500, display_height - BUTTON_SIZE + 10, ALLEGRO_ALIGN_LEFT, "Idle workers: %i", idle_workers);
	al_draw_textf(font15, WRITING_COLOR, 500, display_height - BUTTON_SIZE + 50, ALLEGRO_ALIGN_LEFT, "Max workers: %i", workers_by_house_level[upgrade_level]);
}

market::market(building_type type, int tile_x, int tile_y, int surface_height, player owner, bool is_real) 
			: building(type, tile_x, tile_y, surface_height, owner, is_real) 
{
	std::vector<resources> in{MILK, FISH, MEAT, FRUIT, BREAD, POTTERY};
	building_info info = building_info::show_building_info(type);
	output = boost::shared_ptr<carrier_output>(new carrier_output(info.capacity, in, std::vector<resources>(0), info.number_of_carriers)); 

	//required_workers = 0;
	max_distance_to_house = 10;	//increased by upgrade
}

void market::draw_specific_interface()
{
	output->draw_nonzero_resources(15, true);
}

void market::upgrade()
{
	max_distance_to_house += upgrade_house_distance_increase;
	output->increase_capacity(output->show_capacity() + upgrade_capacity_increase);
}

great_hall::great_hall(building_type type, int tile_x, int tile_y, int surface_height, player owner, bool is_real) 
			: building(type, tile_x, tile_y, surface_height, owner, is_real)
{	
	std::vector<resources> in{MILK, FISH, MEAT, FRUIT, BREAD, POTTERY};
	building_info info = building_info::show_building_info(type);
	output = boost::shared_ptr<carrier_output>(new carrier_output(info.capacity, in, std::vector<resources>(0), info.number_of_carriers)); 

	if(is_real)
		session->honour += honour_for_upgrade[0];
}

void great_hall::draw_specific_interface()
{
	for(size_t i=0; i<upgrade_level + 1; ++i)
	{
		al_draw_bitmap_region(image_list[FEAST_IMAGE], 64*i, 0, 64, 64, 8 + BUTTON_SIZE*i, display_height - BUTTON_SIZE + 8, 0);
	}

	output->draw_nonzero_resources((upgrade_level + 1)* BUTTON_SIZE + 10, true);
}

void great_hall::function_click(int mouse_x, int mouse_y)
{
	building::function_click(mouse_x, mouse_y);
	int feast_size = compute_button_number(mouse_x, mouse_y);
	
	if(feast_size > upgrade_level)
		return;

	int foods_in_stock = 0;

	std::vector<resources> food_types{MEAT, FISH, BREAD, FRUIT, MILK};
	for(size_t i=0; i<food_types.size(); ++i)
	{
		if(output->show_amount(food_types[i]) >= feast_size + food_for_smallest_feast)
		{
			foods_in_stock++;
		}
	}
	if(foods_in_stock < feast_size + 2)
		return;

	if(feast_size == 3) 
	{
		if(output->show_amount(POTTERY) < feast_size + food_for_smallest_feast)
			return;
		else
			output->try_subtract(POTTERY, feast_size + food_for_smallest_feast);
	}

	int food_subtracted = 0;

	for(size_t i=0; i<food_types.size(); ++i)
	{
		if(output->show_amount(food_types[i]) >= feast_size + food_for_smallest_feast)
		{
			output->try_subtract(food_types[i], feast_size + food_for_smallest_feast);
			food_subtracted++;
			
			if(food_subtracted >= feast_size + 2)
				break;
		}
	}

	session->honour += honour_for_feast[feast_size];
	return;
}

void great_hall::specific_draw_function_info(int mouse_x, int mouse_y)
{
	int number_of_function = compute_button_number(mouse_x, mouse_y);
	if(number_of_function > upgrade_level)
		return;

	std::vector<int> prices(NUMBER_OF_RESOURCES, 0);
	std::string name, text;

	switch(number_of_function)
	{
		case(0):
		{
			name = "Small feast";
			text = "Small feast needs 2 units of 2 types of food. It will give you 5 honour.";
		}
		break;
		case(1):
		{
			name = "Medium feast";
			text = "Medium feast needs 3 units of 3 types of food. It will give you 20 honour.";
		}
		break;
		case(2):
		{
			name = "Large feast";
			text = "This feast needs 4 units of 4 types of food. It will give you 50 honour.";
		}
		break;
		case(3):
		{
			name = "Huge feast";
			text = "Biggest feast needs 5 units of food for each food type. You will receive 100 honour for it.";
		}
		break;
		default:
			throw std::exception();
	}

	button::draw_button_info(name, text, prices, 0, 0, number_of_function*BUTTON_SIZE);
}

void great_hall::upgrade()
{
	output->increase_capacity(1 + output->show_capacity());
	session->honour += honour_for_upgrade[upgrade_level];
}

church::church(building_type type, int tile_x, int tile_y, int surface_height, player owner, bool is_real) 
			: building(type, tile_x, tile_y, surface_height, owner, is_real)
{
}

void church::specific_update()
{
	if(action_duration == 0)
	{
		session->honour += church::honour_produced[upgrade_level];
		action_duration = time_to_produce_honour;
	}
	else
		action_duration--;
}

void church::draw_specific_interface()
{
	button::draw_progress_bar(15, display_height - BUTTON_SIZE + 10, (100*(time_to_produce_honour - action_duration))/time_to_produce_honour, 100, 20);
}

store::store(building_type type, int tile_x, int tile_y, int surface_height, player owner, bool is_real) 
			: building(type, tile_x, tile_y, surface_height, owner, is_real)
{
	building_info info = building_info::show_building_info(type);
	output = boost::shared_ptr<carrier_output>(new carrier_output(info.capacity, std::vector<resources>(), std::vector<resources>(), info.number_of_carriers)); 
	
	for(size_t i=0; i<NUMBER_OF_RESOURCES; ++i)
	{
		buying.push_back(resource_status::NOT_TRADING);
	}
}

void store::specific_update()
{
	if(action_duration > 0)
	{
		action_duration--;
		return;
	}
	
	for(size_t i=0; i<buying.size(); ++i)
	{
		resources r = static_cast<resources>(i);
		if(buying[i] == resource_status::SELLING)
		{
			if((output->show_usable_amount(r) >= show_selling_prices()[i]) && (output->show_free_space(GOLD) > 1))
			{
				output->save(GOLD, 1);
				assert(output->try_subtract(r, show_selling_prices()[i]));
			}
		}
		else if(buying[i] == resource_status::BUYING)
		{
			if((output->show_usable_amount(GOLD) >= show_buying_prices()[i]) && (output->show_free_space(r) > 1))
			{
				assert(output->try_subtract(GOLD, show_buying_prices()[i]));
				output->save(r, 1);
			}
		}
	}
	action_duration = time_to_trade[upgrade_level];
}

void store::draw_specific_interface()
{
	al_draw_filled_rectangle(5, display_height - BUTTON_SIZE + 5, BUTTON_SIZE - 5 , display_height - 5, BLACK_COLOR);
	al_draw_bitmap(image_list[DETAILS_IMAGE], 8, display_height - BUTTON_SIZE + 8, 0);
	al_draw_textf(font15, WRITING_COLOR, 72, display_height - 23, ALLEGRO_ALIGN_RIGHT, "%i", 1);

	output->draw_nonzero_resources(100, true);
	
	button::draw_progress_bar(100, display_height - 40, (100 * (time_to_trade[upgrade_level] - action_duration)) / time_to_trade[upgrade_level], 50, 20);
}

void store::function_click(int mouse_x, int mouse_y)
{
	building::function_click(mouse_x, mouse_y);

	if(compute_button_number(mouse_x, mouse_y) == 0)
	{
		boost::shared_ptr<store> ptr = boost::dynamic_pointer_cast<store>(shared_from_this());
		window::active_windows.push_back(boost::shared_ptr<store_window>(new store_window(ptr)));
	}
}

void store::specific_draw_function_info(int mouse_x, int mouse_y)
{
	if(compute_button_number(mouse_x, mouse_y) == 0)
	{
		button::draw_button_info("Details", "Show more details about store.", std::vector<int>(NUMBER_OF_RESOURCES, 0), 0, 0, 0);
	}
}

void store::upgrade()
{
	output->increase_capacity(output->show_capacity() + 5);
}

void store::draw_window(int start_x, int start_y)
{
	int drawn = 0;
	for(size_t i=0; i<NUMBER_OF_RESOURCES; ++i)
	{
		if(static_cast<resources>(i) != GOLD)
		{	
			draw_button(start_x, drawn * line_heigth + start_y + y_offset, static_cast<resources>(i));
			++drawn;
		}
	}
}

void store::window_function_click(int relative_mouse_x, int relative_mouse_y)
{
	if((button_start_x > relative_mouse_x) || (button_start_x + button_length < relative_mouse_x))
		return;

	int drawn = 0;
	int button_number = -1;
	for(size_t i=0; i<NUMBER_OF_RESOURCES; ++i)
	{
		if(static_cast<resources>(i) != GOLD)
		{
			if((relative_mouse_y > drawn * line_heigth + button_start_y + y_offset) && (relative_mouse_y < (drawn + 1) * line_heigth + button_start_y + y_offset))
			{
				button_number = drawn;
			}
			drawn++;
		}
	}
	
	resources r = static_cast<resources>(button_number);
	switch(buying[button_number])
	{
		case(resource_status::BUYING):
		{
			buying[button_number] = resource_status::SELLING;
			output->change_accepted_resources(r, true, true);
			output->change_accepted_resources(r, false, false);
		}
		break;
		case(resource_status::SELLING):
		{
			buying[button_number] = resource_status::NOT_TRADING;
			output->change_accepted_resources(r, false, false);
			output->change_accepted_resources(r, true, false);
		}
		break;
		case(resource_status::NOT_TRADING):
		{
			buying[button_number] = resource_status::BUYING;
			output->change_accepted_resources(r, true, false);
			output->change_accepted_resources(r, false, true);
		}
	}

	bool buying_something = false;
	bool selling_something = false;
	for(int i=0; i<buying.size(); ++i)
	{
		if(buying[i] == resource_status::BUYING)
			buying_something = true;
		if(buying[i] == resource_status::SELLING)
			selling_something = true;
	}
	
	if(buying_something)
		output->change_accepted_resources(GOLD, true, true);
	else 
		output->change_accepted_resources(GOLD, true, false);
	
	if(selling_something)
		output->change_accepted_resources(GOLD, false, true);
	else
		output->change_accepted_resources(GOLD, false,false);

}

void store::draw_button(int x, int y, resources r)
{
	std::string text;
	switch(buying[r])
	{
		case(resource_status::BUYING):
			text = "Buying";
			break;
		case(resource_status::SELLING):
			text = "Selling";
			break;
		case(resource_status::NOT_TRADING):
			text = "Not trading";
			break;
		default:
			assert(false);
	}

	al_draw_bitmap_region(image_list[RESOURCES_IMAGE], r * 30, 0, 30, 30, x, y, 0);
	al_draw_textf(font15, WRITING_COLOR, x + 40, y + 10, ALLEGRO_ALIGN_LEFT, "%i/%i", output->show_amount(r), output->show_capacity());
	al_draw_filled_rectangle(x + button_start_x, y + button_start_y, x + button_start_x + button_length, y + button_start_y + button_height, BLACK_COLOR);
	al_draw_textf(font15, WRITING_COLOR, x + button_start_x, y + 10, ALLEGRO_ALIGN_LEFT, "%s", text.c_str());
	
	x += button_start_x + button_length;
	
	std::vector<ALLEGRO_COLOR> tint_colors(2, GREY_COLOR);
	std::vector<resources> first{GOLD, r};
	std::vector<resources> second{r, GOLD};
	std::vector<int> rate{show_buying_prices()[r], show_selling_prices()[r]};

	if(buying[r] == resource_status::BUYING)
		tint_colors[0] = WHITE_COLOR;

	if(buying[r] == resource_status::SELLING)
		tint_colors[1] = WHITE_COLOR;

	for(size_t i=0; i<2; ++i)
	{
		for(size_t j=0; j<rate[i]; ++j)
		{
			al_draw_tinted_bitmap_region(image_list[RESOURCES_IMAGE], tint_colors[i], first[i]*30, 0, 30, 30, x + 10*j, y, 0);
		}
		al_draw_text(font15, WRITING_COLOR, x + 70, y + 10, ALLEGRO_ALIGN_LEFT, " = ");
		
		al_draw_tinted_bitmap_region(image_list[RESOURCES_IMAGE], tint_colors[i], second[i]*30, 0, 30, 30, x + 90, y, 0);
		
		x += exchange_rate_length;
	}
}

std::vector<int> store::show_selling_prices()
{
	std::vector<int> result;
	for(size_t i=0; i<NUMBER_OF_RESOURCES; ++i)
	{
		if(static_cast<resources>(i) != GOLD)
			result.push_back(5 - upgrade_level);

		else 
			result.push_back(0);
	}
	return result;
}

std::vector<int> store::show_buying_prices()
{
	std::vector<int> result;
	for(size_t i=0; i<NUMBER_OF_RESOURCES; ++i)
	{
		if(static_cast<resources>(i) != GOLD)
			result.push_back(5 - upgrade_level);

		else 
			result.push_back(0);
	}
	return result;
}

scout::scout(building_type type, int tile_x, int tile_y, int surface_height, player owner, bool is_real) 
			: building(type, tile_x, tile_y, surface_height, owner, is_real)
{	
	std::vector<resources> all_resources;
	for(size_t i=0; i<NUMBER_OF_RESOURCES; ++i)
		all_resources.push_back(static_cast<resources>(i));

	building_info info = building_info::show_building_info(type);
	output = boost::shared_ptr<carrier_output>(new carrier_output(info.capacity, std::vector<resources>(0), all_resources, info.number_of_carriers)); 
}

void scout::draw_specific_interface()
{
}

stairs::stairs(building_type type, int tile_x, int tile_y, int surface_height, player owner, bool is_real) 
			: building(type, tile_x, tile_y, surface_height, owner, is_real)
{
	if(is_real)
	{
		session->tile_list[tile_y][tile_x]->can_go_on_building = true;
		session->tile_list[tile_y][tile_x]->stairs_on_tile = true;
	}
}

stairs::~stairs()
{
	if(is_real)
	{
		session->tile_list[tile_y][tile_x]->can_go_on_building = false;
		session->tile_list[tile_y][tile_x]->stairs_on_tile = true;
	}
}

std::vector<game_object*> stairs::draw(int screen_position_x, int screen_position_y)
{
	int region_start_x = (static_cast<int>(type) - static_cast<int>(NORTHWEST_STAIRS))*64;
	
	int drawing_x = game_x - screen_position_x - 32;
	int drawing_y = game_y - screen_position_y - show_surface_height()*32 - 16 - drawing_floor*32;
	
	if(draw_selection)
	{
		draw_selection = false;
		al_draw_bitmap_region(image_list[BUILDING_SELECTION_IMAGE], size * 192, 0, 192, 128, drawing_x - 64, drawing_y - 64, 0);
	}

	if(is_real)
	{
		al_draw_bitmap_region(image_list[image], region_start_x, 0, 64, 64, drawing_x, drawing_y, 0);
	}
	else
	{
		if(draw_green)
			al_draw_tinted_bitmap_region(image_list[image], LIGHT_GREEN_COLOR, region_start_x, 0, 64, 64, drawing_x, drawing_y, 0);

		else
			al_draw_tinted_bitmap_region(image_list[image], LIGHT_RED_COLOR, region_start_x, 0, 64, 64, drawing_x, drawing_y, 0);
	}

	return std::vector<game_object*>();
}

void stairs::rotate(int tile_x, int tile_y, bool clockwise)
{
	building::rotate(tile_x, tile_y, clockwise);

	if(clockwise)
	{
		if(type == SOUTHWEST_STAIRS)
			type = NORTHWEST_STAIRS;

		else 
			type = static_cast<building_type>(static_cast<int>(type) + 1);
	}
	else
	{
		if(type == NORTHWEST_STAIRS)
			type = SOUTHWEST_STAIRS;

		else
			type = static_cast<building_type>(static_cast<int>(type) - 1);
	}
}

/*Finds free tile which is nearest to tile defined by parametres tile_x a tile_y and creates there warrior or archer. */
boost::weak_ptr<warrior> warrior_born_near(people_type type, int tile_x, int tile_y, player owner)
{
	if(((tile_x < 0) || (tile_x > game_info::map_width - 1)) || ((tile_y < 0) || (tile_y > game_info::map_height - 1)))
		LOG("error wrong coordinates");

	int range = 1;
	bool done = false;
	boost::weak_ptr<warrior> return_value;
	while(!done)
	{
		std::vector<tile*> near_tiles = tiles_in_circle((float)range, session->tile_list[tile_y][tile_x].get());
		for(int i=0; i<near_tiles.size(); i++)
		{
			if(near_tiles[i]->is_free())
			{
				return_value = near_tiles[i]->warrior_born(type, owner);
				done = true;
				break;
			}
		}
		range++;
		if(range > 2*game_info::map_width)
			done = true;
	}
	return return_value;
}

/*Returns price of buidling in stone or wood, which depends on parametr material_type.*/
int show_building_price(building_type type, resources resource_type, upgrade_level upgrade)
{
	building_info info = building_info::show_building_info(type);
	
	std::vector<int> price;

	switch(upgrade)
	{
		case(NO_UPGRADE):			//price to build building
			price = info.building_price;
			break;

		case(FIRST_UPGRADE):
			price = info.first_upgrade_price;
			break;
		
		case(SECOND_UPGRADE):
			price = info.second_upgrade_price;
			break;

		case(THIRD_UPGRADE):
			price = info.third_upgrade_price;
			break;

		default:
		{
			LOG("");
			throw std::exception();
		}
	}

	return price[resource_type];
}

std::vector<tile*> tiles_under_building(boost::shared_ptr<building> b)
{
	return tiles_under_building(b->show_tile_x(), b->show_tile_y(), b->show_size());
}

/*Returns std::vector with pointers to tiles*/
std::vector<tile*> tiles_under_building(int tile_x, int tile_y, building_size type_of_building)
{					
	int size_x, size_y;
	if(type_of_building == ONE_TILE_BUILDING)
	{
		size_x = 1;
		size_y = 1;
	}
	if(type_of_building == FOUR_TILE_BUILDING)
	{
		size_x = 2;
		size_y = 2;
	}
	if(type_of_building == NINE_TILE_BUILDING)
	{
		size_x = 3;
		size_y = 3;
	}
	if(type_of_building == RIGHT_GATE_BUILDING)
	{
		size_x = 1;
		size_y = 3;
	}
	if(type_of_building == LEFT_GATE_BUILDING)
	{
		size_x = 3;
		size_y = 1;
	}

	std::vector<tile*> tiles;
	
	for(int x=0; x<size_x; x++)
	{
		for(int y=0; y<size_y; y++)
		{
			tiles.push_back(session->tile_list[tile_y - y][tile_x - x].get());
		}
	}

	return tiles;
}

/*Returns buildings connected by path to start tile, path length is less or equal to distance. Int in return value is distance to building.*/
std::vector<std::pair<boost::shared_ptr<building>, int>> buildings_connected_by_path(std::vector<tile*> queue, int max_distance)
{
	std::vector<std::pair<boost::shared_ptr<building>, int>> buildings;
	std::vector<std::vector<bool>> explored(game_info::map_height, std::vector<bool>(game_info::map_width, false));
	for(size_t i=0; i<queue.size(); ++i)
		explored[queue[i]->show_tile_y()][queue[i]->show_tile_x()] = true;

	int explored_index = 0;
	int increase_distance_index = queue.size();

	for(size_t distance=0; distance<max_distance; ++distance)
	{
		for(size_t i=explored_index; i<increase_distance_index; ++i)
		{
			for(size_t j=0; j<queue[i]->accessible_neighbours.size(); ++j)
			{
				tile* next_tile = queue[i]->accessible_neighbours[j];

				if(explored[next_tile->show_tile_y()][next_tile->show_tile_x()])
					continue;

				explored[next_tile->show_tile_y()][next_tile->show_tile_x()] = true;

				if(!next_tile->building_on_tile.expired())
				{
					bool already_founded = false;
					for(size_t k=0; k<buildings.size(); ++k)
					{
						if(buildings[i].first == next_tile->building_on_tile.lock())
						{
							already_founded = true;
							break;
						}
					}
					if(!already_founded)
						buildings.push_back(std::pair<boost::shared_ptr<building>, int>(next_tile->building_on_tile.lock(), distance));
				}

				if(next_tile->real_path_on_tile())
					queue.push_back(next_tile);
			}
		}
		explored_index = increase_distance_index;
		increase_distance_index = queue.size();
	}
	
	return buildings;
}

