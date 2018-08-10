#ifndef IDEFIX_RENKOBRICK_H
#define IDEFIX_RENKOBRICK_H

#include <string>

namespace IDEFIX {
	struct RenkoBrick {
		int tick_volume;
		double open_price;
		double close_price;
		int status; // -1 down, 0 uninitialized, 1 up
		std::string close_time;
		std::string symbol;
	};
};

#endif