#include "MovingAverage.h"
#include "../FIXManager.h"
#include <iterator>

namespace IDEFIX {
	MovingAverage::MovingAverage(const int period, const int offset):
		m_period( period ), m_offset( offset ) {

	}

	// Get unique name of indicator
	std::string MovingAverage::getName() const {
		return "MovingAverage";
	}

	// Update indicator
	void MovingAverage::onTick(FIXManager &manager, const MarketSnapshot &snapshot) {
		// do nothing with tick
	}

	void MovingAverage::calculate(const double next_price) {
		// add next price to list
		m_prices.push_back( next_price );
		// check if we have enough data to calculate
		if ( m_prices.size() < m_period ) {
			m_current_value = 0;
			return;
		}

		// calculate for period
		double sum = 0;
		// iterator start element last first
		auto it = m_prices.rbegin();
		// iterate for periods
		for ( int i = 0; i < m_period; i++ ) {
			// move iterator by incrementor
			auto vi = std::next( it, i );
			// get close price
			sum += *vi;
		}

		// calculate average
		m_current_value = sum / m_period;
	}

	// Return current average value
	double MovingAverage::getValue() const {
		return m_current_value;
	}
};