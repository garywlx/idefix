#ifndef IDEFIX_TYPES_H
#define IDEFIX_TYPES_H

#include "quickfix/Application.h"

namespace IDEFIX {

struct MarketOrder {
  FIX::Symbol symbol;
  FIX::OrderQty qty;
  FIX::Side side;
  FIX::Price price;
  FIX::OrdType ordtype;
  FIX::Account accountid;
};

struct MarketSnapshot {
	FIX::Symbol symbol;
	double bid;
	double ask;
	double spread;
	std::string sending_time;
};

};

inline std::ostream& operator<<(std::ostream& out, const IDEFIX::MarketSnapshot& ms){
	out << "[MarketSnapshot] " << ms.symbol << " bid: " << ms.bid << " ask: " << ms.ask << " spread: " << ms.spread << " ts: " << ms.sending_time;
	return out;
}
#endif