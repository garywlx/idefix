#include "MovingAverage.h"
#include "../FIXManager.h"

namespace IDEFIX {
	MovingAverage::MovingAverage() {

	}

	// Get unique name of indicator
	std::string MovingAverage::getName() const {
		return "MovingAverage";
	}

	// Update indicator
	void MovingAverage::onTick(FIXManager &manager, const MarketSnapshot &snapshot) {

	}
};