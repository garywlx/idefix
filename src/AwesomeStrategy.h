#ifndef IDEFIX_AWESOME_STRATEGY_H
#define IDEFIX_AWESOME_STRATEGY_H

#include "MarketSnapshot.h"
#include "MarketOrder.h"
#include "RenkoChart.h"
#include "Account.h"
#include "SimpleMovingAverage.h"
#include "Bar.h"
#include "SignalType.h"
#include "MarketSide.h"
#include <string>
#include <nod/nod.hpp>
#include <quickfix/Mutex.h>
#include <quickfix/FIXFields.h>

namespace IDEFIX {
	struct AwesomeStrategyConfig {
		// how many parallel short positions are allowed?
		int max_short_pos;
		// how many parallel long positions are allowed?
		int max_long_pos;
		// wait for at least 5 bricks before entering the markets
		int wait_bricks;
		// SMA size
		int sma_size;
		// maximum risk per trade in percent
		double max_risk;
		// maximum quantity size
		double max_qty;
		// maximum spread to open a position
		double max_spread;
		// Renko brick size
		double renko_size;
	};

	class AwesomeStrategy {
	public:
		AwesomeStrategy(const std::string& symbol, const AwesomeStrategyConfig& config);
		~AwesomeStrategy();

		// Signals
		nod::signal<void(const MarketSide side)> on_entry_signal;
		nod::signal<void(const std::string&)> on_close_all_signal;
		nod::signal<void(const Bar&)> on_bar_signal;

		// Slots
		void on_tick(const MarketSnapshot& tick);
		void on_init();
		void on_exit();
		void on_bar(const Bar& bar);
		void on_market_order(const SignalType type, const MarketOrder& mo);
		
		std::string& get_symbol();
		double get_max_risk() const;
		double get_max_qty() const;
		double get_max_spread() const;

	private:
		SimpleMovingAverage* m_sma5;
		RenkoChart* m_chart;
		std::string m_symbol;

		int m_max_long_pos;
		int m_max_short_pos;
		int m_long_pos;
		int m_short_pos;
		int m_wait_bricks;
		int m_sma_size;
		double m_max_risk;
		double m_max_qty;
		double m_max_spread;
		double m_current_spread;
		double m_renko_size;

		FIX::Mutex m_mutex;

		void log_brick(const Bar& bar, const double sma);
	};
};

#endif