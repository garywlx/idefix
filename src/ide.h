#ifndef IDEFIX_H
#define IDEFIX_H

#include <iostream>
#include <iomanip> // std::setprecision
#include <vector>
#include <deque>
#include <cmath>
#include <map>

namespace idefix {

	/*!
	 * Tick Structure
	 */
	struct tick_struct {
		// tick datetime
		std::string datetime;
		// bid price
		double bid;
		// ask price
		double ask;
		// price digits
		int digits;
	};

	/*!
	 * Short type definition of tick deque
	 */
	typedef std::deque<tick_struct> tick_deq_t;

	/*!
	 * Short type definition of associative tick map
	 */
	typedef std::map<std::string, tick_deq_t> tick_map_t;

	/*!
	 * Overloading operator<< for tick_struct
	 */
	inline std::ostream& operator<< (std::ostream& os, const idefix::tick_struct tick){
		os << "[TICK] " << tick.datetime << ";";
		os.precision(tick.digits);
		os << " bid: " << std::fixed << tick.bid << ";";
		os << " ask: " << std::fixed << tick.ask << ";";
		return os;
	}

	/*!
	 * Candle type
	 */
	enum candle_type {
		bullish, bearish
	};

	/*!
	 * Overloading operator<< for candle_type enum
	 */
	inline std::ostream& operator<< (std::ostream& os, const idefix::candle_type ctype){
		os << (ctype == candle_type::bullish ? "bullish" : "bearish");
		return os;
	}

	/*!
	 * Candle Bar Structure
	 */
	struct bar_struct {
		// candle datetime
		std::string datetime;
		// price data
		double high, low, open, close, volume;
		// candle type
		candle_type type;
		// price digits
		int digits;
		// vector of all ticks inside period
		std::vector<tick_struct*> tick_v;
	};

	/*!
	 * Short definition of bar deque
	 */
	typedef std::deque<bar_struct> bar_deq_t;

	/*!
	 * Overloading operator<< for bar_struct
	 */
	inline std::ostream& operator<< (std::ostream& os, const idefix::bar_struct bar){
		os << "[BAR] " << bar.datetime << ";";
		os.precision(bar.digits);
		os << " O: " << std::fixed << bar.open << ";";
		os << " H: " << std::fixed << bar.high << ";";
		os << " L: " << std::fixed << bar.low << ";";
		os << " C: " << std::fixed << bar.close << ";";
		os << " V: " << bar.tick_v.size() << ";";
		os << " T: " << (bar.type == idefix::candle_type::bullish ? "bullish" : "bearish") << ";";
		os << " D: " << bar.digits << ";";
		return os;
	}

	/*!
	 * Position type long or short
	 */
	enum pos_type {
		tlong, tshort
	};

	/*!
	 * Overloading operator<< for pos_type enum
	 */
	inline std::ostream& operator<< (std::ostream& os, const pos_type ptype){
		os << (ptype == pos_type::tlong ? "long" : "short");
		return os;
	}

	/*!
	 * Get tick value of position pos
	 * @param int	pos current tick is 0, previous tick is 1...
	 * @return tick_struct
	 */
	tick_struct tick(const int pos);

	/*!
	 * Get tick list size of current symbol
	 * @return unsigned int
	 */
	unsigned int ticks();

	/*!
	 * Get bar value of position pos
	 * @param  int	pos current bar is 0, previous bar is 1...
	 * @return bar_struct
	 */
	bar_struct bar(const int pos);

	/*!
	 * Get bar list size of current symbol
	 * @return unsigned int
	 */
	unsigned int bars();

	/*!
	 * Get bar open value of position pos
	 * @param int pos current bar is 0, previous bar is 1...
	 * @return double 
	 */
	double open(const int pos) noexcept;

	/*!
	 * Get bar high value of position pos
	 * @param int pos current bar is 0, previous bar is 1...
	 * @return double
	 */
	double high(const int pos) noexcept;

	/*!
	 * Get bar low value of position pos
	 * @param int pos current bar is 0, previous bar is 1...
	 * @return double
	 */
	double low(const int pos) noexcept;

	/*!
	 * Get bar close value of position pos
	 * @param int pos current bar is 0, previous bar is 1...
	 * @return double
	 */
	double close(const int pos) noexcept;

	/*!
	 * Get current ask price
	 * @return double
	 */
	double ask() noexcept;

	/*!
	 * Get current bid price
	 * @return double
	 */
	double bid() noexcept;

	/*!
	 * Get current spread
	 * @return double
	 */
	double spread() noexcept;

	/*!
	 * Get current balance
	 * @return double
	 */
	double balance() noexcept;

	/*!
	 * Switch current active symbol
	 * @param std::string symbol EUR/USD
	 */
	void set_symbol(const std::string& symbol) noexcept;

	/*!
	 * Get current active symbol
	 * @return std::string
	 */
	std::string symbol() noexcept;

	/*!
	 * Add new tick to global tick list
	 * @param tick_struct tick
	 */
	 void add_tick(idefix::tick_struct tick);

	 /*!
	  * Add new bar to global bar list
	  * @param bar_struct bar
	  */
	 void add_bar(idefix::bar_struct bar);

	 /*!
	  * Check if list contains the symbol
	  */
	 template <typename LIST_TYPE> 
	 bool hasSymbol(LIST_TYPE list, const std::string symbol);



	 void debug_bar_list();
	 void debug_tick_list();
};

#endif // IDEFIX_H