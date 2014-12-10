#ifndef game_functions_h
#define game_functions_h

#define TILE_WIDTH 64					//real width of one image in pixels
#define TILE_HEIGHT 32

//#include <memory>

//int game_loop();

int draw_map(int screen_position_x, int screen_position_y, game_mouse* mouse);

ALLEGRO_BITMAP* create_minimap();

int update_minimap(ALLEGRO_BITMAP* minimap);

int screen_move(ALLEGRO_MOUSE_STATE* mouse_state, ALLEGRO_KEYBOARD_STATE* keyboard_state, int &screen_position_x, int &screen_position_y);

int compute_tile_x(int drawing_x, int drawing_y, int screen_position_x, int screen_position_y);

int compute_tile_y(int drawing_x, int drawing_y, int screen_position_x, int screen_position_y);

int compute_tile_x(int game_x, int game_y);

int compute_tile_y(int game_x, int game_y);

inline int compute_game_x(int tile_x, int tile_y) {return (tile_x - tile_y + game_info::map_height)*TILE_WIDTH/2;}			//middle of the tile

inline int compute_game_y(int tile_x, int tile_y) {return (tile_x + tile_y + 1)*TILE_HEIGHT/2;}

int draw_main_panel(ALLEGRO_BITMAP* minimap, int screen_position_x, int screen_position_y, game_mouse* mouse, game_session* session);

int delete_death(game_mouse* mouse);

std::vector<tile*> tiles_in_circle(float tile_range, tile* start);

std::vector<tile*> tiles_in_rectangle(int x1, int y1, int x2, int y2);

std::vector<std::vector<boost::shared_ptr<tile>>> generate_map(std::vector<int> natural_resources, int mountains);

void generate_surface(std::vector<std::vector<boost::shared_ptr<tile>>>& map, int mountains);

void generate_resource(std::vector<std::vector<boost::shared_ptr<tile>>>& map, object_on_tile ob, int amount);

void generate_fertile(std::vector<std::vector<boost::shared_ptr<tile>>>& map, int amount);

void generate_water(std::vector<std::vector<boost::shared_ptr<tile>>>& map, int amount);

bool check_map(std::vector<std::vector<boost::shared_ptr<tile>>>& map);

std::vector<std::vector<bool>>& find_accessible(std::vector<std::vector<bool>> & explored, tile* start);

void rotate_map(bool clockwise, ALLEGRO_BITMAP** minimap, int *screen_position_x, int* screen_position_y);

direction compute_direction(tile* old_tile, tile* new_tile);

int init_static_variables();

//bool ingame_menu(ALLEGRO_EVENT_QUEUE* queue, ALLEGRO_EVENT* ev);

#endif
