#ifndef IDEFIX_RENKO_STRATEGY_H
#define IDEFIX_RENKO_STRATEGY_H

#include "AbstractStrategy.h"
#include <quickfix/Mutex.h>

namespace IDEFIX {
	class Chart;
	class Tick;
	class Bar;
	
	class RenkoStrategy: public AbstractStrategy {
	private:
		FIX::Mutex m_mutex;
		
	public:
		RenkoStrategy();

		// is called on initialization
		void on_init(Chart& chart);
		// is called on chart tick
		void on_tick(Chart& chart, const Tick& tick);
		// is called on chart bar
		void on_bar(Chart& chart, const Bar& bar);
		// is called on exit
		void on_exit(Chart& chart);
	};
};

#endif