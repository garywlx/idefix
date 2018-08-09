#ifndef IDEFIX_INDICATOR_H
#define IDEFIX_INDICATOR_H

#include <string>

namespace IDEFIX {
	class FIXManager;
	class MarketSnapshot;
	
	/*!
	 * Abstract Base Class of Indicator
	 */
	class Indicator {
		public:
			enum PriceType {
				BID_PRICE, ASK_PRICE
			};

			// Get name of this indicator, must be unique
			virtual std::string getName() const =0;
			// Is called on every tick
			virtual void onTick(FIXManager& manager, const MarketSnapshot& snapshot) =0;
	};
};

#endif