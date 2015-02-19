#include "core.h"

//extern int display_width;
//extern int display_height;
extern ALLEGRO_BITMAP** image_list;
extern ALLEGRO_FONT* font25;
extern ALLEGRO_FONT* font15;
extern game_session* sesion;

std::vector<boost::shared_ptr<window>> window::active_windows;

window::window() : start_x(0), start_y(0), end_x(display_width), end_y(display_height), closed(false)
{ }

window::window(int start_x, int start_y, int end_x, int end_y) : 
		start_x(start_x), start_y(start_y), end_x(end_x), end_y(end_y), closed(false)
{ }

void window::draw()
{
	if(closed)
		return;

	al_draw_bitmap_region(image_list[TEXTURE_GREY_IMAGE], 0, 0, end_x - start_x, end_y - start_y, start_x, start_y, 0);

	al_draw_filled_rectangle(start_x + 3, start_y + 3, end_x - 26, start_y + 23, DARK_GREY_COLOR);
	al_draw_filled_rectangle(end_x - 23, start_y + 3, end_x - 3, start_y + 23, DARK_GREY_COLOR);

	al_draw_bitmap(image_list[CLOSE_WINDOW_IMAGE], end_x - 23, start_y + 3, 0);

	specific_draw();
}

bool window::function_click(int mouse_x, int mouse_y)
{
	if(closed)
		return false;

	if((mouse_x < start_x) || (mouse_x > end_x) || (mouse_y < start_y) || (mouse_y > end_y))
		return false;

	if((mouse_x > end_x - 26) && (mouse_y < start_y + 23))
		close();

	specific_function_click(mouse_x, mouse_y);

	return true;
}

void window::remove_closed()
{
	auto it = remove_if(active_windows.begin(), active_windows.end(), [](boost::shared_ptr<window> w) {return w->closed;});
	active_windows.erase(it, active_windows.end());
}

void window::draw_active_windows()
{
	remove_closed();
	for_each(active_windows.begin(), active_windows.end(), [](boost::shared_ptr<window> w) {w->draw();});
}

bool window::active_windows_function_click(int mouse_x, int mouse_y)
{
	remove_closed();
	for(int i=active_windows.size() - 1; i >= 0; --i)
	{
		if(active_windows[i]->function_click(mouse_x, mouse_y))
		{	
			return true;
		}
	}
	return false;
}

store_window::store_window(boost::shared_ptr<store> store_ptr) : window((display_width - store_window_width)/2, store_window_start_y, 
					(display_width + store_window_width)/2, store_window_start_y + store_window_heigth), the_store(store_ptr) {}

void store_window::specific_draw()
{
	if(the_store.expired())
	{
		close();
		return;
	}
	
	the_store.lock()->draw_window(start_x, start_y);
}

void store_window::specific_function_click(int mouse_x, int mouse_y)
{
	if(the_store.expired())
	{
		close();
		return;
	}
	the_store.lock()->window_function_click(mouse_x - start_x, mouse_y - start_y);
}

	
slider::slider(int x, int y, std::string label, int initial_value, int max_value) 
	: x(x), y(y), label(label), value(initial_value), max_value(max_value), has_focus(false)
{ };

void slider::mouse_down(int mouse_x, int mouse_y)
{
	if(has_focus)
		has_focus = false;

	else if((mouse_x > x) && (mouse_x < x + name_length + slider_length) && (mouse_y > y) && (mouse_y < y + height))
	{
		has_focus = true;
		change_value(mouse_x);
	}
}

void slider::update_mouse_position(int mouse_x, int mouse_y)
{
	if(has_focus)
		change_value(mouse_x);
}

void slider::mouse_up(int mouse_x, int mouse_y)
{
	if(has_focus)
		change_value(mouse_x);

	has_focus = false;
}

void slider::draw()
{
	al_draw_textf(font25, WRITING_COLOR, x, y, ALLEGRO_ALIGN_LEFT, "%s", label.c_str());
	al_draw_filled_rounded_rectangle(x + name_length, y + 4, x + name_length + slider_length, y + height - 4, 4, 4, GREY_COLOR);
	al_draw_filled_circle(x + name_length + slider_length * ((double)value/(double)max_value), y + height/2, 10, WRITING_COLOR);
}

void slider::change_value(int mouse_x)
{
	mouse_x = std::max(x + name_length, std::min(x + name_length + slider_length, mouse_x)) + (slider_length/max_value)/2;	//last summend is for better rounding
	value = (((double)(mouse_x - x - name_length))/(double)(slider_length))*max_value;
}

text_field::text_field(int x, int y, std::string label, std::string initial_value, int max_size, bool numbers_only) : x(x), y(y), label(label), value(initial_value), max_size(max_size), numbers_only(numbers_only), has_focus(false)
{ }

void text_field::mouse_down(int mouse_x, int mouse_y)
{
	if(has_focus)
		has_focus = false;
	
	else if(!has_focus)
	{
		if((mouse_x > x) && (mouse_x < x + name_length + field_length) && (mouse_y > y) && (mouse_y < y + height))
			has_focus = true;
	}
}

void text_field::key_char(ALLEGRO_EVENT* ev)
{
	if(!has_focus)
		return;
	
	char c = ev->keyboard.unichar;
	if(((c >= '0') && (c <= '9')) || (!numbers_only && (((c >= 'a') && (c <= 'z')) || ((c >= 'A') && (c <= 'Z')))))
	{
		if(value.size() < max_size)
			value.push_back(c);
	}

	if((ev->keyboard.keycode == ALLEGRO_KEY_BACKSPACE) || (ev->keyboard.keycode == ALLEGRO_KEY_DELETE))
	{
		if(value.size() > 0)
			value.pop_back();
	}

	if(ev->keyboard.keycode == ALLEGRO_KEY_ENTER)
		has_focus = false;
}

void text_field::draw()
{
	al_draw_textf(font25, WRITING_COLOR, x, y, ALLEGRO_ALIGN_LEFT, "%s", label.c_str());
	ALLEGRO_COLOR background_color = BLACK_COLOR;
	ALLEGRO_COLOR text_color = WRITING_COLOR;
	if(has_focus)
	{
		background_color = DARK_GREY_COLOR;
		text_color = WHITE_COLOR;
	}
	
	al_draw_filled_rectangle(x + name_length, y, x + name_length + field_length, y + height, background_color);
	al_draw_rectangle(x + name_length, y, x + name_length + field_length, y + height, GREY_COLOR, 4);
	al_draw_textf(font25, text_color, x + name_length + 10, y, ALLEGRO_ALIGN_LEFT, "%s", value.c_str());
}

music::music() : sample_playing(nullptr), mixer(al_create_mixer(44100, ALLEGRO_AUDIO_DEPTH_INT16, ALLEGRO_CHANNEL_CONF_2)), 
			voice(al_create_voice(44100, ALLEGRO_AUDIO_DEPTH_INT16, ALLEGRO_CHANNEL_CONF_2)), number_of_background_music(0)
{ 
	al_attach_mixer_to_voice(mixer, voice);

	background_music.push_back(al_load_sample("data/sound/firesong.wav"));
	background_music.push_back(al_load_sample("data/sound/night_of_the_owl.wav"));

	al_reserve_samples(1);		//how much samples can play in one moment
	for(int i=0; i<background_music.size(); ++i)
	{
		if(background_music[i] == nullptr)
			LOG("Failed to load music sample " << i);
	}
	auto it = std::remove_if(background_music.begin(), background_music.end(), [](ALLEGRO_SAMPLE* s) {return s == nullptr;});
	background_music.erase(it, background_music.end());
}

music::~music()
{	
	for(int i=0; i<background_music.size(); ++i)
	{
		al_destroy_sample(background_music[i]);
		background_music[i] = nullptr;
	}
	al_destroy_sample_instance(sample_playing);
	al_destroy_mixer(mixer);
}

music& music::get_instance()
{
	static music instance;
	return instance;
}

void music::play_background_music()
{
	if(number_of_background_music < background_music.size())
	{
		al_destroy_sample_instance(sample_playing);
		sample_playing = al_create_sample_instance(background_music[number_of_background_music]);
		al_set_sample_instance_playmode(sample_playing, ALLEGRO_PLAYMODE_ONCE);
		al_attach_sample_instance_to_mixer(sample_playing, mixer);
		al_play_sample_instance(sample_playing);
	}
}

void music::stop_background_music()
{
	al_stop_sample_instance(sample_playing);
}

void music::update()
{
	if(game_info::music)
	{
		if((!al_get_sample_instance_playing(sample_playing)) && (!background_music.empty()))		//sample has finished
		{
			number_of_background_music++;
			if(number_of_background_music == background_music.size())
				number_of_background_music = 0;
			
			play_background_music();
		}
	}
}


