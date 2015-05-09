
/**
 * \file game_functions.h
 * \brief Various ingame functions.
 * Contains functions for drawing map, creating minimap and few other map oriented functions.
 */

#ifndef game_functions_h_bedas_guard
#define game_functions_h_bedas_guard

#define TILE_WIDTH 64					//real width of one image in pixels
#define TILE_HEIGHT 32

/// Draws map with all buildings, people, ....
int draw_map(int screen_position_x, int screen_position_y, game_mouse* mouse);	

///Creates minimap. Uses session->tile_list.
ALLEGRO_BITMAP* create_minimap();	

/// Updates tiles from tile::minimap_updates in minimap.
int update_minimap(ALLEGRO_BITMAP* minimap);	

/// Handles movement of screen. Should be called once per frame.
int screen_move(ALLEGRO_MOUSE_STATE* mouse_state, ALLEGRO_KEYBOARD_STATE* keyboard_state, int &screen_position_x, int &screen_position_y);

/// Computes x index of tile in session->tile_list, but does not take in account height of tiles.
/// \param drawing_x Distance from left side of display in pixels.
/// \param drawing_y Distance from upper side of display in pixels.
/// \param screen_position_x Display position relative to left corner of map in pixels. If display moves left, value grows.
/// \param scrren_position_y Display position relative to right corner of map i pixels. If display moves down, value grows.
int compute_tile_x(int drawing_x, int drawing_y, int screen_position_x, int screen_position_y);

/// Computes x index of tile in session->tile_list, but does not take in account height of tiles.
/// \param drawing_x Distance from left side of display in pixels.
/// \param drawing_y Distance from upper side of display in pixels.
/// \param screen_position_x Display position relative to left corner of map in pixels. If display moves left, value grows.
/// \param scrren_position_y Display position relative to right corner of map i pixels. If display moves down, value grows.
int compute_tile_y(int drawing_x, int drawing_y, int screen_position_x, int screen_position_y);

/// Computes x index of tile in session->tile_list, but does not take in account height of tiles.
/// \param game_x Distance from left corner of map in pixels.
/// \param game_y Distance from upper corner of map in pixels.
int compute_tile_x(int game_x, int game_y);

/// Computes y index of tile in session->tile_list, but does not take in account height of tiles.
/// \param game_x Distance from left corner of map in pixels.
/// \param game_y Distance from upper corner of map in pixels.
int compute_tile_y(int game_x, int game_y);

/// Returns tile's distance from left corner of map.
inline int compute_game_x(int tile_x, int tile_y) {return (tile_x - tile_y + game_info::map_height)*TILE_WIDTH/2;}			//middle of the tile

/// Returns tile's distance from upper corner of map.
inline int compute_game_y(int tile_x, int tile_y) {return (tile_x + tile_y + 1)*TILE_HEIGHT/2;}

/// Main panel is the lower part of game screen with buttons and minimap and other things.
int draw_main_panel(ALLEGRO_BITMAP* minimap, int screen_position_x, int screen_position_y, game_mouse* mouse, game_session* session);

/// Deletes death objects (people, buildings, missiles).
int delete_death(game_mouse* mouse);

/// Returns tiles in circle with diameter tile_range and middle point start. Tiles are from session->tile_list.
std::vector<tile*> tiles_in_circle(float tile_range, tile* start);

/// Returns tiles in rectangle given by two corners. Rectangle's sides are horizontal and vertical, not diagonal. Tiles are from session->tile_list.
std::vector<tile*> tiles_in_rectangle(int x1, int y1, int x2, int y2);

/// Rotates session->tile_list and all objects on map by 90 degrees. Might take some time.
void rotate_map(bool clockwise, ALLEGRO_BITMAP** minimap, int *screen_position_x, int* screen_position_y);

/// Compute direction from old_tile to new_tile.
direction compute_direction(tile* old_tile, tile* new_tile);

/// Returns kind of borders (Ramp borders, normal borders) from cTile in given direction.
possible_borders tile_borders(tile* cTile, direction way);

#endif
