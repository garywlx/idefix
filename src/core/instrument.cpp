#include "instrument.h"
#include <cmath>

namespace idefix {
Instrument::Instrument(): Instrument("", 0, 0, 0) {}
Instrument::Instrument(const std::string& symbol): Instrument( symbol, 0, 0, 0 ) {}
Instrument::Instrument(const std::string& symbol, const double ask, const double bid, const double point): 
	m_symbol(symbol), m_ask_price(ask), m_bid_price(bid), m_point_size(point), m_high_price(ask), m_low_price(bid), m_volume(0) {}

/**
 * Formats the given value as a string.
 * 
 * @param const double value
 * @return std::string
 */
std::string Instrument::format(const double value) {
	return std::to_string( ::round( value ) );
}

/**
 * Gets the current ask price for this instrument.
 * 
 * @return double
 */
double Instrument::getAskPrice() {
	return m_ask_price;
}

/**
 * Gets the current bid price for this instrument.
 * 
 * @return double
 */
double Instrument::getBidPrice() {
	return m_bid_price;
}

/**
 * Gets the current high price for this instrument
 * 
 * @return double
 */
double Instrument::getHighPrice() {
	return m_high_price;
}

/**
 * Gets the current low price for this instrument
 * 
 * @return double
 */
double Instrument::getLowPrice() {
	return m_low_price;
}


/**
 * Gets the size of a 'point'.
 * 
 * @return double
 */
double Instrument::getPointSize() {
	return m_point_size;
}

/**
 * Gets the difference between the bid and ask price for this instrument.
 * 
 * @return double
 */
double Instrument::getSpread() {
	if ( m_point_size == 0 ) return -1;
	return ( m_bid_price - m_ask_price ) * ( 1 / m_point_size );
}

/**
 * Gets the volume
 * 
 * @return int
 */
int Instrument::getVolume() {
	return m_ticks.size();
}

/**
 * Gets the symbol for this instrument.
 * 
 * @return std::string
 */
std::string Instrument::getSymbol() const {
	return m_symbol;
}

/**
 * Rounds the given value to the minimum tick size.
 * Returns -1 on error
 * 
 * @param const double value
 * @return double 
 */
double Instrument::round(const double value) {
	if ( m_point_size == 0 ) return -1;
	int ticksize = 1 / m_point_size;
	double nearest = round( value * ticksize ) / ticksize;
	return nearest;
}

/**
 * Add exchange tick to instrument list and calculate the high and low
 * 
 * @param ExchangeTick tick  exchange market snapshot
 */
void Instrument::addTick(const ExchangeTick tick) {
	m_ticks.push_back( tick );

	// set bid and ask price
	m_bid_price = tick.bid;
	m_ask_price = tick.ask;
	// set high and low
	if ( m_ask_price > m_high_price ) m_high_price = m_ask_price;
	if ( m_bid_price < m_low_price ) m_low_price = m_bid_price;
}

};