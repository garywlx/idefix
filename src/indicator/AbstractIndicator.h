#ifndef IDEFIX_ABSTRACT_INDICATOR_H
#define IDEFIX_ABSTRACT_INDICATOR_H

#include "Tick.h"

namespace IDEFIX {

struct AbstractIndicator {
	// Unique identifier for the indicator
	virtual std::string name() const = 0;
	// Handle on tick method
	virtual void on_tick(const Tick& tick) = 0;
};

};

#endif