#ifndef IDEFIX_MOVINGAVERAGE_H
#define IDEFIX_MOVINGAVERAGE_H

#include "Indicator.h"
#include <vector>

namespace IDEFIX {
	class MovingAverage: public Indicator {
	private:
		int m_period;
		int m_offset;
		std::vector<double> m_prices;
		double m_current_value;

	public:
		MovingAverage(const int period, const int offset = 0);
		// Get name of this indicator, must be unique
		std::string getName() const;
		// Is called on every tick
		void onTick(FIXManager& manager, const MarketSnapshot& snapshot);
		void calculate(const double next_price);
		double getValue() const;
	};
};

#endif