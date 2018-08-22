#ifndef IDEFIX_SIMPLEMOVINGAVERAGE_H
#define IDEFIX_SIMPLEMOVINGAVERAGE_H

#include "IMovingAverage.h"

namespace IDEFIX {
class SimpleMovingAverage: public IMovingAverage {
public:
	SimpleMovingAverage();
	SimpleMovingAverage(const int period);
	~SimpleMovingAverage();

	double value();
	void add(const double value);
	void clear();
};
};

#endif