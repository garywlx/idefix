#ifndef IDEFIX_RANGEINDICATOR_H
#define IDEFIX_RANGEINDICATOR_H

#include "Indicator.h"
#include <vector>

namespace IDEFIX {
	struct RenkoBrick {
		int tick_volume;
		double open_price;
		double close_price;
		int status; // -1 down, 0 uninitialized, 1 up
		std::string close_time;
	};

	class RenkoIndicator: public Indicator {
	private:
		// the size of the renko brick
		double m_brick_size;
		// which price to use
		PriceType m_price_type;
		// current brick
		RenkoBrick m_current_brick;
		// hold all bricks
		std::vector<RenkoBrick> m_bricks;

	public:
		RenkoIndicator(const double brick_size, const PriceType price_type);
		std::string getName() const;
		void onTick(FIXManager& manager, const MarketSnapshot& snapshot);

		void resetCurrentBrick();
		RenkoBrick getLatest();
		
	};
};

#endif