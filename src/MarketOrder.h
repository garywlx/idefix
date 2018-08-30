#ifndef IDEFIX_MARKETORDER_H
#define IDEFIX_MARKETORDER_H

#include <iostream>
#include <sstream>
#include <string>
#include <iomanip>
#include <quickfix/FixValues.h>

using namespace std;

namespace IDEFIX {
class MarketOrder {
private:
	string m_clOrd_ID;
	string m_pos_ID; // FXCM_POS_ID
	string m_order_ID;
	string m_account_ID;
	string m_symbol;
	string m_sending_time;
	string m_close_time; // the timestamp on which the order was closed
	char m_side; // 1 = Buy, 2 = Sell (FixValues.h)
	double m_qty;
	double m_price;
	double m_stop_price;
	double m_take_price;
	double m_close_price; // the price on which the order was closed
	double m_profit_loss_value; // negativ = loss
	int m_precision;
	double m_point_size;

public:
	enum Status {
		NEW, FILLED, REMOVED, STOPPED, UPDATE
	};

	// Construct empty market order
	explicit MarketOrder() {}
	// Construct market order with default values except symbol
	explicit MarketOrder(const string symbol): m_clOrd_ID("0"), m_pos_ID(""), m_order_ID(""), m_account_ID("0"), 
	m_symbol(symbol), m_qty(0), m_side('1'), m_price(0), m_stop_price(0), m_take_price(0), m_close_price(0),
	m_profit_loss_value(0), m_sending_time(""), m_close_time(""), m_precision(5), m_point_size(0) {}

	inline ~MarketOrder(){}

	inline string getClOrdID() const { 
		return m_clOrd_ID; 
	}
	inline void setClOrdID(const string clOrdID){ 
		if( m_clOrd_ID != clOrdID ){
			m_clOrd_ID = clOrdID;
		}
	}
	inline string getPosID() const {
		return m_pos_ID; 
	}
	inline void setPosID(const string pos_ID){
		if( m_pos_ID != pos_ID ){
			m_pos_ID = pos_ID;
		}
	}
	inline string getOrderID() const {
		return m_order_ID;
	}
	inline void setOrderID(const string orderID){
		if ( m_order_ID != orderID ){
			m_order_ID = orderID;
		}
	}
	inline string getAccountID() const {
		return m_account_ID; 
	}
	inline void setAccountID(const string accountID){
		if( m_account_ID != accountID ){
			m_account_ID = accountID;
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
		if( getSide() == FIX::Side_BUY ){
			return "Side_BUY";
		} else if( getSide() == FIX::Side_SELL ){
			return "Side_SELL";
		} else {
			ostringstream out;
			out << getSide();
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
		return m_stop_price; 
	}
	inline void setStopPrice(const double price){
		if( m_stop_price != price ){
			m_stop_price = price;
		}
	}
	inline double getTakePrice() const {
		return m_take_price; 
	}
	inline void setTakePrice(const double price){
		if( m_take_price != price ){
			m_take_price = price;
		}
	}
	inline double getClosePrice() const {
		return m_close_price;
	}
	inline void setClosePrice(const double price) {
		if ( m_close_price != price ) {
			m_close_price = price;
		}
	}
	inline double getProfitLoss() const {
		return m_profit_loss_value; 
	}
	inline void setProfitLoss(const double value){
		if( m_profit_loss_value != value ){
			m_profit_loss_value = value;
		}
	}
	inline bool isValid() const {
		return ! m_clOrd_ID.empty();
	}
	inline string getSendingTime() const {
		return m_sending_time;
	}
	inline void setSendingTime(const string sending_time){
		if( m_sending_time != sending_time ){
			m_sending_time = sending_time;
		}
	}
	inline string getCloseTime() const {
		return m_close_time;
	}
	inline void setCloseTime(const string close_time) {
		if ( m_close_time != close_time ) {
			m_close_time = close_time;
		}
	}
	inline int getPrecision() const { return m_precision; }
	inline void setPrecision(const int precision) {
		if ( m_precision != precision ) {
			m_precision = precision;
		}
	}

	inline double getPointSize() const { return m_point_size; }
	inline void setPointSize(const double pointsize) {
		if ( m_point_size != pointsize ) {
			m_point_size = pointsize;
		}
	}

	inline std::string toString() const {
		std::ostringstream out;
		out << "MarketOrder {" << endl
			<< "  pos_ID      " << getPosID() << endl
			<< "  account     " << getAccountID() << endl
			<< "  clOrd_ID    " << getClOrdID() << endl
			<< "  order_ID    " << getOrderID() << endl
			<< "  symbol      " << getSymbol() << endl
			<< "  side        " << getSide() << ": " << getSideStr() << endl
			<< "  qty         " << setprecision( 2 ) << fixed << getQty() << endl
			<< "  price       " << setprecision( getPrecision() ) << fixed << getPrice() << endl
			<< "  stopPrice   " << setprecision( getPrecision() ) << fixed << getStopPrice() << endl
			<< "  takePrice   " << setprecision( getPrecision() ) << fixed << getTakePrice() << endl
			<< "  P&L         " << setprecision( 2 ) << fixed << getProfitLoss() << endl
			<< "  sendingtime " << getSendingTime() << endl
			<< "}" << endl;

		return out.str();
	}
};

// Operator Magic
inline std::ostream& operator<<(std::ostream& out, const IDEFIX::MarketOrder& mo){
	out << mo.toString();
	return out;
}

};

#endif