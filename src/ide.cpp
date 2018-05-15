#include "ide.h"
#include "globals.h"
#include <map>
#include <vector>

namespace idefix {	

	/*!
	 * Set current symbol
	 * @param const std::string& symbol like EUR/USD
	 */
	void set_symbol(const std::string& symbol) noexcept {
		g_symbol = symbol;
	}

	/*!
	 * Get current symbol
	 * @return std::string
	 */
	std::string symbol() noexcept {
		return g_symbol;
	}

	/*!
	 * Add new tick to global g_ticklist for current symbol
	 * @param tick_struct tick
	 */
	void add_tick(idefix::tick_struct tick){
		// search for symbol in ticklist
		if(g_ticklist.find(symbol()) != g_ticklist.end()){
			// found, add tick
			g_ticklist[symbol()].push_back(tick);
		} else {
			// not found, add new pair
			idefix::tick_vect_t ticks;
			ticks.push_back(tick);
			g_ticklist.insert(std::make_pair(symbol(), ticks));
		}
	}

	/*!
	 * Add new bar to global g_barlist for current symbol
	 * @param bar_struct bar
	 */
	void add_bar(idefix::bar_struct bar){
		// search for symbol in barlist
		if(g_barlist.find(symbol()) != g_barlist.end()){
			// found, add bar
			g_barlist[symbol()].push_back(bar);
		} else {
			// not found, add new pair
			idefix::bar_vec_t bars;
			bars.push_back(bar);
			g_barlist.insert(std::make_pair(symbol(), bars));
		}
	}

	void debug_bar_list(){
		auto list = g_barlist[symbol()];
		auto it = list.begin();
		std::cout << "[BARLIST] " << symbol() << std::endl;
		while(it != list.end()){
			std::cout << *it << std::endl;
			it++;
		} 
	}

	void debug_tick_list(){
		auto list = g_ticklist[symbol()];
		auto it = list.begin();
		std::cout << "[TICKLIST] " << symbol() << std::endl;
		while(it != list.end()){
			std::cout << *it << std::endl;
			it++;
		} 
	}
};

