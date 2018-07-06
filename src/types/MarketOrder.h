#ifndef IDEFIX_MARKETORDER_H
#define IDEFIX_MARKETORDER_H

#include <iostream>
#include <sstream>
#include <string>
#include <iomanip>
#include <quickfix/FixValues.h>
#include "../TradeMath.h"

using namespace std;

namespace IDEFIX {
class MarketOrder {
private:
	string m_clOrdID;
	string m_posID; // FXCM_POS_ID
	string m_accountID;
	string m_symbol;
	string m_sending_time;
	char m_side; // 1 = Buy, 2 = Sell (FixValues.h)
	double m_qty;
	double m_price;
	double m_stopPrice;
	double m_takePrice;
	double m_profitLossValue; // negativ = loss

public:
	// Construct empty market order
	explicit MarketOrder() {}
	// Construct market order with default values except symbol
	explicit MarketOrder(const string symbol): m_clOrdID("0"), m_posID(""), m_accountID("0"), 
	m_symbol(symbol), m_qty(0), m_side('1'), m_price(0), m_stopPrice(0), m_takePrice(0),
	m_profitLossValue(0), m_sending_time("") {}

	inline ~MarketOrder(){}

	inline string getClOrdID() const { 
		return m_clOrdID; 
	}
	inline void setClOrdID(const string clOrdID){ 
		if( m_clOrdID != clOrdID ){
			m_clOrdID = clOrdID;
		}
	}
	inline string getPosID() const {
		return m_posID; 
	}
	inline void setPosID(const string posID){
		if( m_posID != posID ){
			m_posID = posID;
		}
	}
	inline string getAccountID() const {
		return m_accountID; 
	}
	inline void setAccountID(const string accountID){
		if( m_accountID != accountID ){
			m_accountID = accountID;
		}
	}
	inline string getSymbol() const {
		return m_symbol; 
	}
	inline void setSymbol(const string symbol){
		if( m_symbol != symbol ){
			m_symbol = symbol;
		}
	}
	inline double getQty() const {
		return m_qty; 
	}
	inline void setQty(const double qty){
		if( m_qty != qty ){
			m_qty = qty;
		}
	}
	inline char getSide() const {
		return m_side; 
	}
	inline string getSideStr() const {
		const char side = getSide();
		if( side == FIX::Side_BUY ){
			return "Side_BUY";
		} else if( side == FIX::Side_SELL ){
			return "Side_SELL";
		} else {
			ostringstream out;
			out << side;
			return out.str();
		}
	}
	/*!
	 * Get the opposide of Side. e.g. Side = BUY return SELL
	 * Returns getSide value if opposide can not be find
	 * @return char
	 */
	inline char getOpposide() const {
		if( getSide() == FIX::Side_BUY ) {
			return FIX::Side_SELL;
		} else if( getSide() == FIX::Side_SELL ){
			return FIX::Side_BUY;
		} else {
			return getSide();
		}
	}
	inline void setSide(const char side){
		if( m_side != side ){
			m_side = side;
		}
	}
	inline double getPrice() const {
		return m_price; 
	}
	inline void setPrice(const double price){
		if( m_price != price ){
			m_price = price;
		}
	}
	inline double getStopPrice() const {
		return m_stopPrice; 
	}
	inline void setStopPrice(const double price){
		if( m_stopPrice != price ){
			m_stopPrice = price;
		}
	}
	inline double getTakePrice() const {
		return m_takePrice; 
	}
	inline void setTakePrice(const double price){
		if( m_takePrice != price ){
			m_takePrice = price;
		}
	}
	inline double getProfitLoss() const {
		return m_profitLossValue; 
	}
	inline void setProfitLoss(const double value){
		if( m_profitLossValue != value ){
			m_profitLossValue = value;
		}
	}
	inline bool isValid() const {
		return ! m_clOrdID.empty();
	}
	inline string getSendingTime() const {
		return m_sending_time;
	}
	inline void setSendingTime(const string sending_time){
		if( m_sending_time != sending_time ){
			m_sending_time = sending_time;
		}
	}
	inline string toString() const {
		ostringstream out;
		out << "MarketOrder {" << endl
			<< setprecision(TradeMath::getPrecision(getSymbol()))
			<< "  posID     " << getPosID() << endl
			<< "  account   " << getAccountID() << endl
			<< "  symbol    " << getSymbol() << endl
			<< "  qty       " << getQty() << endl
			<< "  side      " << getSideStr() << endl
			<< "  price     " << fixed << getPrice() << endl
			<< "  stopPrice " << fixed << getStopPrice() << endl
			<< "  takePrice " << fixed << getTakePrice() << endl
			<< "  plValue   " << fixed << getProfitLoss() << endl
			<< "}" << endl;

		return out.str();
	}
};
};

// Operator Magic
inline std::ostream& operator<<(std::ostream& out, const IDEFIX::MarketOrder& mo){
	return out << mo.toString();
}

#endif