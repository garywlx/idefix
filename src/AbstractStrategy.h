#ifndef IDEFIX_ABSTRACTSTRATEGY_H
#define IDEFIX_ABSTRACTSTRATEGY_H

#include "indicator/Tick.h"

namespace IDEFIX {
	struct AbstractStrategy {
		// On tick handler
		virtual void on_tick(const Tick& tick)=0;
	};
};

#endif