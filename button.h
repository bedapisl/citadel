#ifndef button_h_bedas_guard
#define button_h_bedas_guard

	//button pointers are in chosen_mouse and tile::action_on_tile - so when deleting buttons take care of them

/**
 * \brief Represents button on panel in game not in menus. Also does not represents buttons in buildings interfaces, which are handled by buildings, rotate buttons and button to invoke menu.
 */
class button : public boost::enable_shared_from_this<button>
{
public:
	button(ALLEGRO_BITMAP* image, button_type type, bool multiple_selection);
	virtual ~button() {}
	virtual game_object* draw(int tile_x, int tile_y, int surface_height, int number_of_tile) {return nullptr;}	///< Returns object which should be drawn on given tile (for example building for which is player choosing location). Path is drawn directly by the tile where it is.
	virtual void draw_button(int button_number);	///< Draws button on panel on position specified by button_number.
	virtual void draw_info(int button_number) {}	///< Draws small window with informations about button on position specified by button number.
	virtual bool panel_click() {return true;}	///< Handles click to button in panel. Returns if button should became "chosen_button" (active button) - whether it will be applied to map. 
	virtual void map_click() {}			///< Handles click to map. Called only for "chosen_button" (active button). 
	virtual void draw_action(int screen_position_x, int screen_position_y) { }	///< Registers this button on tile, where it's action will be drawn.
	virtual int scroll() {return 0;}						///< Handles scrolling with mouse wheel. Called only for active button.
	button_type show_type() {return type;}
	bool show_multiple_selection() {return multiple_selection;}
	virtual int update_tiles_with_action(bool mouse_button_down, int tile_x, int tile_y, int button_down_tile_x, int button_down_tile_y) 
	{general_update_tiles_with_action(mouse_button_down, tile_x, tile_y, button_down_tile_x, button_down_tile_y); return 0;}
							///< Updates tiles where the action of this button will be applied, for example after player moved mouse.

	static void set_basic_button_list(bool restart_unlocks);			///< Sets buttons which will be drawn on the panel.
	static boost::shared_ptr<std::vector<boost::shared_ptr<button>>> button_list; 	///< Currently usable buttons.
	static void draw_button_info(const std::string& name, const std::string& text, const std::vector<int>& prices, int honour_price, int needed_workres, int real_x);				///< Draws window with informations about button.
	static void draw_progress_bar(int start_x, int start_y, int percentage, int bar_lenght = 100, int bar_height = 30);
				///< Draws progress bar. Used mainly by buildings, probably.
	virtual void reset_tiles_with_action();		

protected:	
	int general_update_tiles_with_action(bool mouse_button_down, int tile_x, int tile_y, int button_down_tile_x, int button_down_tile_y);
	static boost::shared_ptr<std::vector<boost::shared_ptr<button>>> init_buttons();

	ALLEGRO_BITMAP* action_image;		//needed for drawing button icon on panel
	button_type type;
	bool multiple_selection;
	std::vector<tile*> tiles_with_action;
};

/**
 * \brief Button to build a building.
 */
class button_build : public button
{
public:
	button_build(building_type type_of_building);
	game_object* draw(int tile_x, int tile_y, int surface_height, int number_of_tile);
	bool panel_click();
	void map_click();
	void draw_action(int screen_position_x, int screen_position_y);
	int scroll();
	int show_number_of_floors() {return number_of_floors;}
	void draw_info(int button_number);
	void draw_button(int button_number);
	int update_tiles_with_action(bool mouse_button_down, int tile_x, int tile_y, int button_down_tile_x, int button_down_tile_y);
	void reset_tiles_with_action();

private:
	building_type type_of_building;
	int number_of_floors;
	std::vector<boost::shared_ptr<building>> buildings_to_draw;
};

/**
 * \brief Button for building or destroying path.
 */
class button_path : public button
{
public:
	button_path(bool add);
	game_object* draw(int tile_x, int tile_y, int surface_height, int number_of_tile);
	void map_click();
	void draw_action(int screen_position_x, int screen_position_y);
	void draw_info(int button_number);
	void draw_button(int button_number);
	void reset_tiles_with_action();

private:
	bool add;		//determines if paths are build or destroyed by this button
};

/**
 * \brief Button for changing currently visible buttons.
 */
class navigation_button : public button
{
public:
	navigation_button(navigation_button_type navigation_type, const std::string& button_name);
	void set_buttons_after_use(boost::shared_ptr<std::vector<boost::shared_ptr<button>>> new_buttons_after_use) {buttons_after_use = new_buttons_after_use;}
	bool panel_click() {button::button_list = buttons_after_use; return false;}
	void draw_info(int button_number);
	void draw_button(int button_number);	

private:
	boost::shared_ptr<std::vector<boost::shared_ptr<button>>> buttons_after_use;
	navigation_button_type navigation_type;
	const std::string name;
};


#endif
