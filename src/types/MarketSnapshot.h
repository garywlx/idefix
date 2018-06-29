#ifndef IDEFIX_MARKETSNAPSHOT_H
#define IDEFIX_MARKETSNAPSHOT_H

#include <string>
#include <ostream>
#include <iomanip>
#include "../TradeMath.h"
#include <quickfix/Mutex.h>

using namespace std;

namespace IDEFIX {
class MarketSnapshot {
private:
	string m_symbol;
	double m_bid;
	double m_ask;
	double m_session_high;
	double m_session_low;
	double m_spread;
	unsigned int m_precision;
	string m_sending_time;
	mutable FIX::Mutex m_mutex;

public:
	explicit MarketSnapshot(): m_symbol(""), m_bid(0), m_ask(0), m_session_high(0), m_session_low(0), m_spread(0), m_precision(5), m_sending_time("") {}
	explicit MarketSnapshot(const string symbol, const double bid, const double ask, const double spread, const double high, const double low, const string sending_time)
	: m_symbol(symbol), m_bid(bid), m_ask(ask), m_spread(spread), m_session_high(high), m_session_low(low), m_sending_time(sending_time), m_precision(5) {}
	
	inline ~MarketSnapshot() {}
	inline string getSymbol() const {
		FIX::Locker lock(m_mutex);
		return m_symbol;
	}
	inline void setSymbol(const string symbol) {
		FIX::Locker lock(m_mutex);
		if( m_symbol != symbol ) {
			m_symbol = symbol;
		}
	}
	inline double getBid() const {
		FIX::Locker lock(m_mutex);
		return m_bid;
	}
	inline void setBid(const double price){
		FIX::Locker lock(m_mutex);
		if( m_bid != price ){
			m_bid = price;
		}
		updateSpread();
	}
	inline double getAsk() const {
		FIX::Locker lock(m_mutex);
		return m_ask;
	}
	inline void setAsk(const double price){
		FIX::Locker lock(m_mutex);
		if( m_ask != price ){
			m_ask = price;
		}
		updateSpread();
	}
	inline double getSpread() const {
		FIX::Locker lock(m_mutex);
		return m_spread;
	}
	inline void setSpread(const double spread){
		FIX::Locker lock(m_mutex);
		if( m_spread != spread ){
			m_spread = spread;
		}
	}
	inline double getSessionHigh() const {
		FIX::Locker lock(m_mutex);
		return m_session_high;
	}
	inline void setSessionHigh(const double high){
		FIX::Locker lock(m_mutex);
		if( m_session_high != high ){
			m_session_high = high;
		}
	}
	inline double getSessionLow() const {
		FIX::Locker lock(m_mutex);
		return m_session_low;
	}
	inline void setSessionLow(const double low){
		FIX::Locker lock(m_mutex);
		if( m_session_low != low ){
			m_session_low = low;
		}
	}
	inline unsigned int getPrecision() const {
		FIX::Locker lock(m_mutex);
		return m_precision;
	}
	inline void setPrecision(const unsigned int precision){
		FIX::Locker lock(m_mutex);
		if( m_precision != precision ){
			m_precision = precision;
		}
	}
	inline string getSendingTime() const {
		FIX::Locker lock(m_mutex);
		return m_sending_time;
	}
	inline void setSendingTime(const string sending_time){
		FIX::Locker lock(m_mutex);
		if( m_sending_time != sending_time ){
			m_sending_time = sending_time;
		}
	}
	inline bool isValid() const {
		FIX::Locker lock(m_mutex);
		return ( ! getSymbol().empty() && getBid() > 0 && getAsk() > 0 && getSpread() >= 0 );
	}
	// Calculates spread
	// http://www.youngbrokers.net/2013/bid-ask-spread/
	inline void updateSpread() {
		FIX::Locker lock(m_mutex);
		if( getAsk() > 0 && getBid() > 0 && getPrecision() > 0 ){
			setSpread((std::abs(getAsk() - getBid()) * TradeMath::decimalMultiplicator(getPrecision())));
		}
	}
};

// Operator Magic
inline std::ostream& operator<<(std::ostream& out, const IDEFIX::MarketSnapshot& ms){
	out << ms.getSymbol() << std::setprecision(5)
		<< " bid: " << std::fixed << ms.getBid() 
		<< " ask: " << std::fixed << ms.getAsk() 
		<< " spread: " << std::fixed << std::setprecision(2) << ms.getSpread() 
		<< " shigh: " << std::fixed << std::setprecision(5) << ms.getSessionHigh()
		<< " slow: " << std::fixed << ms.getSessionLow()
		<< " ts: " << ms.getSendingTime();
	return out;
}
};

#endif 