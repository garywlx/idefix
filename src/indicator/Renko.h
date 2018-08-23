#ifndef IDEFIX_RENKO_H
#define IDEFIX_RENKO_H

#include <vector>
#include "RenkoBrick.h"
#include "Tick.h"

namespace IDEFIX {
	class Renko {
	private:
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

	private:
		bool init_brick(const Tick& tick);
	};
};

#endif