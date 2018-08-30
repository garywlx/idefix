#ifndef IDEFIX_RENKO_CHART_H
#define IDEFIX_RENKO_CHART_H

#include <vector>
#include "RenkoBrick.h"
#include "MarketSnapshot.h"
#include "Exceptions.h"
#include <quickfix/Mutex.h>
#include <nod/nod.hpp>

namespace IDEFIX {
	class RenkoChart {
	private:
		FIX::Mutex m_mutex;
		double m_period;
		std::vector<RenkoBrick> m_bricks;

		RenkoBrick m_current_brick;
		RenkoBrick m_last_brick;
		RenkoBrick m_init_brick;

	public:
		RenkoChart();
		RenkoChart(const double period);
		~RenkoChart();

		void on_tick(const MarketSnapshot& tick);
		std::vector<RenkoBrick> brick_list();
		int brick_count();
		RenkoBrick at(const int index) throw( IDEFIX::out_of_range, IDEFIX::element_not_found );

		// signals
		nod::signal<void(const RenkoBrick&)> on_brick;

	private:
		bool init_brick(const MarketSnapshot& tick);
	};
};

#endif