#include "datacenter.h"
#include <date/date.h>

namespace idefix {

/*!
 * Constructor
 */
Datacenter::Datacenter(asset_struct asset): m_asset(asset) {
	// defaults to 60 seconds
	set_lowest_bar_period(60);
}

/*!
 * Deconstructor
 */
Datacenter::~Datacenter() {

}

/*!
 * Set current asset
 * @param const idefix::asset_struct asset
 */
void Datacenter::set(const idefix::asset_struct asset) noexcept {
	m_asset = asset;
}

/*!
 * Set account
 * @param idefix::account_struct account
 */
void Datacenter::set(idefix::account_struct account) noexcept {
	m_account = account;
}

/*!
 * Set lowest bar period
 * @param unsigned int period
 */
void Datacenter::set_lowest_bar_period(const unsigned int period) noexcept {
	m_bar_period_seconds = period;
}

/*!
 * Get current asset
 * @return asset_struct
 */
asset_struct Datacenter::asset() noexcept {
	return m_asset;
}

/*!
 * Get current symbol
 * @return std::string
 */
std::string Datacenter::symbol() noexcept {
	return m_asset.name;
}

/*!
 * Get current balance
 * @return double
 */
double Datacenter::balance() noexcept {
	return m_account.balance;
}

/*!
 * Get current equity
 * @return double
 */
double Datacenter::equity() noexcept {
	return m_account.equity;
}

/*!
 * Get account information
 * @return account_struct
 */
account_struct Datacenter::account() {
	return m_account;
}

/*!
 * Get tick at position
 * @param const int pos 0 = latest, 1 = previous ...
 * @return tick_struct
 */
tick_struct Datacenter::tick(const int pos) {
	if( ! has_symbol(m_ticklist, symbol()) ){
		throw std::runtime_error{"Symbol " + symbol() + " not found."};
	}

	auto list = m_ticklist[symbol()];

	if( pos > (list.size() - 1) ){
		throw std::out_of_range{"pos is out of range"};
	}

	return list[pos];
}

/*!
 * Get tick list size of current symbol
 * @return unsigned int
 */
unsigned int Datacenter::ticks(){
	if( ! has_symbol(m_ticklist, symbol()) ){
		throw std::runtime_error{"Symbol " + symbol() + " not found."};
	}

	auto list = m_ticklist[symbol()];

	return list.size();
}

/*!
 * Get bar at position
 * @param const int pos 0 = latest, 1 = previous ...
 * @return bar_struct
 */
bar_struct Datacenter::bar(const int pos) {
	if( ! has_symbol(m_barlist, symbol() )){
		throw std::runtime_error{"Symbol " + symbol() + "not found."};
	}

	auto list = m_barlist[symbol()];

	if( pos > (list.size() - 1) ){
		throw std::out_of_range{"pos is out of range."};
	}

	return list[pos];
}

/*!
 * Get bar list size of current symbol
 * @return unsigned int
 */
unsigned int Datacenter::bars(){
	if( ! has_symbol(m_barlist, symbol()) ){
		throw std::runtime_error{"Symbol " + symbol() + " not found."};
	}

	auto list = m_barlist[symbol()];

	return list.size();	
}

/*!
 * Add new tick to global m_ticklist for current symbol
 * Also add tick to current open bar, or create a new bar with this tick
 * 
 * @param tick_struct tick
 */
void Datacenter::add_tick(idefix::tick_struct tick){
	// search for symbol in ticklist
	if( has_symbol(m_ticklist, tick.symbol) ){
		// found, add tick
		m_ticklist[tick.symbol].push_front(tick);
	} else {
		// not found, add new pair
		idefix::tick_deq_t ticks;
		ticks.push_front(tick);
		m_ticklist.insert(std::make_pair(tick.symbol, ticks));
	}

	// add tick to open bar, or create a new bar with this tick as first element
	

	// call event on_tick
	on_tick(tick);
}

/*!
 * Add new bar to global m_barlist for current symbol
 * @param bar_struct bar
 */
void Datacenter::add_bar(idefix::bar_struct bar){
	// search for symbol in barlist
	if( has_symbol(m_barlist, symbol()) ){
		// found, add bar
		m_barlist[symbol()].push_front(bar);
	} else {
		// not found, add new pair
		idefix::bar_deq_t bars;
		bars.push_front(bar);
		m_barlist.insert(std::make_pair(symbol(), bars));
	}
}

/*!
 * Check if list contains the symbol
 */
template <typename LIST_TYPE>
bool Datacenter::has_symbol(LIST_TYPE list, const std::string symbol){
	return (list.find(symbol) != list.end());
}

/*!
 * Get latest ask price
 * @return double
 */
double Datacenter::ask() noexcept {
	return tick(0).ask;
}

/*!
 * Get latest bid price
 * @return double
 */
double Datacenter::bid() noexcept {
	return tick(0).bid;
}

/*!
 * Get latest spread
 * @return double
 */
double Datacenter::spread() noexcept {
	return tick(0).spread;
}

/*!
 * Callback initializing 
 */
void Datacenter::on_init() {
	std::cout << "onInit not yet implemented." << std::endl;
};

/*!
 * Callback for every new tick
 * @param const tick_struct&
 */
void Datacenter::on_tick(const tick_struct& tick){
	std::cout << tick << std::endl;
};

/*!
 * Callback if an error occours
 * @param const std::string&
 */
void Datacenter::on_error(const std::string& error){
	std::cout << "[ERROR] " << error << std::endl;
};

/*!
 * Callback if balance changes
 */
void Datacenter::on_balance(){
	std::cout << "[BALANCE] " << balance() << std::endl;
};

/*!
 * Callback for every new candle
 */
void Datacenter::on_candle(const bar_struct& candle){
	// add bar to list
	add_bar(candle);

	std::cout << candle << std::endl;
};

/*!
 * Callback when the datacenter exits
 */
void Datacenter::on_exit(){
	std::cout << "onExit not yet implemented." << std::endl;
}

/*!
 * Returns true if the period for a new bar is over
 * @return [description]
 */
bool Datacenter::is_next_bar_time() {
	using namespace std::chrono;

	steady_clock::time_point now_tp = steady_clock::now();
	duration<double> time_span = duration_cast<duration<double> >(m_last_bar_tp - now_tp);

	return ( time_span.count() >= m_bar_period_seconds );
}
};