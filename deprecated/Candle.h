#ifndef IDEFIX_CANDLE_H
#define IDEFIX_CANDLE_H

#include "Tick.h"
#include <vector>
#include <quickfix/Mutex.h>
#include <ostream>
#include <iomanip>
#include <string>

namespace IDEFIX {
	class Candle {
	private:
		mutable FIX::Mutex m_mutex;

		// Candle period in seconds, 1h = 3600, 30min = 1800, 15min = 900, 5min = 300, 1min = 60
		unsigned int m_period;
		// hold all ticks for the period
		std::vector<Tick> m_ticks;

		// open bid price
		double m_open;
		// last ask price for period
		double m_high;
		// last bid price for period
		double m_low;
		// last bid price for period
		double m_close;

		// precision for output
		unsigned int m_precision;

		// open and close time stamp
		std::string m_open_time;
		std::string m_close_time;

	public:
		Candle(const unsigned int period, const unsigned int precision = 5);
		~Candle();

		void addTick(const Tick tick);
		unsigned int getPeriod() const;
		double getOpen() const;
		double getHigh() const;
		double getLow() const;
		double getClose() const;
		unsigned int getPrecision() const;
		unsigned int getVolume() const;
		std::string getOpenTime() const;
		std::string getCloseTime() const;
		bool isClosed() const;

		std::vector<Tick> getTicks();
		void close(const std::string close_time);
	};

	inline std::ostream& operator<<(std::ostream& out, const IDEFIX::Candle& candle) {
		out << std::setprecision( candle.getPrecision() ) << std::fixed;
		out << "[Candle] o(" << candle.getOpen() << ") "
			<< " h(" << candle.getHigh() << ") "
			<< " l(" << candle.getLow() << ") "
			<< " c(" << candle.getClose() << ") "
			<< std::setprecision( 0 )
			<< " v(" << candle.getVolume() << ") "
			<< " ot: " << candle.getOpenTime()
			<< " ct: " << candle.getCloseTime();

		return out;
	}
};

#endif