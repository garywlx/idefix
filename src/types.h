#ifndef IDEFIX_TYPES_H
#define IDEFIX_TYPES_H

#include "quickfix/Application.h"

namespace IDEFIX {

struct MarketOrder {
	FIX::ClOrdID clOrdID;
	std::string posID;
	FIX::Symbol symbol;
	FIX::OrderQty qty;
	FIX::Side side;
	FIX::Price price;
	FIX::StopPx stopPrice;
	FIX::Price takePrice;
	FIX::Account accountid;
	double plValue;
	FIX::OrdStatus ordStatus;
};

struct MarketSnapshot {
	FIX::Symbol symbol;
	double bid;
	double ask;
	double spread;
	std::string sending_time;
};

// Check if marketorder is valid and has a clordid
inline bool isValidMarketOrder(const IDEFIX::MarketOrder& mo){
	return !mo.clOrdID.getValue().empty();
}

// Get new marketorder struct with prefilled values
inline MarketOrder getNewMarketOrderStruct(const FIX::Symbol symbol){
	MarketOrder mo = {
		FIX::ClOrdID("0"),
		"",
		FIX::Symbol(symbol),
		FIX::OrderQty(0),
		FIX::Side(FIX::Side_BUY),
		FIX::Price(0),
		FIX::StopPx(0),
		FIX::Price(0),
		FIX::Account("0"),
		0,
		FIX::OrdStatus(FIX::ListOrderStatus_EXECUTING)
	};

	return mo;
}

};

inline std::ostream& operator<<(std::ostream& out, const IDEFIX::MarketSnapshot& ms){
	out << "[MarketSnapshot] " << ms.symbol << " bid: " << ms.bid << " ask: " << ms.ask << " spread: " << ms.spread << " ts: " << ms.sending_time;
	return out;
}

inline std::ostream& operator<<(std::ostream& out, const IDEFIX::MarketOrder& mo){
	out << "[MarketOrder] ClOrdID " << mo.clOrdID
		<< " posID " << mo.posID
		<< " symbol " << mo.symbol
		<< " qty " << mo.qty 
		<< " side " << (mo.side == FIX::Side_BUY ? "Side_BUY" : "Side_SELL")
		<< " price " << mo.price 
		<< " stopPrice " << mo.stopPrice
		<< " takePrice " << mo.takePrice
		<< " account " << mo.accountid
		<< " plValue " << mo.plValue
		<< " ordStatus " << mo.ordStatus;

	return out;
}
#endif