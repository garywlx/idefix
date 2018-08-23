#ifndef IDEFIX_TICK_H
#define IDEFIX_TICK_H

#include <iostream>
#include <sstream>
#include <iomanip>

namespace IDEFIX {
struct Tick {
	std::string datetime;
	double bid;
	double ask;
	double point_size;
};
};

inline std::ostream& operator<<(std::ostream& out, const IDEFIX::Tick& tick) {
	out << "Tick " << tick.datetime 
		<< " bid " << std::setprecision(5) << std::fixed << tick.bid 
	    << " ask " << std::setprecision(5) << std::fixed << tick.ask
	    << " point_size " << std::setprecision(4) << std::fixed << tick.point_size;
	return out;
};

#endif