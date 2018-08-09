#ifndef IDEFIX_RANGEINDICATOR_H
#define IDEFIX_RANGEINDICATOR_H

#include "Indicator.h"
#include <vector>
#include <string>

namespace IDEFIX {
	struct RenkoBrick {
		int tick_volume;
		double first_price;
		double last_price;
		int is_up; // -1 down, 0 uninitialized, 1 up
		std::string close_time;
	};

	class RenkoIndicator: public Indicator {
	private:
		// the size of the renko brick
		int m_brick_size;
		// which price to use
		PriceType m_price_type;
		// current brick
		RenkoBrick m_current_brick;
		// how many pips are we moved
		double m_pips_moved;

		// hold all bricks
		std::vector<RenkoBrick> m_bricks;

	public:
		RenkoIndicator(const int pip_range, const PriceType price_type);
		std::string getName() const;
		void onTick(FIXManager& manager, const MarketSnapshot& snapshot);

		void resetCurrentBrick();
		RenkoBrick getLatest();
	};
};

#endif