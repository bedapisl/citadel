#include "core.h"

extern ALLEGRO_BITMAP** image_list;
extern ALLEGRO_FONT* font35;
extern ALLEGRO_FONT* font30;
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

void gui_block::mouse_down(ALLEGRO_EVENT* ev)
{
	if(is_mouse_on_block(ev->mouse.x, ev->mouse.y))
	{
		for(int i=0; i<elements.size(); ++i)
			elements[i].lock()->mouse_down(ev->mouse.x, ev->mouse.y);
	}
	else
	{
		for(int i=0; i<elements.size(); ++i)
			elements[i].lock()->click_out_of_block();
	}
}

void gui_block::mouse_axes(ALLEGRO_EVENT* ev)
{
	is_mouse_on_block(ev->mouse.x, ev->mouse.y);

	for(int i=0; i<elements.size(); ++i)
		elements[i].lock()->mouse_axes(ev->mouse.x, ev->mouse.y);
}

void gui_block::mouse_up(ALLEGRO_EVENT* ev)
{	
	if(is_mouse_on_block(ev->mouse.x, ev->mouse.y))
	{
		for(int i=0; i<elements.size(); ++i)
			elements[i].lock()->mouse_up(ev->mouse.x, ev->mouse.y);
	}
	else
	{
		for(int i=0; i<elements.size(); ++i)
			elements[i].lock()->click_out_of_block();
	}
}

void gui_block::key_char(ALLEGRO_EVENT* ev)
{
	for(int i=0; i<elements.size(); ++i)
		elements[i].lock()->key_char(ev);
}

void gui_block::enter_down(ALLEGRO_EVENT* ev)
{
	for(int i=0; i<elements.size(); ++i)
	{
		if(elements[i].lock()->has_focus || elements[i].lock()->highlighted)
			elements[i].lock()->enter_down();
	}
}

void gui_block::up_arrow_down(ALLEGRO_EVENT* ev)
{
	bool founded_active = false;
	for(int i=0; i<elements.size(); ++i)
	{
		if(elements[i].lock()->has_focus || elements[i].lock()->highlighted)
		{
			elements[i].lock()->has_focus = false;
			elements[i].lock()->highlighted = false;
			i--;
			if(i < 0)
				i = elements.size() - 1;;

			elements[i].lock()->has_focus = true;
			elements[i].lock()->highlighted = true;
			founded_active = true;
			break;
		}
	}
	if(!founded_active)
	{
		elements[0].lock()->has_focus = true;
		elements[0].lock()->highlighted = true;
	}
}

void gui_block::down_arrow_down(ALLEGRO_EVENT* ev)
{
	bool founded_active = false;
	for(int i=0; i<elements.size(); ++i)
	{
		if(elements[i].lock()->has_focus || elements[i].lock()->highlighted)
		{
			elements[i].lock()->has_focus = false;
			elements[i].lock()->highlighted = false;
			i++;
			if(i == elements.size())
				i = 0;

			elements[i].lock()->has_focus = true;
			elements[i].lock()->highlighted = true;
			founded_active = true;
			break;
		}
	}
	if(!founded_active)
	{
		elements[0].lock()->has_focus = true;
		elements[0].lock()->highlighted = true;
	}
}

void gui_block::draw()
{
	int x_drawn = 0;
	int y_drawn = 0;
	ALLEGRO_BITMAP* image = image_list[TEXTURE_GREY_IMAGE];
	int image_length = 1024;
	int image_height = 1024;

	while(y_drawn < height)
	{
		int height_to_draw = std::min(image_height, height - y_drawn);

		x_drawn = 0;
		while(x_drawn < length)
		{
			int length_to_draw = std::min(image_length, length - x_drawn);

			al_draw_bitmap_region(image, 0, 0, length_to_draw, height_to_draw, x + x_drawn, y + y_drawn, 0);

			x_drawn += length_to_draw;
		}

		y_drawn += height_to_draw;
	}

	for(int i=0; i<elements.size(); ++i)
	{
		if(elements[i].lock()->y + elements[i].lock()->height < y + height)
		{
			elements[i].lock()->draw();
		}
	}
}

void gui_block::update_position(int new_x, int new_y)
{
	x = new_x;
	y = new_y;

	int element_y = y;
	for(int i=0; i<elements.size(); ++i)
	{
		elements[i].lock()->update_position(x + vertical_border, element_y + horizontal_border);
		element_y += elements[i].lock()->height + vertical_space_between_elements;
	}
}

void gui_block::add_gui_element(boost::shared_ptr<gui_element> e)
{
	if(!hardwired_height)
	{
		if(elements.size() > 0)
			height += vertical_space_between_elements;
		else
			height += 2 * horizontal_border;
		
		height += e->height;
	}
	
	if(!hardwired_length)
	{
		if(length < e->length + 2 * vertical_border)
			length = e->length + 2 * vertical_border;
	}
	
	elements.push_back(e);
}

void gui_block::remove_invalid_elements()
{
	for(int i=0; i<elements.size(); ++i)
	{
		if(elements[i].expired())
			elements.erase(elements.begin() + i);
	}
}

bool gui_block::is_mouse_on_block(int mouse_x, int mouse_y)
{
	if(((mouse_x >= x) && (mouse_x <= x + length)) && ((mouse_y >= y) && (mouse_y <= y + height)))
		mouse_on_block = true;
	else
		mouse_on_block = false;

	return mouse_on_block;
}

void gui_element::mouse_axes(int mouse_x, int mouse_y)
{
	if((mouse_x > x) && (mouse_x < x + length) && (mouse_y > y) && (mouse_y < y + height))
		highlighted = true;
	else
		highlighted = false;
}

slider::slider(std::string label, int initial_value, int max_value) 
	: gui_element(name_length + slider_length, slider_height), label(label), value(initial_value), max_value(max_value)
{ }

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

void slider::mouse_axes(int mouse_x, int mouse_y)
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

void slider::enter_down()
{
	if(highlighted)
		has_focus = !has_focus;
	
	else
		has_focus = false;
}

void slider::draw()
{
	al_draw_textf(font25, WRITING_COLOR, x, y, ALLEGRO_ALIGN_LEFT, "%s", label.c_str());
	ALLEGRO_COLOR bar_color, modifier_color;
	if(has_focus)
	{
		bar_color = LIGHT_GREY_COLOR;
		modifier_color = WHITE_COLOR;
	}
	else
	{
		bar_color = GREY_COLOR;
		modifier_color = WRITING_COLOR;
	}
	al_draw_filled_rounded_rectangle(x + name_length, y + 4, x + name_length + slider_length, y + height - 4, 4, 4, bar_color);
	al_draw_filled_circle(x + name_length + slider_length * ((double)value/(double)max_value), y + height/2, 10, modifier_color);
}

void slider::change_value(int mouse_x)
{
	mouse_x = std::max(x + name_length, std::min(x + name_length + slider_length, mouse_x)) + (slider_length/max_value)/2;	//last summend is for better rounding
	value = (((double)(mouse_x - x - name_length))/(double)(slider_length))*max_value;
}

text_field::text_field(std::string label, std::string initial_value, bool numbers_only, int max_length) : gui_element(name_length + letter_size * max_length + 20, text_field_height), field_length(max_length * letter_size + 20), label(label), value(initial_value), max_value_length(max_length), numbers_only(numbers_only)
{
}

void text_field::mouse_down(int mouse_x, int mouse_y)
{
	if(has_focus)
		has_focus = false;
	
	else if(!has_focus)
	{
		if((mouse_x > x) && (mouse_x < x + name_length + field_length) && (mouse_y > y) && (mouse_y < y + height))
		{
			has_focus = true;
			//value = "";
		}
	}
}

void text_field::key_char(ALLEGRO_EVENT* ev)
{
	if(!has_focus)
		return;
	
	char c = ev->keyboard.unichar;
	if(((c >= '0') && (c <= '9')) || (!numbers_only && (((c >= 'a') && (c <= 'z')) || ((c >= 'A') && (c <= 'Z')) || (c == '_') || (c == '-'))))
	{
		if(value.size() < max_value_length)
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

menu_button::menu_button() : gui_element(200, 30), font_size(25), centre_aligned(true), was_clicked(false), name("Missing name") 
{
}

menu_button::menu_button(const std::string& name, bool centre_aligned, int font_size_par, int length_par, int height_par) 
			: gui_element(std::max((int)name.size() * (font_size_par - 10), length_par), height_par)
				, font_size(font_size_par), centre_aligned(centre_aligned), was_clicked(false), name(name) 
{
} 
	

void menu_button::mouse_down(int mouse_x, int mouse_y)
{
	mouse_axes(mouse_x, mouse_y);
	if(has_focus)
		was_clicked = true;
}

void menu_button::mouse_axes(int mouse_x, int mouse_y)
{
	if(((mouse_x > x) && (mouse_x < x + length)) && ((mouse_y > y) && (mouse_y < y + height)))
		has_focus = true;
	else
		has_focus = false;
}

void menu_button::enter_down()
{
	if(highlighted || has_focus)
		was_clicked = true;
}

void menu_button::draw()
{
	ALLEGRO_COLOR c = WRITING_COLOR;
	if(has_focus)
		c = WHITE_COLOR;
	
	ALLEGRO_FONT* f;
	switch(font_size)
	{
		case(25):
			f = font25;
			break;
		case(30):
			f = font30;
			break;
		case(35):
			f = font35;
			break;
		default:
			throw std::exception();
	}

	if(centre_aligned)
		al_draw_text(f, c, x + length/2, y, ALLEGRO_ALIGN_CENTRE, name.c_str());
	else
		al_draw_text(f, c, x, y, ALLEGRO_ALIGN_LEFT, name.c_str());
}

bool menu_button::clicked()
{
	bool return_value = was_clicked;
	was_clicked = false;
	return return_value;
}

void switch_button::mouse_down(int mouse_x, int mouse_y)
{
	mouse_axes(mouse_x, mouse_y);
	if(has_focus)
		value = !value;
}

void switch_button::mouse_axes(int mouse_x, int mouse_y)
{
	if(((mouse_x > x + name_length) && (mouse_x < x + name_length + value_length)) && ((mouse_y > y) && (mouse_y < y + height)))
		has_focus = true;
	else
		has_focus = false;
}

void switch_button::enter_down()
{
	if(highlighted || has_focus)
		value = !value;
}

void switch_button::draw()
{
	std::string text;
	if(value)
		text = "Enabled";
	else
		text = "Disabled";

	ALLEGRO_COLOR c;
	if(has_focus)
		c = WHITE_COLOR;
	else
		c = WRITING_COLOR;
	
	//al_draw_bitmap(image_list[BUTTON_BACKGROUND_IMAGE], x, y, 0);

	al_draw_text(font25, WRITING_COLOR, x, y, ALLEGRO_ALIGN_LEFT, name.c_str());
	al_draw_text(font25, c, x + name_length, y, ALLEGRO_ALIGN_LEFT, text.c_str());
}

text_element::text_element(const std::vector<std::string> lines_of_text) 
	: gui_element(letter_size * 
	(std::max_element(lines_of_text.begin(), lines_of_text.end(), [] (std::string s1, std::string s2) {return s1.size() < s2.size();})->size())
	+ 20, line_height * lines_of_text.size()), lines_of_text(lines_of_text)
{
}

void text_element::draw()
{
	for(int i=0; i<lines_of_text.size(); ++i)
	{
		al_draw_text(font25, WRITING_COLOR, x + 10, y + i * line_height, ALLEGRO_ALIGN_LEFT, lines_of_text[i].c_str());
	}
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
	playing = true;
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
	playing = false;
	al_stop_sample_instance(sample_playing);
}

void music::update()
{
	if((game_info::music) && playing)
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


