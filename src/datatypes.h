#ifndef IDEFIX_DATATYPES_H
#define IDEFIX_DATATYPES_H

#include <iostream>
#include <iomanip> // std::setprecision
#include <vector>
#include <deque>
#include <cmath>
#include <map>

namespace idefix {
	/*!
	 * Asset Structure
	 */
	struct asset_struct {
		// name
		std::string name;
		// contract size
		unsigned int contract_size;
		// price decimal places
		unsigned int decimal_places;
		// market open
		std::string market_open_dt;
		// market close
		std::string market_close_dt;
	};

	/*!
	 * Overloading operator<< for asset_struct
	 */
	inline std::ostream& operator<< (std::ostream& os, const idefix::asset_struct asset){
		os << "[ASSET] " << asset.name << ";";
		os << " contract_size: " << asset.contract_size << ";";
		os << " market_open_dt: " << asset.market_open_dt << ";";
		os << " market_close_dt: " << asset.market_close_dt << ";";
		os << " decimal_places: " << asset.decimal_places << ";";

		return os;
	};

	/*!
	 * Tick Structure
	 */
	struct tick_struct {
		// tick symbol
		std::string symbol;
		// tick datetime
		std::string datetime;
		// bid price
		double bid;
		// ask price
		double ask;
		// spread
		double spread;
		// price decimal places
		unsigned int decimal_places;
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
		os << "[TICK] " << tick.symbol << ";";
		os << " dt: " << tick.datetime << ";";
		os.precision(tick.decimal_places);
		os << " bid: " << std::fixed << tick.bid << ";";
		os << " ask: " << std::fixed << tick.ask << ";";
		os << " spread: " << std::fixed << std::setprecision(2) << tick.spread << ";";
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
		// the symbol of this bar
		std::string symbol;
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
	 * Short definition of associative bar map
	 */
	typedef std::map<std::string, bar_deq_t> bar_map_t;

	/*!
	 * Overloading operator<< for bar_struct
	 */
	inline std::ostream& operator<< (std::ostream& os, const idefix::bar_struct bar){
		os << "[BAR] " << bar.symbol << ";";
		os << " dt: " << bar.datetime << ";";
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
};

#endif // IDEFIX_DATATYPES_H