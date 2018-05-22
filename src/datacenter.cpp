#include "datacenter.h"

namespace idefix {

/*!
 * Constructor
 */
Datacenter::Datacenter(asset_struct asset): m_asset(asset) {

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
void Datacenter::set_asset(const idefix::asset_struct asset) noexcept {
	m_asset = asset;
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
 * Get tick at position
 * @param const int pos 0 = latest, 1 = previous ...
 * @return tick_struct
 */
tick_struct Datacenter::tick(const int pos) {
	if( ! hasSymbol(m_ticklist, symbol()) ){
		throw std::runtime_error{"Symbol " + symbol() + " not found."};
	}

	auto list = m_ticklist[symbol()];

	if( pos > (list.size() - 1 ) ){
		throw std::out_of_range{"pos is out of range"};
	}

	return list[pos];
}

/*!
 * Get tick list size of current symbol
 * @return unsigned int
 */
unsigned int Datacenter::ticks(){
	if( ! hasSymbol(m_ticklist, symbol()) ){
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
	if( ! hasSymbol(m_barlist, symbol() )){
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
	if( ! hasSymbol(m_barlist, symbol()) ){
		throw std::runtime_error{"Symbol " + symbol() + " not found."};
	}

	auto list = m_barlist[symbol()];

	return list.size();	
}


void Datacenter::debug_bar_list(){
	auto list = m_barlist[symbol()];
	auto it = list.begin();
	std::cout << "[BARLIST] " << symbol() << std::endl;
	while(it != list.end()){
		std::cout << *it << std::endl;
		it++;
	} 
}

void Datacenter::debug_tick_list(){
	auto list = m_ticklist[symbol()];
	auto it = list.begin();
	std::cout << "[TICKLIST] " << symbol() << std::endl;
	while(it != list.end()){
		std::cout << *it << std::endl;
		it++;
	} 
}

/*!
 * Add new tick to global m_ticklist for current symbol
 * @param tick_struct tick
 */
void Datacenter::add_tick(idefix::tick_struct tick){
	// search for symbol in ticklist
	if( hasSymbol(m_ticklist, tick.symbol) ){
		// found, add tick
		m_ticklist[tick.symbol].push_front(tick);
	} else {
		// not found, add new pair
		idefix::tick_deq_t ticks;
		ticks.push_front(tick);
		m_ticklist.insert(std::make_pair(tick.symbol, ticks));
	}
}

/*!
 * Add new bar to global m_barlist for current symbol
 * @param bar_struct bar
 */
void Datacenter::add_bar(idefix::bar_struct bar){
	// search for symbol in barlist
	//if(m_barlist.find(symbol()) != m_barlist.end()){
	if( hasSymbol(m_barlist, symbol()) ){
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
bool Datacenter::hasSymbol(LIST_TYPE list, const std::string symbol){
	return (list.find(symbol) != list.end());
}

};