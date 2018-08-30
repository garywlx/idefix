#ifndef IDEFIX_RENKO_STRATEGY_H
#define IDEFIX_RENKO_STRATEGY_H

#include "AbstractStrategy.h"
#include <quickfix/Mutex.h>
#include "../indicator/SimpleMovingAverage.h"

namespace IDEFIX {
	class Chart;
	class MarketSnapshot;
	class Bar;
	
	class RenkoStrategy: public AbstractStrategy {
	private:
		FIX::Mutex m_mutex;
		SimpleMovingAverage m_sma5;

		int m_max_long_pos;
		int m_max_short_pos;
		int m_long_pos;
		int m_short_pos;
		int m_wait_bricks;
		double m_max_risk;
		
	public:
		RenkoStrategy();

		// is called on initialization
		void on_init(Chart* chart);
		// is called on chart tick
		void on_tick(Chart* chart, const MarketSnapshot& tick);
		// is called on chart bar
		void on_bar(Chart* chart, const Bar& bar);
		// is called on exit
		void on_exit(Chart* chart);
	};
};

#endif