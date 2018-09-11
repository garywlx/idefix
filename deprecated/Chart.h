#ifndef IDEFIX_CHART_H
#define IDEFIX_CHART_H

#include "indicator/AbstractIndicator.h"
#include <vector>
#include <string>
#include <quickfix/Mutex.h>
#include "MarketSnapshot.h"

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
		// this needs to be overwritten by derived class
		virtual void on_tick(const MarketSnapshot& tick) = 0;

		void add_indicator(AbstractIndicator* indicator);
		void set_symbol(const std::string& symbol);
		std::string symbol() const;

	protected:
		std::string m_symbol;
		std::vector<AbstractIndicator*> m_indicators;
		FIX::Mutex m_mutex;
	};
};

#endif