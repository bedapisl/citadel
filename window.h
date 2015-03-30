#ifndef citadel_window_h
#define citadel_window_h

#include "core.h"

extern int display_width;
extern int display_height;

class window
{
public:
	window();
	window(int start_x, int start_y, int end_x, int end_y);
	virtual ~window() {}
	void draw();
	bool function_click(int mouse_x, int mouse_y); //return false if click is not in window
	bool is_closed() {return closed;}
	void close() {closed = true;}

	static void remove_closed();
	static void draw_active_windows();
	static bool active_windows_function_click(int mouse_x, int mouse_y);	//return false if click in not in any window
	static std::vector<boost::shared_ptr<window>> active_windows;

protected:
	virtual void specific_draw() {}
	virtual void specific_function_click(int mouse_x, int mouse_y) {}

	int start_x;
	int start_y;
	int end_x;
	int end_y;
	bool closed;
};

class store;

class store_window : public window
{
public:
	store_window(boost::shared_ptr<store> store_ptr);	
	void specific_draw();
	void specific_function_click(int mouse_x, int mouse_y);
	~store_window() {}

private:
	boost::weak_ptr<store> the_store;
	static const int store_window_start_y = 50;
	static const int store_window_width = 700;
	static const int store_window_heigth = 600;
};

class gui_element;

class gui_block
{
public:
	gui_block(int vertical_border = 10, int horizontal_border = 10) : length(0), height(0), vertical_border(vertical_border), horizontal_border(horizontal_border), mouse_on_block(false), x(0), y(0), hardwired_length(false), hardwired_height(false) { }
	void mouse_down(ALLEGRO_EVENT* ev);
	void mouse_axes(ALLEGRO_EVENT* ev);
	void mouse_up(ALLEGRO_EVENT* ev);
	void key_char(ALLEGRO_EVENT* ev);
	void enter_down(ALLEGRO_EVENT* ev);
	void up_arrow_down(ALLEGRO_EVENT* ev);
	void down_arrow_down(ALLEGRO_EVENT* ev);
	void draw();			//draw pro vsechny elementy + nakresli ramecek
	void update_position(int new_x, int new_y);
	void add_gui_element(boost::shared_ptr<gui_element> e);
	void remove_invalid_elements();
	void set_length(int new_length) {length = new_length; hardwired_length = true;}
	void set_height(int new_height) {height = new_height; hardwired_height = true;}
	
	int length;
	int height;
	static const int vertical_space_between_elements = 20;
	int vertical_border = 10;
	int horizontal_border = 10;
	bool mouse_on_block;
	int x;
	int y;
			
private:
	//void draw_line(ALLEGRO_BITMAP* image, int image_region_y, int line_y, int line_height);	//horizontal line
	bool is_mouse_on_block(int mouse_x, int mouse_y);
	bool hardwired_length;
	bool hardwired_height;
std::vector<boost::weak_ptr<gui_element>> elements;
};

class gui_element
{
public:
	gui_element(int length, int height) : length(length), height(height), x(0), y(0), has_focus(false), highlighted(false) { }
	virtual ~gui_element() { };
	virtual void mouse_down(int mouse_x, int mouse_y) { }
	virtual void mouse_axes(int mouse_x, int mouse_y);
	virtual void mouse_up(int mouse_x, int mouse_y) { }
	virtual void enter_down() { }
	virtual void key_char(ALLEGRO_EVENT* ev) { }
	virtual void click_out_of_block() {has_focus = false; highlighted = false;}

	virtual void draw() = 0;
	virtual void update_position(int new_x, int new_y) {x = new_x; y = new_y;}
	int length;
	int height;
	int x;
	int y;
	bool has_focus;
	bool highlighted;
};

class slider : public gui_element
{
public:
	slider(std::string label, int initial_value, int max_value);
	void mouse_down(int mouse_x, int mouse_y);
	void mouse_axes(int mouse_x, int mouse_y);
	void mouse_up(int mouse_x, int mouse_y);
	void enter_down();
	int get_value() {return value;}
	void draw();

private:
	void change_value(int mouse_x);

	static const int name_length = 200;
	static const int slider_length = 100;
	static const int slider_height = 30;

	std::string label;
	int value;
	int max_value;
};

class text_field : public gui_element
{
public:
	text_field(std::string label, std::string initial_value, bool numbers_only, int max_length);
	void mouse_down(int mouse_x, int mouse_y);
	void key_char(ALLEGRO_EVENT* ev);
	std::string get_value() {return value;}
	void draw();
	void set_value(const std::string& new_text) {value = new_text;}

private:
	static const int name_length = 200;		//in pixels
	int field_length;
	static const int letter_size = 15;
	static const int text_field_height = 30;
	std::string label;
	std::string value;
	int max_value_length;				//max length in letters
	bool numbers_only;
};

class menu_button : public gui_element
{
public:
	menu_button();
	menu_button(const std::string& name, bool centre_aligned, int font_size_par = 25, int length_par = 200, int height_par = 30);
	
	void mouse_down(int mouse_x, int mouse_y);
	void mouse_axes(int mouse_x, int mouse_y);
	void enter_down();
	void draw();
	bool clicked();
	std::string show_name() {return name;}
	
	int font_size;
private:
	bool centre_aligned;
	bool was_clicked;
	const std::string name;
};

class switch_button : public gui_element		//button in menu with values: disabled, enabled
{
public:
	switch_button(const std::string& name, bool initial_value) : gui_element(name_length + value_length, button_height), name(name), value(initial_value) {}
	void mouse_down(int mouse_x, int mouse_y);
	bool get_value() {return value;}
	void mouse_axes(int mouse_x, int mouse_y);
	void enter_down();
	void draw();

private:
	static const int name_length = 200;
	static const int value_length = 100;
	static const int button_height = 30;
	
	std::string name;
	bool value;
};

class music
{
public:
	~music();
	void play_background_music();
	void stop_background_music();
	void update();
	static music& get_instance();

private:
	ALLEGRO_SAMPLE_INSTANCE* sample_playing;
	ALLEGRO_MIXER* mixer;
	ALLEGRO_VOICE* voice;
	std::vector<ALLEGRO_SAMPLE*> background_music;
	int number_of_background_music;
	music();
};


#endif













