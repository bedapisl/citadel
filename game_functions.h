#ifndef game_functions_h_bedas_guard
#define game_functions_h_bedas_guard

#define TILE_WIDTH 64					//real width of one image in pixels
#define TILE_HEIGHT 32

/// Draws map with all buildings, people, ....
int draw_map(int screen_position_x, int screen_position_y, game_mouse* mouse);	

ALLEGRO_BITMAP* create_minimap();	

int update_minimap(ALLEGRO_BITMAP* minimap);	
/// Handles movement of screen. Should be called once per frame.
int screen_move(ALLEGRO_MOUSE_STATE* mouse_state, ALLEGRO_KEYBOARD_STATE* keyboard_state, int &screen_position_x, int &screen_position_y);

int compute_tile_x(int drawing_x, int drawing_y, int screen_position_x, int screen_position_y);

int compute_tile_y(int drawing_x, int drawing_y, int screen_position_x, int screen_position_y);

int compute_tile_x(int game_x, int game_y);

int compute_tile_y(int game_x, int game_y);

inline int compute_game_x(int tile_x, int tile_y) {return (tile_x - tile_y + game_info::map_height)*TILE_WIDTH/2;}			//middle of the tile

inline int compute_game_y(int tile_x, int tile_y) {return (tile_x + tile_y + 1)*TILE_HEIGHT/2;}
/// Main panel is the lower part of game screen with buttons and minimap and other things.
int draw_main_panel(ALLEGRO_BITMAP* minimap, int screen_position_x, int screen_position_y, game_mouse* mouse, game_session* session);
/// Deletes death objects (people, buildings, missiles).
int delete_death(game_mouse* mouse);

std::vector<tile*> tiles_in_circle(float tile_range, tile* start);

std::vector<tile*> tiles_in_rectangle(int x1, int y1, int x2, int y2);

void rotate_map(bool clockwise, ALLEGRO_BITMAP** minimap, int *screen_position_x, int* screen_position_y);
/// Compute direction from old_tile to new_tile.
direction compute_direction(tile* old_tile, tile* new_tile);

#endif
