#ifndef IDEFIX_MARKETSNAPSHOT_H
#define IDEFIX_MARKETSNAPSHOT_H

#include <string>
#include <ostream>
#include <iomanip>
#include <cmath>
#include "String.h"

using namespace std;

namespace IDEFIX {
class MarketSnapshot {
private:
	string m_symbol;
	string m_base_currency;
	string m_quote_currency;
	double m_bid;
	double m_ask;
	double m_session_high;
	double m_session_low;
	double m_spread;
	unsigned int m_precision;
	double m_point_size;
	double m_contract_size;
	string m_sending_time;

public:
	explicit MarketSnapshot(): m_symbol(""), m_bid(0), m_ask(0), m_session_high(0), m_session_low(0), m_spread(0), m_precision(5), m_point_size(0.0001), m_contract_size(100000), m_sending_time("") {}
	explicit MarketSnapshot(const string symbol, const double bid, const double ask, const double spread, const double high, const double low, const string sending_time)
	: m_symbol(symbol), m_bid(bid), m_ask(ask), m_spread(spread), m_session_high(high), m_session_low(low), m_sending_time(sending_time), m_precision(5), m_point_size(0.0001), m_contract_size(100000) {
		setBaseAndQuote( symbol );
	}
	
	inline ~MarketSnapshot() {}
	inline string getSymbol() const {
		return m_symbol;
	}
	inline void setSymbol(const string symbol) {
		if( m_symbol != symbol ) {
			m_symbol = symbol;
			setBaseAndQuote( symbol );
		}
	}
	inline void setBaseAndQuote(const std::string symbol) {
		// set base and quote currency
		auto parts = str::explode( symbol, '/' );
		m_base_currency  = parts[0];
		m_quote_currency = parts[1];
	}
	inline double getBid() const {
		return m_bid;
	}
	inline void setBid(const double price){
		if( m_bid != price ){
			m_bid = price;
		}
		updateSpread();
	}
	inline double getAsk() const {
		return m_ask;
	}
	inline void setAsk(const double price){
		if( m_ask != price ){
			m_ask = price;
		}
		updateSpread();
	}
	inline double getSpread() const {
		return m_spread;
	}
	inline void setSpread(const double spread){
		if( m_spread != spread ){
			m_spread = spread;
		}
	}
	inline double getSessionHigh() const {
		return m_session_high;
	}
	inline void setSessionHigh(const double high){
		if( m_session_high != high ){
			m_session_high = high;
		}
	}
	inline double getSessionLow() const {
		return m_session_low;
	}
	inline void setSessionLow(const double low){
		if( m_session_low != low ){
			m_session_low = low;
		}
	}
	inline unsigned int getPrecision() const {
		return m_precision;
	}
	inline void setPrecision(const unsigned int precision){
		if( m_precision != precision ){
			m_precision = precision;
		}
	}
	inline string getSendingTime() const {
		return m_sending_time;
	}
	inline void setSendingTime(const string sending_time){
		if( m_sending_time != sending_time ){
			m_sending_time = sending_time;
		}
	}
	inline bool isValid() const {
		return ( ! getSymbol().empty() && getBid() > 0 && getAsk() > 0 && getSpread() >= 0 );
	}
	// Calculates spread
	// http://www.youngbrokers.net/2013/bid-ask-spread/
	inline void updateSpread() { 
		if( getAsk() > 0 && getBid() > 0 && getPrecision() > 0 ){
			auto spread = std::abs( getAsk() - getBid() );
			setSpread( spread * ( 1 / getPointSize() ) );
		}
	}
	/*!
	 * Get quote currency of forex pair
	 * GBP/USD, USD is quote
	 * 
	 * @return std::string
	 */
	inline std::string getQuoteCurrency() const {
		return m_quote_currency;
	}
	/*!
	 * Get base currency of forex pair
	 * GBP/USD, GBP is base
	 * 
	 * @return std::string
	 */
	inline std::string getBaseCurrency() const {
		return m_base_currency;
	}

	inline double getPointSize() const {
		return m_point_size;
	}
	inline void setPointSize(const double point_size) {
		if ( m_point_size != point_size ) {
			m_point_size = point_size;
		}
	}
	inline double getContractSize() const {
		return m_contract_size;
	}
	inline void setContractSize(const double contract_size) {
		if ( m_contract_size != contract_size ) {
			m_contract_size = contract_size;
		}
	}
	inline std::string toString() const {
		std::ostringstream oss;
		oss << getSymbol() << std::setprecision(getPrecision())
		<< " bid: " << std::fixed << getBid() 
		<< " ask: " << std::fixed << getAsk() 
		<< " spread: " << std::fixed << std::setprecision(2) << getSpread() 
		<< " shigh: " << std::fixed << std::setprecision( getPrecision() ) << getSessionHigh()
		<< " slow: " << std::fixed << getSessionLow()
		<< " ts: " << getSendingTime();

		return oss.str();
	}
};

// Operator Magic
inline std::ostream& operator<<(std::ostream& out, const IDEFIX::MarketSnapshot& ms){
	out << ms.toString();
	return out;
}
};

#endif 