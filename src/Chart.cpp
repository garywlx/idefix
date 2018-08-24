#include "Chart.h"

namespace IDEFIX {

Chart::Chart() {}

Chart::~Chart() {}

/*!
 * Is called in fixmanager.onInit
 */
void Chart::on_init() {
	// init indicators
	for ( auto indicator : m_indicators ) {
		indicator->on_init( *this );
	}

	// init strategy
	if ( m_strategy != nullptr ) {
		m_strategy->on_init( *this );
	}
}

/*!
 * Add new tick to the chart and update all indicators
 * 
 * @param const Tick& tick
 */
void Chart::add_tick(const Tick& tick) {
	// update indicator
	for ( auto indicator : m_indicators ) {
		indicator->on_tick( tick );
	}

	if ( m_strategy != nullptr ) {
		m_strategy->on_tick( *this, tick );
	}
}

/*!
 * Add indicator to the list. If the name already exist,
 * the indicator will not be listed again.
 * 
 * @param AbstractIndicator* indicator
 */
void Chart::add_indicator(AbstractIndicator* indicator) {
	auto it = std::find_if( m_indicators.begin(), m_indicators.end(), [=](AbstractIndicator* indi) {
		return indicator->name() == indi->name();
	});

	if ( it != m_indicators.end() ) {
		// indicator found, don't add it twice
		return;
	}

	m_indicators.push_back( indicator );
}

/*!
 * Draw the chart, just for reference at the moment.
 * Used if the dashboard is build
 */
void Chart::plot() {}

/*!
 * Setter symbol
 * 
 * @param const std::string& symbol
 */
void Chart::set_symbol(const std::string &symbol) {
	if ( m_symbol != symbol ) {
		m_symbol = symbol;
	}
}

/*!
 * Getter symbol
 * 
 * @return const std::string
 */
std::string Chart::symbol() const {
	return m_symbol;
}

/*!
 * Getter tick list
 * 
 * @return std::vector<Tick>
 */
std::vector<Tick> Chart::ticks() {
	return m_ticks;
}

/*!
 * Setter Strategy
 * 
 * @param AbstractStrategy* strategy
 */
void Chart::set_strategy(AbstractStrategy* strategy) {
	if ( m_strategy != strategy ) {
		m_strategy = strategy;

		m_strategy->on_init( *this );
	}
}

/*!
 * Getter strategy
 * 
 * @return AbstractStrategy*
 */
AbstractStrategy* Chart::strategy() {
	return m_strategy;
}


};