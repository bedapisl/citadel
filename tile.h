
/**
 * \file tile.h
 * \brief Contains class \ref tile which describes one tile in game map.
 */

#ifndef tile_h
#define tile_h

#include "core.h"

class tree;
class button;
class people;
class building;
class warrior;

extern std::ofstream log_file;

/**
 * \brief Represents ont tile in map.
 */
class tile : public game_object
{
public:
	tile(tile_type type, int tile_x, int tile_y, int surface_height);
	std::vector<game_object*> draw(int screen_position_x, int screen_position_y);	///< Draws tile. Returns objects which should be drawed above/on this tile.
	int set_right_drawing(std::vector<std::vector<boost::shared_ptr<tile>>>& map);	///< Chooses the right image for this tile depending on adjacent tiles and this tile type. Should be called if map changed.
	void set_accessible_neighbours(std::vector<std::vector<boost::shared_ptr<tile>>> & map);	///< Used to speed up pathfinding. Should be called if map changed.
	void build(building_type type, player owner);	///< Builds new building on this tile.
	boost::weak_ptr<warrior> warrior_born(people_type type, player owner);	///< Creates new warrior on this tile.
	int people_die();							///< Registers this tile to be updated in minimap.
	int people_come(boost::shared_ptr<people> people_which_comes);		///< Registers that some people are staying on this tile.
	int people_leave(boost::shared_ptr<people> people_which_leaves);	///< Registers that some people left this tile.
	int show_effective_height();						///< Shows the height of this tile with taking buildings in the account.
	borders show_border() {return border;}
	tile_type show_type() {return type;}
	bool show_visible() {return visible;}					///< Indicates if this tile is visible by player.
	bool show_draw_building() {return draw_building;}			///< Returns whether this tile will draw building (buildings which stays on more tiles are drawed only by one)
	void set_type(tile_type type);
	bool is_free() {return ((building_on_tile.expired()) && (!are_people_on_tile()) && (object == NOTHING));}	///< Returns whether tile is empty.
	bool is_ramp() {return type == RAMP;}
	bool are_people_on_tile();
	void check_death_people_on_tile();					///< Removes dead people from tile inner structures.
	void set_draw_building(bool draw) {draw_building = draw;}	
	int add_object(object_on_tile);
	int add_path(bool real);
	int remove_path(bool real);
	bool unreal_path_on_tile() {return path_on_tile;}			///< Returns whether there is any path (real or unreal) on tile.
	bool real_path_on_tile() {return path_on_tile && bIs_path_real;}
	borders show_borders() {return border;}
	bool is_water_tile() {return object == WATER_TILE;}
	bool is_fertile() {return fertile;}
	void add_fertile();
	boost::weak_ptr<building> building_on_tile;
	std::vector<boost::weak_ptr<people>> people_on_tile;
	boost::weak_ptr<button> action_on_tile;
	int number_of_tile_with_action;			// used by buttons with multiple selection to draw objects in right order
	object_on_tile object;				///< These objects are rocks or trees something derived from class game_object.
	bool can_go_inside_building;			///< Whether it is possible to go inside building on this tile (e.g. people going through gate)
	bool can_go_on_building;			///< Whether it is possible to go on building on this tile (e.g. people staying on palisade)
	bool stairs_on_tile;
	picture tile_object_image;
	std::vector<tile*> accessible_neighbours;	///< Pointers to neighbour tiles, where can people move if there are no buildings, people, ... if they cant move, pointer is NULL.
	std::vector<tile*> neighbours_with_path;	///< Only neighbours with real path.
	int set_path_drawing();				///< Chooses right path image to be drawn on this tile.
	static void set_neighbours_with_path();		///< Computes neighbours with path for all tiles in session->tile_list.
	static std::vector<tile*> minimap_updates;	///< Tiles which should be updated on minimap.

	void prepare_serialization();
	void finish_serialization();
	void check_consistency();
	
	std::vector<std::pair<int, int>> serializable_accessible_neighbours;	///< Normal accessible_neighbours would cause stack overflow if serialized. This workaround remembers accessible neighbours with their coordinates.
	std::vector<std::pair<int, int>> serializable_neighbours_with_path;
	
	friend class boost::serialization::access;

	template <class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		LOG("serializing tile");
		
		using boost::serialization::make_nvp;

		ar & make_nvp("game_object", boost::serialization::base_object<game_object>(*this));
		ar & make_nvp("building_on_tile", building_on_tile);
		ar & make_nvp("people_on_tile", people_on_tile);
		ar & make_nvp("number_of_tile_with_action", number_of_tile_with_action);
		ar & make_nvp("object", object);
		ar & make_nvp("can_go_inside_building", can_go_inside_building);
		ar & make_nvp("can_go_on_building", can_go_on_building);
		ar & make_nvp("stairs_on_tile", stairs_on_tile);
		ar & make_nvp("tile_object_image", tile_object_image);
		//ar & make_nvp("accessible_neighbours", accessible_neighbours);		//this causes stack overflow
		//ar & neighbours_with_path							//causes stack overflow
		ar & make_nvp("minimap_updates", minimap_updates);
		ar & make_nvp("serializable_accessible_neighbours", serializable_accessible_neighbours);
		ar & make_nvp("serializable_neighbours_with_path", serializable_neighbours_with_path);
		ar & make_nvp("path_on_tile", path_on_tile);
		ar & make_nvp("bIs_path_real", bIs_path_real);
		ar & make_nvp("visible", visible);
		ar & make_nvp("border", border);
		ar & make_nvp("path_border", path_border);
		ar & make_nvp("type", type);
		ar & make_nvp("draw_building", draw_building);
		ar & make_nvp("fertile", fertile);
	}

private:
	tile() {}		//for boost serialization

	bool path_on_tile;
	bool bIs_path_real;
	bool visible;
	borders border;
	borders path_border;
	tile_type type;
	bool draw_building;
	bool fertile;
};

#endif
