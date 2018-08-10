#ifndef IDEFIX_MOVINGAVERAGE_H
#define IDEFIX_MOVINGAVERAGE_H

#include "Indicator.h"

namespace IDEFIX {
	class MovingAverage: public Indicator {
	private:

	public:
		MovingAverage();
		// Get name of this indicator, must be unique
		std::string getName() const;
		// Is called on every tick
		void onTick(FIXManager& manager, const MarketSnapshot& snapshot);
	};
};

#endif