#include "order.h"
#include <sstream>

namespace idefix {
	Order::Order() : m_symbol(""), m_clOrd_ID("0"), m_pos_ID(""), m_order_ID(""), m_account_ID("0"), 
	m_qty(0), m_side('1'), m_price(0), m_stop_price(0), m_take_price(0), m_close_price(0),
	m_profit_loss_value(0), m_sending_time(""), m_close_time(""), m_precision(5), m_point_size(0) {}

std::string Order::getClOrdID() const { 
	return m_clOrd_ID; 
}
void Order::setClOrdID(const std::string clOrdID){ 
	if( m_clOrd_ID != clOrdID ){
		m_clOrd_ID = clOrdID;
	}
}
std::string Order::getPosID() const {
	return m_pos_ID; 
}
void Order::setPosID(const std::string pos_ID){
	if( m_pos_ID != pos_ID ){
		m_pos_ID = pos_ID;
	}
}
std::string Order::getOrderID() const {
	return m_order_ID;
}
void Order::setOrderID(const std::string orderID){
	if ( m_order_ID != orderID ){
		m_order_ID = orderID;
	}
}
std::string Order::getAccountID() const {
	return m_account_ID; 
}
void Order::setAccountID(const std::string accountID){
	if( m_account_ID != accountID ){
		m_account_ID = accountID;
	}
}
std::string Order::getSymbol() const {
	return m_symbol; 
}
void Order::setSymbol(const std::string symbol){
	if( m_symbol != symbol ){
		m_symbol = symbol;
	}
}
double Order::getQty() const {
	return m_qty; 
}
void Order::setQty(const double qty){
	if( m_qty != qty ){
		m_qty = qty;
	}
}
char Order::getSide() const {
	return m_side; 
}
std::string Order::getSideStr() const {
	if( getSide() == FIX::Side_BUY ){
		return "Side_BUY";
	} else if( getSide() == FIX::Side_SELL ){
		return "Side_SELL";
	} else {
		std::ostringstream out;
		out << getSide();
		return out.str();
	}
}
/*!
 * Get the opposide of Side. e.g. Side = BUY return SELL
 * Returns getSide value if opposide can not be find
 * @return char
 */
char Order::getOpposide() const {
	if( getSide() == FIX::Side_BUY ) {
		return FIX::Side_SELL;
	} else if( getSide() == FIX::Side_SELL ){
		return FIX::Side_BUY;
	} else {
		return getSide();
	}
}
void Order::setSide(const char side){
	if( m_side != side ){
		m_side = side;
	}
}
double Order::getPrice() const {
	return m_price; 
}
void Order::setPrice(const double price){
	if( m_price != price ){
		m_price = price;
	}
}
double Order::getStopPrice() const {
	return m_stop_price; 
}
void Order::setStopPrice(const double price){
	if( m_stop_price != price ){
		m_stop_price = price;
	}
}
double Order::getTakePrice() const {
	return m_take_price; 
}
void Order::setTakePrice(const double price){
	if( m_take_price != price ){
		m_take_price = price;
	}
}
double Order::getClosePrice() const {
	return m_close_price;
}
void Order::setClosePrice(const double price) {
	if ( m_close_price != price ) {
		m_close_price = price;
	}
}
double Order::getProfitLoss() const {
	return m_profit_loss_value; 
}
void Order::setProfitLoss(const double value){
	if( m_profit_loss_value != value ){
		m_profit_loss_value = value;
	}
}
bool Order::isValid() const {
	return ! m_clOrd_ID.empty();
}
std::string Order::getSendingTime() const {
	return m_sending_time;
}
void Order::setSendingTime(const std::string sending_time){
	if( m_sending_time != sending_time ){
		m_sending_time = sending_time;
	}
}
std::string Order::getCloseTime() const {
	return m_close_time;
}
void Order::setCloseTime(const std::string close_time) {
	if ( m_close_time != close_time ) {
		m_close_time = close_time;
	}
}
int Order::getPrecision() const { 
	if ( m_precision < 0 ) {
		return 0;
	}
	return m_precision; 
}
void Order::setPrecision(const int precision) {
	if ( m_precision != precision ) {
		m_precision = precision;
	}
}

double Order::getPointSize() const { 
	if ( m_point_size < 0 ) {
		return 0;
	}
	return m_point_size; 
}
void Order::setPointSize(const double pointsize) {
	if ( m_point_size != pointsize ) {
		m_point_size = pointsize;
	}
}

std::string Order::toString() const {
	std::ostringstream out;
	out << "Order {" << std::endl
		<< "  pos_ID      " << getPosID() << std::endl
		<< "  account     " << getAccountID() << std::endl
		<< "  clOrd_ID    " << getClOrdID() << std::endl
		<< "  order_ID    " << getOrderID() << std::endl
		<< "  symbol      " << getSymbol() << std::endl
		<< "  side        " << getSide() << ": " << getSideStr() << std::endl
		<< "  qty         " << std::setprecision( 2 ) << std::fixed << getQty() << std::endl
		<< "  price       " << std::setprecision( 5 ) << std::fixed << getPrice() << std::endl
		<< "  stopPrice   " << std::setprecision( 5 ) << std::fixed << getStopPrice() << std::endl
		<< "  takePrice   " << std::setprecision( 5 ) << std::fixed << getTakePrice() << std::endl
		<< "  P&L         " << std::setprecision( 2 ) << std::fixed << getProfitLoss() << std::endl
		<< "  sendingtime " << getSendingTime() << std::endl
		<< "}" << std::endl;

	return out.str();
}
};