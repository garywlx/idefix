#ifndef IDEFIX_SIMPLEMOVINGAVERAGE_H
#define IDEFIX_SIMPLEMOVINGAVERAGE_H

#include "AbstractMovingAverage.h"

namespace IDEFIX {
class SimpleMovingAverage: public AbstractMovingAverage {
public:
	SimpleMovingAverage();
	SimpleMovingAverage(const int period);
	~SimpleMovingAverage();

	double value();
	void add(const double value);
	void clear();
	bool is_valid();
};
};

#endif