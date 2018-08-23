#ifndef IDEFIX_RENKO_H
#define IDEFIX_RENKO_H

#include <vector>
#include "RenkoBrick.h"
#include "Tick.h"
#include <quickfix/Mutex.h>
#include "../Exceptions.h"
#include "Indicator.h"

namespace IDEFIX {
	class FIXManager;
	class MarketSnapshot;

	class Renko: public Indicator {
	private:
		FIX::Mutex m_mutex;
		bool m_verbose_mode;
		double m_period;
		std::vector<Tick> m_ticks;
		std::vector<RenkoBrick> m_bricks;

		RenkoBrick m_current_brick;
		RenkoBrick m_last_brick;
		RenkoBrick m_init_brick;

	public:
		Renko(const double period, const bool verbose_mode = false);
		~Renko();

		bool add_tick(const Tick& tick);
		std::vector<RenkoBrick> brick_list();
		std::vector<Tick> tick_list();

		RenkoBrick at(const int index) throw( IDEFIX::out_of_range, IDEFIX::element_not_found );

		// Get name of this indicator, must be unique
		std::string getName() const;
		// Is called on every tick
		void onTick(FIXManager& manager, const MarketSnapshot& snapshot);

	private:
		bool init_brick(const Tick& tick);
	};
};

#endif