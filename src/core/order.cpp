#include "order.h"

#include <utility>

namespace idefix {
Order::Order() {}

// Determines if this is a buy order.
bool Order::isBuy() const {
	return m_action == enums::OrderAction::BUY;
}

// Determines if this is a sell order.
bool Order::isSell() const {
	return m_action == enums::OrderAction::SELL;
}

// Determines if this order is filled.
bool Order::isFilled() const {
	return m_filled > 0;
}

// Determines if this order is cancelled.
bool Order::isCancelled() const {
	return m_is_cancelled;
}

// Gets the type of order (Marker, Limit, Stop, Trail).
enums::OrderType Order::getType() const {
	return m_order_type;
}

// Gets the Time In Force for this order.
enums::TIF Order::getTIF() const {
	return m_tif;
}

// Gets the action of this order (Buy or Sell)
enums::OrderAction Order::getAction() const {
	return m_action;
}

// Gets the status of this order (NEW, FILLED, REMOVED, STOPPED, UPDATE)
enums::OrderStatus Order::getStatus() const {
	return m_order_status;
}

// Gets the reference ID for the order.
std::string Order::getReferenceID() const {
	return m_ref_id;
}

// Gets the quantity (size) of this order.
int Order::getQuantity() const {
	return m_quantity;
}

// Gets the unique identifier for this order from exchange side..
std::string Order::getOrderID() const {
	return m_order_id;
}

// Gets the unique identifier for this order from client side.
std::string Order::getClientOrderID() const {
	return m_client_order_id;
}

// Gets the stop price for the order (null if not a stop order).
double Order::getStopPrice() const {
	return m_stop_price;
}

// Gets the limit price for the order (null if not a limit order).
double Order::getLimitPrice() const {
	return m_limit_price;
}

// Gets the time (in milliseconds since 1970) of the last fill on this order.
long Order::getLastFillTime() const {
	return m_last_filltime;
}

// Gets the last fill price for this order.
double Order::getLastFillPrice() const {
	return m_last_fillprice;
}

// Gets the number of shares/contracts that have been filled.
int Order::getFilled() const {
	return m_filled;
}

// Gets the account ID for this order.
std::string Order::getAccountID() const {
	return m_account_id;
}

// Gets the symbol
std::string Order::getSymbol() const {
	return m_symbol;
}

// Gets the status message if available
std::string Order::getStatusMsg() const {
	return m_status_msg;
}

// Gets custom value of custom field
std::string Order::getCustomField(const std::string &key) {
	return FindInMap( m_custom_fields, key );
}

// Gets the profit or loss associated with this trade.
double Order::getPnL() const {
	return m_pnl_pips;
}

// Gets the entry price.
double Order::getEntryPrice() const {
	return m_entry_price;
}

// Gets the open time of a position/trade
std::string Order::getOpenTime() {
	return m_open_time;
}

// Gets the close time of a position/trade
std::string Order::getCloseTime() {
	return m_close_time;
}

// Gets the close order id of a trade
std::string Order::getCloseOrderID() const {
	return m_close_order_id;
}


void Order::setType(const enums::OrderType type) {
	m_order_type = type;
}

void Order::setAction(const enums::OrderAction action) {
	m_action = action;
}

void Order::setTIF(const enums::TIF tif) {
	m_tif = tif;
}

void Order::setStatus(const enums::OrderStatus status) {
	m_order_status = status;
}

void Order::setReferenceID(const std::string refid) {
	m_ref_id = refid;
}

void Order::setQuantity(const int qty) {
	m_quantity = qty;
}

void Order::setOrderID(const std::string orderid) {
	m_order_id = orderid;
}

void Order::setClientOrderID(const std::string orderid) {
	m_client_order_id = orderid;
}

void Order::setStopPrice(const double price) {
	m_stop_price = price;
}

void Order::setLimitPrice(const double price) {
	m_limit_price = price;
}

void Order::setLastFillTime(const long time) {
	m_last_filltime = time;
}

void Order::setLastFillPrice(const double price) {
	m_last_fillprice = price;
}

void Order::setFilled(const int contracts) {
	m_filled = contracts;
}

void Order::setAccountID(const std::string accountid) {
	m_account_id = accountid;
}

void Order::setSymbol(const std::string symbol) {
	m_symbol = symbol;
}

void Order::setEntryPrice(const double price) {
	m_entry_price = price;
}

void Order::setStatusMsg(const std::string msg) {
	m_status_msg = msg;
}

void Order::setPnL(const double value) {
	m_pnl_pips = value;
}

void Order::setOpenTime(const std::string time) {
	m_open_time = time;
}

void Order::setCloseTime(const std::string time) {
	m_close_time = time;
}

void Order::setCloseOrderID(const std::string orderid) {
	m_close_order_id = orderid;
}


void Order::setCustomField(const std::string key, const std::string value) {
	auto it = m_custom_fields.find( key );
	if ( it != m_custom_fields.end() ) {
		// update
		it->second = value;
	} else {
		// insert
		m_custom_fields.emplace( key, value );
	}
}

};