#pragma once

/// Represents an order to buy or sell an instrument.

#include <iostream>
#include <sstream>
#include <string>
#include <iomanip>
#include <quickfix/FixValues.h>

namespace idefix {
class Order {
private:
	std::string m_clOrd_ID;
	std::string m_pos_ID; // FXCM_POS_ID
	std::string m_order_ID;
	std::string m_account_ID;
	std::string m_symbol;
	std::string m_sending_time;
	std::string m_close_time; // the timestamp on which the order was closed
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
	// Construct empty market order
	explicit Order();

	std::string getClOrdID() const;
	void setClOrdID(const std::string clOrdID);
	std::string getPosID() const;
	void setPosID(const std::string pos_ID);
	std::string getOrderID() const;
	void setOrderID(const std::string orderID);
	std::string getAccountID() const;
	void setAccountID(const std::string accountID);
	std::string getSymbol() const;
	void setSymbol(const std::string symbol);
	double getQty() const;
	void setQty(const double qty);
	char getSide() const;
	std::string getSideStr() const;
	/*!
	 * Get the opposide of Side. e.g. Side = BUY return SELL
	 * Returns getSide value if opposide can not be find
	 * @return char
	 */
	char getOpposide() const;
	void setSide(const char side);
	double getPrice() const;
	void setPrice(const double price);
	double getStopPrice() const;
	void setStopPrice(const double price);
	double getTakePrice() const;
	void setTakePrice(const double price);
	double getClosePrice() const;
	void setClosePrice(const double price);
	double getProfitLoss() const;
	void setProfitLoss(const double value);
	bool isValid() const;
	std::string getSendingTime() const;
	void setSendingTime(const std::string sending_time);
	std::string getCloseTime() const;
	void setCloseTime(const std::string close_time);
	int getPrecision() const;
	void setPrecision(const int precision);
	double getPointSize() const;
	void setPointSize(const double pointsize);
	std::string toString() const;
};

// Operator Magic
inline std::ostream& operator<<(std::ostream& out, const idefix::Order& mo);

};
