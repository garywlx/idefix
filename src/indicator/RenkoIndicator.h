#ifndef IDEFIX_RANGEINDICATOR_H
#define IDEFIX_RANGEINDICATOR_H

#include "Indicator.h"
#include "RenkoBrick.h"
#include <vector>
#include <functional>

namespace IDEFIX {

	namespace RENKO {
		typedef std::function<void(const RenkoBrick&)> ONCLOSE_CB_T;
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
		// onClose callback
		RENKO::ONCLOSE_CB_T m_on_close_cb;

	public:
		RenkoIndicator(const double brick_size, const PriceType price_type, RENKO::ONCLOSE_CB_T callback = nullptr);
		std::string getName() const;
		void onTick(FIXManager& manager, const MarketSnapshot& snapshot);

		RenkoBrick getLatest();

	private:
		// reset current brick with defaults
		void resetCurrentBrick();
		// call onClose event with last renko brick
		void onClose(const RenkoBrick& brick);
	};
};

#endif