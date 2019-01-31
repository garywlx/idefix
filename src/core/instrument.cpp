#include "instrument.h"
#include <cmath>
#include <sstream>
#include <iomanip>

namespace idefix {
Instrument::Instrument(): Instrument("", 0, 0, 0) {}
Instrument::Instrument(const std::string& symbol): Instrument( symbol, 0, 0, 0 ) {}
Instrument::Instrument(const std::string& symbol, const double ask, const double bid, const double point): 
	m_symbol(symbol), m_ask_price(ask), m_bid_price(bid), m_sym_point_size(point), m_high_price(ask), m_low_price(bid) {}

/**
 * Formats the given value as a string.
 * 
 * @param const double value
 * @return std::string
 */
std::string Instrument::format(const double value) {
	std::ostringstream ostr;
	ostr << std::setprecision( getPrecision() ) << std::fixed << value;
	return ostr.str();
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
double Instrument::getPointSize() const {
	return m_sym_point_size;
}

/**
 * Gets the difference between the bid and ask price for this instrument in pips.
 * 
 * @return double
 */
double Instrument::getSpread() {
	return abs( m_ask_price - m_bid_price );
}

/**
 * Gets the difference between the bid and ask price in points.
 * 
 * @return double
 */
double Instrument::getSpreadPoints() {
	if ( getPointSize() == 0 ) return -1;
	return getSpread() * ( 1 / getPointSize() );
}

double Instrument::getFactor() const {
	return m_factor;
}

double Instrument::getContractMultiplier() const {
	return m_contract_multiplier;
}

double Instrument::getRoundLot() const {
	return m_round_lot;
}

double Instrument::getInterestBuy() const {
	return m_sym_interest_buy;
}

double Instrument::getInterestSell() const {
	return m_sym_interest_sell;
}

double Instrument::getCondDistStop() const {
	return m_sym_cond_dist_stop;
}

double Instrument::getCondDistLimit() const {
	return m_sym_cond_dist_limit;
}

double Instrument::getCondDistEntryStop() const {
	return m_sym_cond_dist_entry_stop;
}

double Instrument::getCondDistEntryLimit() const {
	return m_sym_cond_dist_entry_limit;
}

double Instrument::getMinQuantity() const {
	return m_min_quantity;
}

double Instrument::getMaxQuantity() const {
	return m_max_quantity;
}

enums::InstrumentType Instrument::getProduct() const {
	switch( m_product ) {
		case 4: return enums::InstrumentType::CURRENCY;
		case 7: return enums::InstrumentType::INDEX;
		case 2: return enums::InstrumentType::COMMODITY;
		default: return enums::InstrumentType::UNKNOWN;
	}
}

int Instrument::getSymID() const {
	return m_sym_id;
}

int Instrument::getPrecision() const {
	return m_sym_precision;
}

int Instrument::getSortOrder() const {
	return m_sym_order;
}

int Instrument::getFieldProductID() const {
	return m_sym_field_product_id;
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
	if ( getPointSize() == 0 ) return -1;
	int ticksize = 1 / getPointSize();
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

void Instrument::setCurrency(const std::string value) {
	m_currency = value;
}

void Instrument::setSubscriptionStatus(const std::string value) {
	m_subscription_status = value;
}

void Instrument::setTradingStatus(const std::string value) {
	m_trading_status = value;
}

void Instrument::setPointSize(const double value) {
	m_sym_point_size = value;
}

void Instrument::setFactor(const double value) {
	m_factor = value;
}

void Instrument::setContractMultiplier(const double value) {
	m_contract_multiplier = value;
}

void Instrument::setRoundLot(const double value) {
	m_round_lot = value;
}

void Instrument::setInterestBuy(const double value) {
	m_sym_interest_buy = value;
}

void Instrument::setInterestSell(const double value) {
	m_sym_interest_sell = value;
}

void Instrument::setCondDistStop(const double value) {
	m_sym_cond_dist_stop = value;
}

void Instrument::setCondDistLimit(const double value) {
	m_sym_cond_dist_limit = value;
}

void Instrument::setCondDistEntryStop(const double value) {
	m_sym_cond_dist_entry_stop = value;
}

void Instrument::setCondDistEntryLimit(const double value) {
	m_sym_cond_dist_entry_limit = value;
}

void Instrument::setMinQuantity(const double value) {
	m_min_quantity = value;
}

void Instrument::setMaxQuantity(const double value) {
	m_max_quantity = value;
}

void Instrument::setProduct(const enums::InstrumentType type) {
	switch( type ) {
		case enums::InstrumentType::CURRENCY: m_product = 4; break;
		case enums::InstrumentType::INDEX: m_product = 7; break;
		case enums::InstrumentType::COMMODITY: m_product = 2; break;
		default: break;
	}
}

void Instrument::setProduct(const int value) {
	m_product = value;
}

void Instrument::setSymID(const int value) {
	m_sym_id = value;
}

void Instrument::setPrecision(const int value) {
	m_sym_precision = value;
}

void Instrument::setSortOrder(const int value) {
	m_sym_order = value;
}

void Instrument::setFieldProductID(const int value) {
	m_sym_field_product_id = value;
}


};