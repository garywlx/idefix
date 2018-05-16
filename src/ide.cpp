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
		//if(g_ticklist.find(symbol()) != g_ticklist.end()){
		if( hasSymbol(g_ticklist, symbol()) ){
			// found, add tick
			g_ticklist[symbol()].push_front(tick);
		} else {
			// not found, add new pair
			idefix::tick_deq_t ticks;
			ticks.push_front(tick);
			g_ticklist.insert(std::make_pair(symbol(), ticks));
		}
	}

	/*!
	 * Add new bar to global g_barlist for current symbol
	 * @param bar_struct bar
	 */
	void add_bar(idefix::bar_struct bar){
		// search for symbol in barlist
		//if(g_barlist.find(symbol()) != g_barlist.end()){
		if( hasSymbol(g_barlist, symbol()) ){
			// found, add bar
			g_barlist[symbol()].push_front(bar);
		} else {
			// not found, add new pair
			idefix::bar_deq_t bars;
			bars.push_front(bar);
			g_barlist.insert(std::make_pair(symbol(), bars));
		}
	}

	/*!
	 * Check if list contains the symbol
	 */
	template <typename LIST_TYPE>
	bool hasSymbol(LIST_TYPE list, const std::string symbol){
		return (list.find(symbol) != list.end());
	}

	/*!
	 * Get tick at position
	 * @param const int pos 0 = latest, 1 = previous ...
	 * @return tick_struct
	 */
	tick_struct tick(const int pos) {
		if( ! hasSymbol(g_ticklist, symbol()) ){
			throw std::runtime_error{"Symbol " + symbol() + " not found."};
		}

		auto list = g_ticklist[symbol()];

		if( pos > (list.size() - 1 ) ){
			throw std::out_of_range{"pos is out of range"};
		}

		return list[pos];
	}

	/*!
	 * Get tick list size of current symbol
	 * @return unsigned int
	 */
	unsigned int ticks(){
		if( ! hasSymbol(g_ticklist, symbol()) ){
			throw std::runtime_error{"Symbol " + symbol() + " not found."};
		}

		auto list = g_ticklist[symbol()];

		return list.size();
	}

	/*!
	 * Get bar at position
	 * @param const int pos 0 = latest, 1 = previous ...
	 * @return bar_struct
	 */
	bar_struct bar(const int pos) {
		if( ! hasSymbol(g_barlist, symbol() )){
			throw std::runtime_error{"Symbol " + symbol() + "not found."};
		}

		auto list = g_barlist[symbol()];

		if( pos > (list.size() - 1) ){
			throw std::out_of_range{"pos is out of range."};
		}

		return list[pos];
	}

	/*!
	 * Get bar list size of current symbol
	 * @return unsigned int
	 */
	unsigned int bars(){
		if( ! hasSymbol(g_barlist, symbol()) ){
			throw std::runtime_error{"Symbol " + symbol() + " not found."};
		}

		auto list = g_barlist[symbol()];

		return list.size();	
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

