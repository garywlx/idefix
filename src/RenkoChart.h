#ifndef IDEFIX_RENKO_CHART_H
#define IDEFIX_RENKO_CHART_H

#include "Chart.h"
#include <vector>
#include <string>
#include "indicator/RenkoBrick.h"
#include <quickfix/Mutex.h>
#include "Exceptions.h"

namespace IDEFIX {
	class RenkoChart: public Chart {
	private:
		FIX::Mutex m_mutex;
		double m_period;
		std::vector<RenkoBrick> m_bricks;

		RenkoBrick m_current_brick;
		RenkoBrick m_last_brick;
		RenkoBrick m_init_brick;

	public:
		RenkoChart(const std::string& symbol, const double period);
		~RenkoChart();

		void add_tick(const Tick& tick);
		std::vector<RenkoBrick> brick_list();
		RenkoBrick at(const int index) throw( IDEFIX::out_of_range, IDEFIX::element_not_found );

	private:
		bool init_brick(const Tick& tick);
	};
};

#endif