#ifndef IDEFIX_TICK_H
#define IDEFIX_TICK_H

#include <iostream>
#include <string>

namespace IDEFIX {
	class Tick {
	private:
		double m_bid;
		double m_ask;
		std::string m_sending_time;

	public:
		Tick(const double bid, const double ask, const std::string sending_time) {
			m_bid = bid;
			m_ask = ask;
			m_sending_time = sending_time;
		}
		~Tick() {}

		inline double getBid() const {
			return m_bid;
		}
		inline double getAsk() const {
			return m_ask;
		}
		inline std::string getSendingTime() const {
			return m_sending_time;
		}
	};
};

#endif