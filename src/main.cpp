#include <iostream>
#include "idefix.h"

int main(int argc, char** argv){
	
	idefix::tick_struct tick = {"04/22/2018 21:02:52.801",1.22789,1.22782,5};
	idefix::bar_struct bar = {1.22789,1.22782,1.22789,1.22782,5,idefix::candle_type::bullish,"04/22/2018 21:02:52.801",5};

	idefix::set_symbol("EUR/USD");
	idefix::add_tick(tick);
	idefix::add_tick(tick);
	idefix::add_bar(bar);

	idefix::debug_tick_list();
	idefix::debug_bar_list();

	idefix::set_symbol("NZD/USD");
	idefix::add_tick(tick);
	idefix::add_tick(tick);
	idefix::add_tick(tick);
	idefix::add_tick(tick);
	idefix::add_tick(tick);
	idefix::add_bar(bar);
	idefix::add_bar(bar);


	idefix::debug_tick_list();
	idefix::debug_bar_list();

	return 0;
}