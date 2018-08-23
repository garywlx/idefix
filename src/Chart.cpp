#include "Chart.h"

namespace IDEFIX {

Chart::Chart(): m_type( Chart::Type::TICK_ONLY ) {}

Chart::~Chart() {}

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
 * Setter chart type
 * 
 * @param const Chart::Type type
 */
void Chart::set_type(const Chart::Type type) {
	if ( m_type != type ) {
		m_type = type;
	}
}

/*!
 * Getter chart type
 * 
 * @return const Chart::Type
 */
Chart::Type Chart::type() const {
	return m_type;
}

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

};