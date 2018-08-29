#ifndef IDEFIX_CHART_H
#define IDEFIX_CHART_H

#include "indicator/Tick.h"
#include "indicator/AbstractIndicator.h"
#include "strategies/AbstractStrategy.h"
#include <vector>
#include <string>
#include <quickfix/Mutex.h>

namespace IDEFIX {
	class Chart {
	public:
		enum Type {
			TICK_ONLY,
			RENKO
		};

		Chart();
		~Chart();
		void on_init();
		void on_exit();
		void add_indicator(AbstractIndicator* indicator);
		void set_symbol(const std::string& symbol);
		std::string symbol() const;
		void set_strategy(AbstractStrategy* strategy);
		AbstractStrategy* strategy();
		std::vector<Tick> ticks();

		// this needs to be overwritten by derived class
		virtual void add_tick(const Tick& tick) = 0;
		

	protected:
		std::string m_symbol;
		std::vector<AbstractIndicator*> m_indicators;
		std::vector<Tick> m_ticks;
		AbstractStrategy* m_strategy;
		FIX::Mutex m_mutex;
	};
};

#endif