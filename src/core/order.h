#pragma once

#include <string>

#include "core/enums.h"
#include "core/utility.h"

namespace idefix {
class Order {
public:
	Order();

	// Gets the unique identifier for this order from exchange side.
	std::string getOrderID() const;
	// Gets the unique identifier for this order from client side
	std::string getClientOrderID() const;
	// Determines if this is a buy order.
	bool isBuy() const;
	// Determines if this is a sell order.
	bool isSell() const; 
	// Determines if this order is filled.
	bool isFilled() const;
	// Determines if this order is cancelled.
	bool isCancelled() const;
	// Gets the type of order (Marker, Limit, Stop, Trail).
	enums::OrderType getType() const;
	// Gets the Time In Force for this order.
	enums::TIF getTIF() const;
	// Gets the action of this order (Buy or Sell)
	enums::OrderAction getAction() const;
	// Gets the status of this order (NEW, FILLED, REMOVED, STOPPED, UPDATE)
	enums::OrderStatus getStatus() const;
	// Gets the reference ID for the order.
	std::string getReferenceID() const;
	// Gets the quantity (size) of this order.
	int getQuantity() const;
	// Gets the stop price for the order (null if not a stop order).
	double getStopPrice() const;
	// Gets the limit price for the order (null if not a limit order).
	double getLimitPrice() const;
	// Gets the time (in milliseconds since 1970) of the last fill on this order.
	long getLastFillTime() const;
	// Gets the last fill price for this order.
	double getLastFillPrice() const;
	// Gets the number of shares/contracts that have been filled.
	int getFilled() const;
	// Gets the account ID for this order.
	std::string getAccountID() const;
	// Gets the symbol
	std::string getSymbol() const;
	// Gets the profit or loss associated with this trade.
	double getPnL() const;
	// Gets the entry price.
	double getEntryPrice() const;
	// Gets the status message if available
	std::string getStatusMsg() const;
	// Gets custom key=value 
	std::string getCustomField(const std::string& key);
	// Gets the open time of a position/trade
	std::string getOpenTime();
	// Gets the close time of a position/trade
	std::string getCloseTime();
	// Gets the close order id of a trade
	std::string getCloseOrderID() const;

	void setType(const enums::OrderType type);
	void setAction(const enums::OrderAction action);
	void setTIF(const enums::TIF tif);
	void setStatus(const enums::OrderStatus status);
	void setReferenceID(const std::string refid);
	void setQuantity(const int qty);
	void setOrderID(const std::string orderid);
	void setClientOrderID(const std::string orderid);
	void setStopPrice(const double price);
	void setLimitPrice(const double price);
	void setEntryPrice(const double price);
	void setLastFillTime(const long time);
	void setLastFillPrice(const double price);
	void setFilled(const int contracts);
	void setAccountID(const std::string accountid);
	void setSymbol(const std::string symbol);
	void setStatusMsg(const std::string msg);
	void setPnL(const double value);
	void setOpenTime(const std::string time);
	void setCloseTime(const std::string time);
	void setCloseOrderID(const std::string orderid);
	void setCustomField(const std::string key, const std::string value);

private:
	// the unique identifier for this order from exchange side.
	std::string m_order_id;
	// the unique identifier for this order from client side.
	std::string m_client_order_id;
	// the symbol for this order.
	std::string m_symbol;
	// the reference ID for the order.
	std::string m_ref_id;
	// the account ID for this order.
	std::string m_account_id;
	// status message
	std::string m_status_msg;
	// the time (in milliseconds since 1970) of the last fill on this order.
	long m_last_filltime;
	// the action of this order (Buy or Sell)
	enums::OrderAction m_action;
	// the type of order (Market, Limit, Stop, Trail).
	enums::OrderType m_order_type;
	// the status of order (New, Filled, Removed, Stopped, Update)
	enums::OrderStatus m_status;
	// the Time In Force for this order.
	enums::TIF m_tif;
	// the order status
	enums::OrderStatus m_order_status;
	// the quantity (size) of this order.
	int m_quantity;
	// the last fill price for this order.
	double m_last_fillprice;
	// the number of shares/contracts that have been filled.
	int m_filled;
	// Determines if this order is cancelled.
	bool m_is_cancelled;
	// the limit price for the order (null if not a limit order).
	double m_limit_price;
	// the stop price for the order (null if not a stop order).
	double m_stop_price;
	// the price on first execution
	double m_entry_price;
	// holds custom fields
	UnorderedStrMap m_custom_fields;
	// the close order id of a trade
	std::string m_close_order_id;
	// the profit/loss in pips
	double m_pnl_pips;
	// the close time of a trade
	std::string m_close_time;
	// the open time of a position/trade
	std::string m_open_time;

};
};