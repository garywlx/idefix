#ifndef IDEFIX_CHART_H
#define IDEFIX_CHART_H

#include "indicator/Tick.h"
#include "indicator/AbstractIndicator.h"
#include <vector>
#include <string>

namespace IDEFIX {
	class Chart {
	public:
		enum Type {
			TICK_ONLY,
			RENKO
		};

		Chart();
		~Chart();
		void add_tick(const Tick& tick);
		void add_indicator(AbstractIndicator* indicator);

		void plot();
		void set_type(const Chart::Type type);
		Chart::Type type() const;
		void set_symbol(const std::string& symbol);
		std::string symbol() const;
		std::vector<Tick> ticks();

	protected:
		Chart::Type m_type;
		std::string m_symbol;
		std::vector<AbstractIndicator*> m_indicators;
		std::vector<Tick> m_ticks;
	};
};

#endif