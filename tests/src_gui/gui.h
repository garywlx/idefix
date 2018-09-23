#ifndef IDEFIX_GUI_H
#define IDEFIX_GUI_H

#include <ncurses.h>
#include <stdlib.h>
#include <string>

namespace IDEFIX {

	struct UIWindow {
		int width;
		int height;
		int x;
		int y;
		char title[];
	};

	class Gui {
	private:
		WINDOW* m_main_wnd;
		WINDOW* m_price_wnd;
		int m_screen_width;
		int m_screen_height;

		int m_last_price_y;
		int m_last_main_y;

	public:
		Gui();
		~Gui();

		void init_windows();
		void draw();
		void cmd();

		void add_price_line(const std::string symbol, const double bid, const double ask, const int point_size = 5);
		void add_main_line(const std::string text);
	};
};

#endif