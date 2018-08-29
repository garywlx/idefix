#define RS_SHOW_DEBUG true

#include "RenkoStrategy.h"
#include "../Chart.h"
#include "../RenkoChart.h"
#include "../indicator/Tick.h"
#include "../indicator/Bar.h"
#include "../Operators.h"

#if RS_SHOW_DEBUG
#include "../Console.h"
#include <sstream>
#endif

namespace IDEFIX {

RenkoStrategy::RenkoStrategy() {}

// is called on initialization
void RenkoStrategy::on_init(Chart* chart) {
#if RS_SHOW_DEBUG
	console()->info("[RenkoStrategy:{}] {}", chart->symbol(), __FUNCTION__ );
#endif

	// set max values
	m_max_short_pos = 3;
	m_short_pos     = 0;
	m_max_long_pos  = 3;
	m_long_pos      = 0;
}

// is called on chart tick
void RenkoStrategy::on_tick(Chart* chart, const Tick& tick) {
#if RS_SHOW_DEBUG
	//console()->info("[RenkoStrategy:{}] {} {}", chart->symbol(), __FUNCTION__, ss.str() );
#endif
}

// is called on chart bar
void RenkoStrategy::on_bar(Chart* chart, const Bar& bar) {
#if RS_SHOW_DEBUG
	std::stringstream ss;
	ss << bar;
	console()->info("[RenkoStrategy:{}] {}", chart->symbol(), ss.str() );
#endif

	try {
		auto derived_chart = static_cast<RenkoChart*>(chart);

		// add value to moving average
		m_sma5.add( ( bar.open_price + bar.close_price + bar.high_price + bar.low_price ) / 4 );

		// get previous bar
		//auto previous_bar = chart->at( 1 );
		auto previous_bar = derived_chart->at( 1 );

		// shortcuts
		double last_ma    = m_sma5.value();
		double open       = bar.open_price;
		double close      = bar.close_price;
		double open_1     = previous_bar.open_price;
		double close_1    = previous_bar.close_price;
		bool enter_long   = false;
		bool exit_long    = false;
		bool enter_short  = false;
		bool exit_short   = false;
		bool bar_long     = open < close;
		bool bar_1_long   = open_1 < close_1;
		bool bar_above_ma = open > last_ma && close > last_ma;
		bool bar_below_ma = open < last_ma && close < last_ma;

		// RULES
		// ------------------------------
		// entry signal long
		// ------------------------------
		// brick_1 == SHORT
		// brick   == LONG
		// OR
		// brick_1 == LONG
		// brick   == LONG
		// 
		// brick > last_ma
		if ( ( ( bar_long && ! bar_1_long ) || ( bar_long && bar_1_long ) ) && bar_above_ma ) {
			enter_long = true;
		}

		// ------------------------------
		// exit signal long
		// ------------------------------
		// brick_1 == LONG
		// brick   == SHORT
		// brick.close <= last_ma
		if ( ! bar_long && bar_1_long && close <= last_ma ) {
			exit_long = true;
		}

		// ------------------------------
		// entry signal short
		// ------------------------------
		// brick_1 == LONG
		// brick   == SHORT
		// OR
		// brick_1 == SHORT
		// brick   == SHORT
		// 
		// brick < last_ma
		if ( ( ( bar_1_long && ! bar_long ) || ( ! bar_1_long && ! bar_long ) ) && bar_below_ma ) {
			enter_short = true;
		}

		// ------------------------------
		// exit singal short
		// ------------------------------
		// brick_1 == SHORT
		// brick   == LONG
		// brick.close >= last_ma
		if ( ! bar_1_long && bar_long && close >= last_ma ) {
			exit_short = true;
		}

		// ------------------------------
		// EXECUTE
		// ------------------------------
		
		if ( enter_long && m_long_pos < m_max_long_pos ) {
			console()->info("[EnterLong] {:d} {}", m_long_pos, chart->symbol() );
			m_long_pos++;
		}
		if ( exit_long && m_long_pos > 0 ) {
			console()->warn("[ExitLong] {:d} {}", m_long_pos, chart->symbol() );
			m_long_pos = 0;
		}
		if ( enter_short && m_short_pos < m_max_short_pos ) {
			console()->error("[EnterShort] {:d} {}", m_short_pos, chart->symbol() );
			m_short_pos++;
		}
		if( exit_short && m_short_pos > 0 ) {
			console()->warn("[ExitShort] {:d} {}", m_short_pos, chart->symbol() );
			m_short_pos = 0;
		}	
	} catch( IDEFIX::out_of_range* oor ) {
		console()->warn("{}", oor->what() );
	} catch( IDEFIX::element_not_found* enf ) {
		console()->warn("{}", enf->what() );
	} catch(...) {

	}
}

// is called on exit
void RenkoStrategy::on_exit(Chart* chart) {
#if RS_SHOW_DEBUG
	console()->info("[RenkoStrategy:{}] {}", chart->symbol(), __FUNCTION__ );
#endif
}


};