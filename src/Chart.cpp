#include "Chart.h"

namespace IDEFIX {

Chart::Chart() {}

Chart::~Chart() {}

/*!
 * Is called in fixmanager.onInit
 */
void Chart::on_init() {
	FIX::Locker lock( m_mutex );

	// init indicators
	if ( ! m_indicators.empty() ) {
		for ( auto indicator : m_indicators ) {
			if ( indicator != NULL ) {
				indicator->on_init( *this );	
			}
		}	
	}
	
	// init strategy
	if ( m_strategy != NULL ) {
		m_strategy->on_init( *this );
	}
}

/*!
 * Is called in fixmanager.onExit
 */
void Chart::on_exit() {
	FIX::Locker lock( m_mutex );

	// init indicators
	if ( ! m_indicators.empty() ) {
		for ( auto indicator : m_indicators ) {
			if ( indicator != NULL ) {
				indicator->on_exit( *this );	
			}
		}
	}

	// init strategy
	if ( m_strategy != NULL ) {
		m_strategy->on_exit( *this );
	}	
}

/*!
 * Add new tick to the chart and update all indicators
 * 
 * @param const Tick& tick
 */
void Chart::add_tick(const Tick& tick) {
	FIX::Locker lock( m_mutex );

	// update indicator
	if ( ! m_indicators.empty() ) {
		for ( auto indicator : m_indicators ) {
			if ( indicator != NULL ) {
				indicator->on_tick( tick );	
			}
		}	
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
	FIX::Locker lock( m_mutex );

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
	FIX::Locker lock( m_mutex );

	return m_ticks;
}

/*!
 * Setter Strategy
 * 
 * @param AbstractStrategy* strategy
 */
void Chart::set_strategy(AbstractStrategy* strategy) {
	FIX::Locker lock( m_mutex );

	if ( m_strategy != strategy && strategy != NULL ) {
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
	FIX::Locker lock( m_mutex );
	
	return m_strategy;
}


};