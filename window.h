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

class slider
{
public:
	slider(int x, int y, std::string label, int initial_value, int max_value);
	void mouse_down(int mouse_x, int mouse_y);
	void update_mouse_position(int mouse_x, int mouse_y);
	void mouse_up(int mouse_x, int mouse_y);
	int get_value() {return value;}
	void draw();
private:
	void change_value(int mouse_x);

	const int name_length = 150;
	const int slider_length = 100;
	const int height = 30;

	int x;
	int y;
	std::string label;
	int value;
	int max_value;
	bool has_focus;
};

class text_field
{
public:
	text_field(int x, int y, std::string label, std::string initial_value, int max_size, bool numbers_only);
	void mouse_down(int mouse_x, int mouse_y);
	void key_char(ALLEGRO_EVENT* ev);
	std::string get_value() {return value;}
	void draw();

private:
	const int name_length = 100;		//in pixels
	const int field_length = 100;		//in pixels
	const int height = 30;
	int x, y;
	std::string label;
	std::string value;
	int max_size;
	bool numbers_only;
	bool has_focus;
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













