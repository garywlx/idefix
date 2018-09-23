#include "gui.h"

namespace IDEFIX {

	Gui::Gui() {
		initscr();
		cbreak();
		noecho();
		clear();
		curs_set(0);

		// get maximum width and height of stdscr
		getmaxyx( stdscr, m_screen_height, m_screen_width );

		// defaults
		m_last_price_y = 0;
		m_last_main_y = 0;
	}

	Gui::~Gui() {
		// destruct windows
		wrefresh( m_main_wnd );
		wrefresh( m_price_wnd );
		delwin( m_main_wnd );
		delwin( m_price_wnd );

		endwin();
	}

	/*!
	 * Initialize main and price windows
	 */
	void Gui::init_windows() {
		// initialize windows
		UIWindow main_wnd;
		main_wnd.width = m_screen_width - 30;
		main_wnd.height = m_screen_height;
		main_wnd.x = 0;
		main_wnd.y = 0;

		UIWindow price_wnd;
		price_wnd.width = m_screen_width - main_wnd.width + 1;
		price_wnd.height = main_wnd.height;
		price_wnd.x = main_wnd.width - 1;
		price_wnd.y = main_wnd.y;

		// main window
		m_main_wnd  = newwin( main_wnd.height, main_wnd.width, main_wnd.y, main_wnd.x );
		scrollok( m_main_wnd, TRUE );
		keypad( m_main_wnd, TRUE );

		// price window
		m_price_wnd = newwin( price_wnd.height, price_wnd.width, price_wnd.y, price_wnd.x );

		// ls, rs, ts, bs, tl, tr, bl, br
		wborder( m_main_wnd, ACS_VLINE, ACS_VLINE, ACS_HLINE, ACS_HLINE, ACS_ULCORNER, ACS_URCORNER, ACS_LLCORNER, ACS_LRCORNER );
		wborder( m_price_wnd, ACS_VLINE, ACS_VLINE, ACS_HLINE, ACS_HLINE, ACS_TTEE, ACS_URCORNER, ACS_BTEE, ACS_LRCORNER );

		// main title
		char main_title[] = " IDEFIX ";
		mvwaddch( m_main_wnd, 0, 2, ACS_RTEE );
		mvwaddstr( m_main_wnd, 0, 3, main_title );
		mvwaddch( m_main_wnd, 0, sizeof( main_title ) + 2, ACS_LTEE );

		// price title
		char price_title[] = " PRICES ";
		mvwaddch( m_price_wnd, 0, 2, ACS_RTEE );
		mvwaddstr( m_price_wnd, 0, 3, price_title );
		mvwaddch( m_price_wnd, 0, sizeof( price_title ) + 2, ACS_LTEE );
	}

	/*!
	 * Draw manipulations to screen
	 */
	void Gui::draw() {
		refresh();
		wrefresh( m_main_wnd );
		wrefresh( m_price_wnd );
	}

	/*!
	 * Command handling
	 */
	void Gui::cmd() {
		// help text
		char exit_text[] = " Press F1 to quit. ";
		mvwaddch( m_main_wnd, m_screen_height - 1, 2, ACS_RTEE );
		mvwaddstr( m_main_wnd, m_screen_height - 1, 3, exit_text );
		mvwaddch( m_main_wnd, m_screen_height - 1, sizeof( exit_text ) + 2, ACS_LTEE );

		int ch;
		while ( ( ch = wgetch( m_main_wnd ) ) != KEY_F( 1 ) ) {
			switch( ch ) {
				// handle something
			}
		}
	}

	/*!
	 * Add text line to main window
	 * @param const std::string text
	 */
	void Gui::add_main_line(const std::string text) {
		m_last_main_y++;
		mvwaddstr( m_main_wnd, m_last_main_y, 1, text.c_str() );
	}

	/*!
	 * Add symbol with price to price window
	 * 
	 * @param const std::string symbol
	 * @param const double      bid
	 * @param const double      ask
	 * @param const int         point_size defaults to 5, set to 3 for JPY
	 */
	void Gui::add_price_line(const std::string symbol, const double bid, const double ask, const int point_size) {
		m_last_price_y++;
		char text[30];
		if ( point_size == 5 ) {
			sprintf( text, " %s     %.5f/%.5f", symbol.c_str(), bid, ask );	
		} else if ( point_size == 3 ) {
			sprintf( text, " %s     %.3f/%.3f", symbol.c_str(), bid, ask );
		}
		
		mvwaddstr( m_price_wnd, m_last_price_y, 1, text );
	}
};