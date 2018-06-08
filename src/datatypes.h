#ifndef IDEFIX_DATATYPES_H
#define IDEFIX_DATATYPES_H

#include <iostream>
#include <iomanip> // std::setprecision
#include <vector>
#include <deque>
#include <cmath>
#include <map>
#include <chrono>
#include <date/date.h>
#include "times.h"

namespace idefix {

	/*!
	 * Short type definition for system clock time point
	 */
	typedef std::chrono::system_clock::time_point sys_time_point_t;

	/*!
	 * Account Structure
	 */
	struct account_struct {
		// the account id
		unsigned int accountid;
		// the current balance
		double balance;
		// the current equity
		double equity;
		// the profit of today
		double today_profit;
		// the profit of today in percent
		double today_profit_percent;
		// the loss of today
		double today_loss;
		// the loss of today in percent
		double today_loss_percent;
		// the account currency
		std::string currency;
	};

	/*!
	 * Overloading operator<< for balance_struct
	 */
	inline std::ostream& operator<< (std::ostream& os, const idefix::account_struct account){
		os << "[BALANCE] acc: " << account.accountid << ";";
		os << " bal: " << std::setprecision(2) << account.balance << " " << account.currency << ";";
		os << " equ: " << std::setprecision(2) << account.equity << " " << account.currency << ";";
		os << " pro: " << std::setprecision(2) << account.today_profit << " " << account.currency << ";";
		os << " pro%:" << std::setprecision(2) << account.today_profit_percent << "%;";
		os << " los: " << std::setprecision(2) << account.today_loss << " " << account.currency << ";";
		os << " los%: " << std::setprecision(2) << account.today_loss_percent << "%;";

		return os;
	};

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
		// tick datetime string
		std::string datetime_str;
		// tick datetime
		sys_time_point_t datetime;
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
		using date::operator<<;
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
		// candle datetime open
		sys_time_point_t tp_open;
		// candle datetime close
		sys_time_point_t tp_close;
		// price data
		double high, low, open, close;
		// candle type
		candle_type type;
		// price decimal places
		unsigned int decimal_places;
		// vector of all ticks inside period
		tick_deq_t ticks;
		// tick count
		unsigned int volume;
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
		using date::operator<<;
		os << " tp_o: " << bar.tp_open << ";";
		os << " tp_c: " << bar.tp_close << ";"; 	
		os.precision(bar.decimal_places);
		os << " O: " << std::fixed << bar.open << ";";
		os << " H: " << std::fixed << bar.high << ";";
		os << " L: " << std::fixed << bar.low << ";";
		os << " C: " << std::fixed << bar.close << ";";
		os << " V: " << bar.volume << ";";
		os << " T: " << (bar.type == idefix::candle_type::bullish ? "bullish" : "bearish") << ";";
		os << " dp: " << bar.decimal_places << ";";
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