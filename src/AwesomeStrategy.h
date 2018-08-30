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
	class AwesomeStrategy {
	public:
		AwesomeStrategy(const std::string& symbol);
		~AwesomeStrategy();

		// Signals
		nod::signal<void(const MarketSide side)> on_entry_signal;
		nod::signal<void(const std::string&)> on_close_all_signal;

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
		double m_max_risk;
		double m_max_qty;
		double m_max_spread;
		double m_current_spread;

		FIX::Mutex m_mutex;
	};
};

#endif