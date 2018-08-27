#ifndef IDEFIX_ABSTRACT_INDICATOR_H
#define IDEFIX_ABSTRACT_INDICATOR_H

#include "Tick.h"

namespace IDEFIX {

class Chart;

struct AbstractIndicator {
	// Unique identifier for the indicator
	virtual std::string name() const = 0;
	// is called on initialization in chart
	virtual void on_init(Chart& chart)=0;
	// Handle on tick method
	virtual void on_tick(const Tick& tick) = 0;
	// is called on exiting that chart
	virtual void on_exit(Chart& chart)=0;
};

};

#endif