#include "gui.h"
#include <vector>
#include <sstream>

using namespace IDEFIX;
using namespace std;

int main()
{	
	Gui ui;
	ui.init_windows();

	ui.add_price_line("AUD/USD", 1.12349, 1.12340);
	ui.add_price_line("EUR/USD", 1.12349, 1.12340);
	ui.add_price_line("GBP/USD", 1.12349, 1.12340);
	ui.add_price_line("NZD/USD", 1.12349, 1.12340);
	ui.add_price_line("USD/CAD", 1.12349, 1.12340);
	ui.add_price_line("USD/CHF", 1.12349, 1.12340);
	ui.add_price_line("USD/JPY", 111.222, 111.333, 3);

	int max_height = getmaxy(stdscr);
	max_height += 5;
	for( int i = 0; i < max_height; i++ ) {
		
		ostringstream os;
		os << "TEXT " << i;

		ui.add_main_line( os.str() );
	}
 	
	ui.draw();
	ui.cmd();

	return 0;
}
