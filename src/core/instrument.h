#pragma once

#include <string>
#include <utility>
#include <vector>

#include "exchangetypes.h"

namespace idefix {
class Instrument {
public:
	Instrument();
	Instrument(const std::string& symbol);
	Instrument(const std::string& symbol, const double ask, const double bid, const double point);

	// Formats the given value as a string.
	std::string format(const double value);
	// Gets the current ask price for this instrument.
	double getAskPrice();
	// Gets the current bid price for this instrument.
	double getBidPrice();
	// Gets the current high price for this instrument
	double getHighPrice();
	// Gets the current low price for this instrument
	double getLowPrice();
	// Gets the size of a 'point'.
	double getPointSize();
	// Gets the difference between the bid and ask price for this instrument.
	double getSpread();
	// Get the volume
	int getVolume();
	// Gets the symbol for this instrument.
	std::string getSymbol() const;
	// Rounds the given value to the minimum tick size.
	double round(const double value);
	// Add tick
	void addTick(const ExchangeTick tick);

	// relational operator
	inline bool operator==(Instrument& x) {
		return ( x.getSymbol() == getSymbol() );
	}

	inline bool operator<(const Instrument& x) const {
		return false;
	}
	// bool operator<(Instrument& x) {
	// 	return ( x.getSymbol() == getSymbol() && x.getAskPrice() < getAskPrice() && x.getBidPrice() < getBidPrice() );
	// }

private:
	std::string m_symbol;
	double m_ask_price;
	double m_bid_price;
	double m_point_size;
	double m_high_price;
	double m_low_price;
	std::vector<ExchangeTick> m_ticks;

};
};