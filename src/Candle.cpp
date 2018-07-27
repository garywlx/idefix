#include "Candle.h"
#include <iostream>

namespace IDEFIX {
	Candle::Candle(const unsigned int period, const unsigned int precision) {
		m_period = period;
		m_open = m_high = m_low = m_close = 0;
		m_precision = precision;
	}

	Candle::~Candle() {

	}

	unsigned int Candle::getPeriod() const {
		return m_period;
	}

	double Candle::getOpen() const {
		return m_open;
	}

	double Candle::getHigh() const {
		return m_high;
	}

	double Candle::getLow() const {
		return m_low;
	}

	double Candle::getClose() const {
		return m_close;
	}

	unsigned int Candle::getPrecision() const {
		return m_precision;
	}

	unsigned int Candle::getVolume() const {
		FIX::Locker lock(m_mutex);
		return m_ticks.size();
	}

	std::string Candle::getOpenTime() const {
		return m_open_time;
	}

	std::string Candle::getCloseTime() const {
		return m_close_time;
	}

	/*!
	 * Add tick to list of ticks, calculates new high/low
	 * 
	 * @param Tick tick one Tick
	 */
	void Candle::addTick(const Tick tick) {
		FIX::Locker lock(m_mutex);
		if ( m_ticks.empty() ) {
			// set open and low
			m_open = tick.getBid();
			// set open time
			m_open_time = tick.getSendingTime();
		}

		m_ticks.push_back( tick );

		// update low, high
		for ( int i = 0; i < m_ticks.size(); i++ ) {
			auto tick = m_ticks.at( i );

			m_low = std::min( m_low, tick.getBid() );
			m_high = std::max( m_high, tick.getAsk() );
		}
	}

	/*!
	 * Close candle
	 * 
	 * @param const std::string close_time The timestamp to close the candle
	 */
	void Candle::close(const std::string close_time) {
		if ( m_close_time != close_time ) {
			m_close_time = close_time;	
		}
	}

	/*!
	 * If close time is not empty, the candle is closed.
	 * 
	 * @return bool
	 */
	bool Candle::isClosed() const {
		return ! m_close_time.empty();
	}
};
