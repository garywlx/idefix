#ifndef IDEFIX_MARKETORDER_H
#define IDEFIX_MARKETORDER_H

#include <quickfix/Application.h>

using namespace FIX;
using namespace std;

namespace IDEFIX {
class MarketOrder {
private:
	ClOrdID m_clOrdID;
	string m_posID; // FXCM_POS_ID
	Account m_accountID;
	Symbol m_symbol;
	OrderQty m_qty;
	Side m_side;
	Price m_price;
	StopPx m_stopPrice;
	Price m_takePrice;
	double m_profitLossValue; // negativ = loss

public:
	// Construct empty market order
	explicit MarketOrder() {}
	// Construct market order with default values except symbol
	explicit MarketOrder(const Symbol symbol): m_clOrdID(ClOrdID("0")), m_posID(""), m_accountID(Account("0")), 
	m_symbol(symbol), m_qty(OrderQty(0)), m_side(FIX::Side_BUY), m_price(Price(0)), m_stopPrice(StopPx(0)), m_takePrice(Price(0)),
	m_profitLossValue(0) {}

	inline ~MarketOrder(){}

	inline ClOrdID getClOrdID() const { 
		return m_clOrdID; 
	}
	inline void setClOrdID(const ClOrdID clOrdID){ 
		m_clOrdID = m_clOrdID != clOrdID ? clOrdID : m_clOrdID;
	}
	inline string getPosID() const {
		return m_posID; 
	}
	inline void setPosID(const string posID){
		m_posID = m_posID != posID ? posID : m_posID;
	}
	inline Account getAccountID() const {
		return m_accountID; 
	}
	inline void setAccountID(const Account accountID){
		m_accountID = m_accountID != accountID ? accountID : m_accountID;
	}
	inline Symbol getSymbol() const {
		return m_symbol; 
	}
	inline void setSymbol(const Symbol symbol){
		m_symbol = m_symbol != symbol ? symbol : m_symbol;
	}
	inline OrderQty getQty() const {
		return m_qty; 
	}
	inline void setQty(const OrderQty qty){
		m_qty = m_qty != qty ? qty : m_qty;
	}
	inline Side getSide() const {
		return m_side; 
	}
	inline void setSide(const Side side){
		m_side = m_side != side ? side : m_side;
	}
	inline Price getPrice() const {
		return m_price; 
	}
	inline void setPrice(const Price price){
		m_price = m_price != price ? price : m_price;
	}
	inline StopPx getStopPrice() const {
		return m_stopPrice; 
	}
	inline void setStopPrice(const StopPx price){
		m_stopPrice = m_stopPrice != price ? price : m_stopPrice;
	}
	inline Price getTakePrice() const {
		return m_takePrice; 
	}
	inline void setTakePrice(const Price price){
		m_takePrice = m_takePrice != price ? price : m_takePrice;
	}
	inline double getProfitLoss() const {
		return m_profitLossValue; 
	}
	inline void setProfitLoss(const double value){
		m_profitLossValue = m_profitLossValue != value ? value : m_profitLossValue;
	}
	inline bool isValid() const {
		return ! m_clOrdID.getValue().empty();
	}
};
};

// Operator Magic
inline std::ostream& operator<<(std::ostream& out, const IDEFIX::MarketOrder& mo){
	out << "[MarketOrder] ClOrdID " << mo.getClOrdID()
		<< " posID " << mo.getPosID()
		<< " symbol " << mo.getSymbol()
		<< " qty " << mo.getQty() 
		<< " side " << (mo.getSide() == FIX::Side_BUY ? "Side_BUY" : "Side_SELL")
		<< " price " << mo.getPrice() 
		<< " stopPrice " << mo.getStopPrice()
		<< " takePrice " << mo.getTakePrice()
		<< " account " << mo.getAccountID()
		<< " plValue " << mo.getProfitLoss();

	return out;
}

#endif