#ifndef button_h_bedas_guard
#define button_h_bedas_guard

	//button pointers are in chosen_mouse and tile::action_on_tile - so when deleting buttons take care of them

class button{
public:
	int init(ALLEGRO_BITMAP* image, button_type type, bool multiple_selection);
	virtual ~button() {}
	virtual int init(building_type type_of_building, int number_of_floors)
		{return 0;}
	virtual int init(bool add) {return 0;}
	virtual game_object* draw(int tile_x, int tile_y, int surface_height, int number_of_tile) {return 0;}
	virtual void draw_button(int button_number);
	virtual void draw_info(int button_number) {}
	virtual bool panel_click() {return true;}		//returns true, if button should become "chosen_button" in mouse 
	virtual void map_click() {}
	virtual void draw_action(int screen_position_x, int screen_position_y, boost::shared_ptr<button> this_button) {}
	virtual int scroll() {return 0;}
	button_type show_type() {return type;}
	bool show_multiple_selection() {return multiple_selection;}
	virtual int update_tiles_with_action(bool mouse_button_down, int tile_x, int tile_y, int button_down_tile_x, int button_down_tile_y) 
	{general_update_tiles_with_action(mouse_button_down, tile_x, tile_y, button_down_tile_x, button_down_tile_y); return 0;}
	virtual void reset_tiles_with_action();

	static void set_basic_button_list(bool restart_unlocks);
	static boost::shared_ptr<std::vector<boost::shared_ptr<button>>> button_list; //points to one of these: base_buttons, food_buttons, economy_buttons, industry_buttons, military_buttons
	static void draw_button_info(const std::string& name, const std::string& text, const std::vector<int>& prices, int honour_price, int needed_workres, int real_x);
	static void draw_progress_bar(int start_x, int start_y, int percentage, int bar_lenght = 100, int bar_height = 30);

protected:	
	int general_update_tiles_with_action(bool mouse_button_down, int tile_x, int tile_y, int button_down_tile_x, int button_down_tile_y);
	static boost::shared_ptr<std::vector<boost::shared_ptr<button>>> init_buttons();

	ALLEGRO_BITMAP* action_image;		//needed for drawing button icon on panel
	button_type type;
	bool multiple_selection;
	std::vector<tile*> tiles_with_action;
};

class button_build : public button
{
public:
	button_build(building_type type_of_building);
	game_object* draw(int tile_x, int tile_y, int surface_height, int number_of_tile);
	bool panel_click();
	void map_click();
	void draw_action(int screen_position_x, int screen_position_y, boost::shared_ptr<button> this_button);
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

class button_path : public button
{
public:
	button_path(bool add);
	game_object* draw(int tile_x, int tile_y, int surface_height, int number_of_tile);
	void map_click();
	void draw_action(int screen_position_x, int screen_position_y, boost::shared_ptr<button> this_button);
	void draw_info(int button_number);
	void draw_button(int button_number);
	void reset_tiles_with_action();

private:
	bool add;		//determines if paths are build or destroyed by this button
};

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
